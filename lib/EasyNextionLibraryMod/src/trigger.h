/*!
 * trigger.cpp - Easy library for Nextion Displays
 * Copyright (c) 2020 Athanasios Seitanis < seithagta@gmail.com >. 
 * All rights reserved under the library's licence
 */

/*! We separate this file from the EasyNextionLibrary.cpp in order to make easier the modifications for it
 * and for simplifying reasons instead of dealing with a large code file.
 
 * if you want to change the name of the predefined voids that I made,(trigger1, trigger2....etc)
 * you must change:
   1. The name from "declare the functions"  
       ---------(example: extern void trigger1(); -> extern void myFunction();)

   2. The name from "declaration of the function as weak"  
    (example: extern void trigger1() __attribute__((weak)); ---> extern void myFunction() __attribute__((weak));)

   3. the predefined name in the switch(){case} for the trigger command group at the callTriggers.cpp file
   
 * With the same way, you can add as many extern void as you like
 * NOTE: WE can have UP TO 255 extern void
 *
 * When a function has a weak attribute it will be created only when user
 * declare this function on the main code
 */


#ifndef trigger_h
#define trigger_h

// weak attribute funcion for read the custom command protocol

extern void easyNexReadCustomCommand();
extern void easyNexReadCustomCommand() __attribute__((weak));

// declare the functions for triggers
extern void trigger0();
extern void trigger1(); 
extern void trigger2();
extern void trigger3();
extern void trigger4();
extern void trigger5();
extern void trigger6();
extern void trigger7();
extern void trigger8();
extern void trigger9();
extern void trigger10();
extern void trigger11();
extern void trigger12();
extern void trigger13();
extern void trigger14();
extern void trigger15();
extern void trigger16();
extern void trigger17();
extern void trigger18();
extern void trigger19();
extern void trigger20();
extern void trigger21();
extern void trigger22();
extern void trigger23();
extern void trigger24();
extern void trigger25();
extern void trigger26();
extern void trigger27();
extern void trigger28();
extern void trigger29();
extern void trigger30();
extern void trigger31();
extern void trigger32();
extern void trigger33();
extern void trigger34();
extern void trigger35();
extern void trigger36();
extern void trigger37();
extern void trigger38();
extern void trigger39();
extern void trigger40();
extern void trigger41();
extern void trigger42();
extern void trigger43();
extern void trigger44();
extern void trigger45();
extern void trigger46();
extern void trigger47();
extern void trigger48();
extern void trigger49();
extern void trigger50();

// declare the functions for triggermenus
extern void triggermainmenu(uint8_t);
extern void triggersubmenu(uint8_t); 
extern void triggermenu2();
extern void triggermenu3();
extern void triggermenu4();
extern void triggermenu5();
extern void triggermenu6();
extern void triggermenu7();
extern void triggermenu8();
extern void triggermenu9();
extern void triggermenu10();
extern void triggermenu11();
extern void triggermenu12();
extern void triggermenu13();
extern void triggermenu14();
extern void triggermenu15();
extern void triggermenu16();
extern void triggermenu17();
extern void triggermenu18();
extern void triggermenu19();
extern void triggermenu20();
extern void triggermenu21();
extern void triggermenu22();
extern void triggermenu23();
extern void triggermenu24();
extern void triggermenu25();
extern void triggermenu26();
extern void triggermenu27();
extern void triggermenu28();
extern void triggermenu29();
extern void triggermenu30();
extern void triggermenu31();
extern void triggermenu32();
extern void triggermenu33();
extern void triggermenu34();
extern void triggermenu35();
extern void triggermenu36();
extern void triggermenu37();
extern void triggermenu38();
extern void triggermenu39();
extern void triggermenu40();
extern void triggermenu41();
extern void triggermenu42();
extern void triggermenu43();
extern void triggermenu44();
extern void triggermenu45();
extern void triggermenu46();
extern void triggermenu47();
extern void triggermenu48();
extern void triggermenu49();
extern void triggermenu50();

// declaration of the function as weak
extern void trigger0() __attribute__((weak));
extern void trigger1() __attribute__((weak));
extern void trigger2() __attribute__((weak));
extern void trigger3() __attribute__((weak));
extern void trigger4() __attribute__((weak));
extern void trigger5() __attribute__((weak));
extern void trigger6() __attribute__((weak));
extern void trigger7() __attribute__((weak));
extern void trigger8() __attribute__((weak));
extern void trigger9() __attribute__((weak));
extern void trigger10() __attribute__((weak));
extern void trigger11() __attribute__((weak));
extern void trigger12() __attribute__((weak));
extern void trigger13() __attribute__((weak));
extern void trigger14() __attribute__((weak));
extern void trigger15() __attribute__((weak));
extern void trigger16() __attribute__((weak));
extern void trigger17() __attribute__((weak));
extern void trigger18() __attribute__((weak));
extern void trigger19() __attribute__((weak));
extern void trigger20() __attribute__((weak));
extern void trigger21() __attribute__((weak));
extern void trigger22() __attribute__((weak));
extern void trigger23() __attribute__((weak));
extern void trigger24() __attribute__((weak));
extern void trigger25() __attribute__((weak));
extern void trigger26() __attribute__((weak));
extern void trigger27() __attribute__((weak));
extern void trigger28() __attribute__((weak));
extern void trigger29() __attribute__((weak));
extern void trigger30() __attribute__((weak));
extern void trigger31() __attribute__((weak));
extern void trigger32() __attribute__((weak));
extern void trigger33() __attribute__((weak));
extern void trigger34() __attribute__((weak));
extern void trigger35() __attribute__((weak));
extern void trigger36() __attribute__((weak));
extern void trigger37() __attribute__((weak));
extern void trigger38() __attribute__((weak));
extern void trigger39() __attribute__((weak));
extern void trigger40() __attribute__((weak));
extern void trigger41() __attribute__((weak));
extern void trigger42() __attribute__((weak));
extern void trigger43() __attribute__((weak));
extern void trigger44() __attribute__((weak));
extern void trigger45() __attribute__((weak));
extern void trigger46() __attribute__((weak));
extern void trigger47() __attribute__((weak));
extern void trigger48() __attribute__((weak));
extern void trigger49() __attribute__((weak));
extern void trigger50() __attribute__((weak));


extern void triggermainmenu() __attribute__((weak));
extern void triggersubmenu() __attribute__((weak));
extern void triggermenu2() __attribute__((weak));
extern void triggermenu3() __attribute__((weak));
extern void triggermenu4() __attribute__((weak));
extern void triggermenu5() __attribute__((weak));
extern void triggermenu6() __attribute__((weak));
extern void triggermenu7() __attribute__((weak));
extern void triggermenu8() __attribute__((weak));
extern void triggermenu9() __attribute__((weak));
extern void triggermenu10() __attribute__((weak));
extern void triggermenu11() __attribute__((weak));
extern void triggermenu12() __attribute__((weak));
extern void triggermenu13() __attribute__((weak));
extern void triggermenu14() __attribute__((weak));
extern void triggermenu15() __attribute__((weak));
extern void triggermenu16() __attribute__((weak));
extern void triggermenu17() __attribute__((weak));
extern void triggermenu18() __attribute__((weak));
extern void triggermenu19() __attribute__((weak));
extern void triggermenu20() __attribute__((weak));
extern void triggermenu21() __attribute__((weak));
extern void triggermenu22() __attribute__((weak));
extern void triggermenu23() __attribute__((weak));
extern void triggermenu24() __attribute__((weak));
extern void triggermenu25() __attribute__((weak));
extern void triggermenu26() __attribute__((weak));
extern void triggermenu27() __attribute__((weak));
extern void triggermenu28() __attribute__((weak));
extern void triggermenu29() __attribute__((weak));
extern void triggermenu30() __attribute__((weak));
extern void triggermenu31() __attribute__((weak));
extern void triggermenu32() __attribute__((weak));
extern void triggermenu33() __attribute__((weak));
extern void triggermenu34() __attribute__((weak));
extern void triggermenu35() __attribute__((weak));
extern void triggermenu36() __attribute__((weak));
extern void triggermenu37() __attribute__((weak));
extern void triggermenu38() __attribute__((weak));
extern void triggermenu39() __attribute__((weak));
extern void triggermenu40() __attribute__((weak));
extern void triggermenu41() __attribute__((weak));
extern void triggermenu42() __attribute__((weak));
extern void triggermenu43() __attribute__((weak));
extern void triggermenu44() __attribute__((weak));
extern void triggermenu45() __attribute__((weak));
extern void triggermenu46() __attribute__((weak));
extern void triggermenu47() __attribute__((weak));
extern void triggermenu48() __attribute__((weak));
extern void triggermenu49() __attribute__((weak));
extern void triggermenu50() __attribute__((weak));

#endif