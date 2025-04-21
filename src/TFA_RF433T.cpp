
// Credits :
//      https://github.com/m5stack/M5Stack/blob/master/examples/Unit/RF433/RF433.ino
//      https://github.com/d10i/TFA433/blob/main/src/tfa433.cpp
//      https://github.com/zopieux/rtl_433_f007th/blob/master/src/main.cpp


#include <Arduino.h>
#include "Config.h"
#include "PoolMaster.h"

#ifdef _TFA_RF433T_
#include "TFA_RF433T.h"
#include <driver/rmt.h>

const char *RF433TName = "TFA_RF433T";
static bool RF433T = false; // no RF433T transmitter by default
#define GPIO 9              // GPIO009 (SD2)

#define RMT_TX_CHANNEL  (RMT_CHANNEL_0)
//#define RMT_RX_CHANNEL  (RMT_CHANNEL_1)
#define RTM_TX_GPIO_NUM (GPIO) 
//#define RTM_RX_GPIO_NUM (36)
#define RTM_BLOCK_NUM   (1)
#define RMT_CLK_DIV     (80) /*!< RMT counter clock divider */
#define RMT_1US_TICKS   (80000000 / RMT_CLK_DIV / 1000000)
#define RMT_1MS_TICKS   (RMT_1US_TICKS * 1000)

rmt_item32_t rmtbuff[2048];

#define T0H 670
#define T1H 320
#define T0L 348
#define T1L 642

#define RMT_CODE_H      {670, 1, 320, 0}
#define RMT_CODE_L      {348, 1, 642, 0}
#define RMT_START_CODE0 {4868, 1, 2469, 0}
#define RMT_START_CODE1 {1647, 1, 315, 0}


void RF433TInit(void)
{
    rmt_config_t txconfig;
    txconfig.rmt_mode                 = RMT_MODE_TX;
    txconfig.channel                  = RMT_TX_CHANNEL;
    txconfig.gpio_num                 = gpio_num_t(RTM_TX_GPIO_NUM);
    txconfig.mem_block_num            = RTM_BLOCK_NUM;
    txconfig.tx_config.loop_en        = false;
    txconfig.tx_config.carrier_en     = false;
    txconfig.tx_config.idle_output_en = true;
    txconfig.tx_config.idle_level     = rmt_idle_level_t(0);
    txconfig.clk_div                  = RMT_CLK_DIV;

    if (rmt_config(&txconfig) == ESP_OK) RF433T = false;

    if ((RF433T) && (rmt_driver_install(txconfig.channel, 0, 0) != ESP_OK))
        RF433T = false;

    if (!RF433T) {
        Debug.print(DBG_INFO,"no RF433T");
        return;
    }
}

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

void RF433TAction(void *pvParameters)
{
    // Read Water Temp.

    // Format TFA Dostmann payload
    
    // send info to TFA receiver at 433Mhz, Channel #1
    uint8_t data[6] = {0xAA, 0x55, 0x01, 0x02, 0x03, 0x04};
    send(data, 6);
   return;
}

//  Publish RF433T -> MQTT
/*
void RF433TMeasureJSON (void)
{
    return;  // No settings to store in MQTT
}

void RF433TSettingsJSON(void)
{
    return;  // No settings to store in MQTT
}
*/

#endif