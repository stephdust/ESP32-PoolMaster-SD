
// Credits :
//      https://github.com/m5stack/M5Stack/blob/master/examples/Unit/RF433/RF433.ino
//      https://github.com/d10i/TFA433/blob/main/src/tfa433.cpp
//      https://github.com/zopieux/rtl_433_f007th/blob/master/src/main.cpp
//      https://github.com/merbanan/rtl_433/blob/master/src/devices/ambient_weather.c
//
// Usage in Indoor Pool, to get and display Water Temp on nice receiver.
// Air Temperature and Humidity of Indoor Pool are measured directly by the device
// Receiver :   TFA Dostmann Venise 30.3056.10 (without the floating sensor/transmiter)
//              https://www.amazon.fr/dp/B010NSG4V2
//              https://www.tfa-dostmann.de/en/product/wireless-pool-thermometer-venice-30-3056/
//             
// Transmiter : We simulate the Ambiant Weather F007TH
//              https://docs.m5stack.com/en/unit/rf433_t
//              5V - GPIO 9
//

#include <Arduino.h>
#include "Config.h"
#include "PoolMaster.h"

#if defined(_ADDONS_) && defined(_IO_ADDON_TFA_RF433T_)
#include "Addon_TFA_RF433T.h"
#include <driver/rmt.h>

const char *TFA_RF433TName = "TFA_RF433T";
static bool TFA_RF433T = false; // no TFA_RF433T transmitter by default
AddonStruct myTFA_RF433T;

#define RMT_TX_CHANNEL  (RMT_CHANNEL_0)
//#define RMT_RX_CHANNEL  (RMT_CHANNEL_1)
//#define RTM_TX_GPIO_NUM (GPIO) 
//#define RTM_RX_GPIO_NUM (36)
#define RTM_BLOCK_NUM   (1)
#define RMT_CLK_DIV     (80) /*!< RMT counter clock divider */
#define RMT_1US_TICKS   (80000000 / RMT_CLK_DIV / 1000000)
#define RMT_1MS_TICKS   (RMT_1US_TICKS * 1000)

rmt_item32_t rmtbuff[2048];
static byte data[6];

#define RMT_CODE_H      {670, 1, 320, 0}
#define RMT_CODE_L      {348, 1, 642, 0}
#define RMT_START_CODE0 {4868, 1, 2469, 0}
#define RMT_START_CODE1 {1647, 1, 315, 0}

static void send(uint8_t* buff, size_t size)
{
    rmtbuff[0] = (rmt_item32_t){RMT_START_CODE0};
    rmtbuff[1] = (rmt_item32_t){RMT_START_CODE1};
    for (int i = 0; i < size; i++) {
        uint8_t mark = 0x80;
        for (int n = 0; n < 8; n++) {
            rmtbuff[2 + i * 8 + n] = ((buff[i] & mark)) ? ((rmt_item32_t){RMT_CODE_H}) : ((rmt_item32_t){RMT_CODE_L});
            mark >>= 1;
        }
    }
    for (int i = 0; i < 8; i++) {
        ESP_ERROR_CHECK(rmt_write_items(RMT_TX_CHANNEL, rmtbuff, 42, false));
        ESP_ERROR_CHECK(rmt_wait_tx_done(RMT_TX_CHANNEL, portMAX_DELAY));
    }
}

static uint8_t lfsr_digest8(uint8_t const message[], unsigned bytes, uint8_t gen, uint8_t key)
{
    uint8_t sum = 0;
    for (unsigned k = 0; k < bytes; ++k) {
        uint8_t data = message[k];
        for (int i = 7; i >= 0; --i) {
            // fprintf(stderr, "key is %02x\n", key);
            // XOR key into sum if data bit is set
            if ((data >> i) & 1)
                sum ^= key;

            // roll the key right (actually the lsb is dropped here)
            // and apply the gen (needs to include the dropped lsb as msb)
            if (key & 1)
                key = (key >> 1) ^ gen;
            else
                key = (key >> 1);
        }
    }
    return sum;
}

static void TFA_Data(double WTemp)
{
#define TFA_CHANNEL 1         // TFA is using channel 1 of 8
int temp_raw = (((WTemp * 9.0f / 5.0f) + 32.0f) * 10 ) + 400;
/*
Byte 0   Byte 1   Byte 2   Byte 3   Byte 4   Byte 5
xxxxMMMM IIIIIIII BCCCTTTT TTTTTTTT HHHHHHHH MMMMMMMM

- x: Unknown 0x04 on F007TH/F012TH
- M: Model Number?, 0x05 on F007TH/F012TH/SwitchDocLabs F016TH
- I: ID byte (8 bits), volatie, changes at power up,
- B: Battery Low
- C: Channel (3 bits 1-8) - F007TH set by Dip switch, F012TH soft setting
- T: Temperature 12 bits - Fahrenheit * 10 + 400
- H: Humidity (8 bits)
- M: Message integrity check LFSR Digest-8, gen 0x98, key 0x3e, init 0x64
*/
    data[0] = 0x04 << 4;        // F007TH 
    data[0] |= 0x05;            // F007TH/F012TH/F016TH
    data[1] = 0x01;             // random id
    data[2] = 0x00;             // all 0 so Battery=0 (OK), 
    data[2] |= TFA_CHANNEL << 4;
    data[2] |= (temp_raw & 0xF00) >> 8;
    data[3] = temp_raw & 0xFF;
    data[4] = 0x32;             // 50% humidity, not used 
    data[5] = lfsr_digest8(data, 5, 0x98, 0x3e) ^ 0x64;
}


void TFA_RF433TTask(void *pvParameters)
{
    if (!TFA_RF433T) return;
   
    // Read Water Temp, Format TFA Dostmann payload and send
    TFA_Data(storage.WaterTemp * 100);
    send(data, 6);
}

AddonStruct TFA_RF433TInit(void)
{
    // search and init the chip
      rmt_config_t txconfig;
    txconfig.rmt_mode                 = RMT_MODE_TX;
    txconfig.channel                  = RMT_TX_CHANNEL;
    txconfig.gpio_num                 = gpio_num_t(_IO_ADDON_TFA_RF433T_);
    txconfig.mem_block_num            = RTM_BLOCK_NUM;
    txconfig.tx_config.loop_en        = false;
    txconfig.tx_config.carrier_en     = false;
    txconfig.tx_config.idle_output_en = true;
    txconfig.tx_config.idle_level     = rmt_idle_level_t(0);
    txconfig.clk_div                  = RMT_CLK_DIV;
    if (rmt_config(&txconfig) == ESP_OK) TFA_RF433T = true;
    if ((TFA_RF433T) && (rmt_driver_install(txconfig.channel, 0, 0) != ESP_OK)) {
        TFA_RF433T = false;
        Debug.print(DBG_INFO,"no TFA_RF433T");
    }

    // Init xTask
    myTFA_RF433T.name         = TFA_RF433TName;
    myTFA_RF433T.Task         = TFA_RF433TTask;
    myTFA_RF433T.frequency    = 1000;     // Update value each 1000 millisecondes (per TFA requirement)
    myTFA_RF433T.SettingsJSON = 0;
    myTFA_RF433T.MeasuresJSON = 0;
    myTFA_RF433T.HistoryStats = 0;
    myTFA_RF433T.LoadConfig   = 0;
    myTFA_RF433T.SaveConfig   = 0;

    if (TFA_RF433T) 
        xTaskCreatePinnedToCore(
            AddonLoop,
            myTFA_RF433T.name,
            3072,
            &myTFA_RF433T,
            1,
            nullptr,
            xPortGetCoreID()
        );
    return myTFA_RF433T;
}

#endif

