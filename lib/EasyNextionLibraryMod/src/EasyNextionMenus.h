/*!
 * EasyNextionMenus.h - Easy library for building menus on Nextion Displays
 * Copyright (c) 2025 Christophe Belmont
 * All rights reserved under the library's licence
 */
#ifndef EasyNextionMenus_h
#define EasyNextionMenus_h
#include "EasyNextionLibrary.h"

#define MAX_MENU_ITEMS      20  // Maximum Number of Items in each menu (main and sub)
#define MAX_NEXT_CMD_LENGTH 32  // Maximum length of commands send to screen via serial

// Default value for Nextion objects names configured in Nextion Editor
#define MENU_MAIN_OBJECT_TXT        "MainItem"  //Object dual-state button that needs to be selected or not
#define MENU_SUB_OBJECT_TXT         "SubItem"    //Object dual-state button that needs to be selected or not
#define MENU_SUB_OBJECT_CHK         "SubIcon"     //Object dual-state button that needs to be selected or not
#define MENU_SUB_OBJECT_SEP         "Separator"     //Object dual-state button that needs to be selected or not
#define MENU_ACTION_OBJECT_CLICK    "vaActionIndex"     //Object dual-state button that needs to be selected or not
#define MENU_ICONS_UNSELECTED    "┡"     //Uncheckbox icon (see dedicated font for corresponding icon)
#define MENU_ICONS_SELECTED      "┢"     //Checked icon  (see dedicated font for corresponding icon)

#define MNU_YES     1
#define MNU_NO      0
#define MNU_CHECK     1
#define MNU_UNCHECK   0

class EasyNextionMenus;

// Type of Menu Items
// ENM_NONE: Not configured, blanck
// ENM_NEXTION_ACTION: Item executes executed on Nextion screen by settting the reference and clicking a button 
// ENM_BISTABLE: Item is represented as checkbox and changes state upon clicking
// ENM_SUBMENU: Item launches a submenu if clicked
typedef enum {
    ENM_NONE                = 0,   // Menu is not shown
    //ENM_CALLBACK_ACTION     = 1,   // Action executed on controller side (via triggerfunctions)
    ENM_ACTION              = 2,   // Action executed on Nextion side
    ENM_BISTABLE            = 4,   // Launch call back and change bistable button
    ENM_SUBMENU             = 8,
    ENM_OVERLAYPAGE         = 16    // Change Page
} enm_menu_item_type;

// Type of Menu
// ENM_MAIN: Main Menu (mainly holding ENM_SUBMENU items)
// ENM_SUB: Sub Menu with actual actions
typedef enum {
    ENM_MAIN         = 0,   // Main Menu
    ENM_SUB          = 1   // Sub Menu
} enm_menu_type;

// Hold the parameter of each items of a menu
struct Menu_Item {
    public:
        const char* mnu_item_name;               // Name as displayed
        const char* mnu_icon_on = nullptr;
        const char* mnu_icon_off = nullptr;
        int mnu_item_type = ENM_NONE;            // Type as Enum 
        bool (*checkEnabledFunction)() = nullptr; // The function executed to know whether item is enabled or greyed out
        bool (*checkStatusFunction)() = nullptr; // The function executed to know whether item is selected (checked) or not
        void (*callBackFunction)() = nullptr;    // The function executed on microcontroller if the item is clicked
        int mnu_action_reference = -1;           // The function executed on Nextion if the item is clicked (via a reference of function)
        const char* mnu_overlay_page_name = nullptr;     // The name of the page Nextion must jump to if clicked
        EasyNextionMenus* submenu = nullptr;     // The submenu reference if the item is of type ENM_SUBMENU

    private:
};

/********************************************************************* 
**********  Main Menu Class
**********************************************************************/
class EasyNextionMenus {
  public:
 //   EasyNextionMenus(EasyNex*,int, void(*)(),int = ENM_MAIN);   //Constructor
    EasyNextionMenus(EasyNex*,int, int = ENM_MAIN);             //Constructor

    void SetNxtObjectName(const char*);     // If we want to change the Nextion Object Name at run time #define MENU_MAIN_OBJECT_TXT & MENU_SUB_OBJECT_TXT
    void SetNxtCheckObjectName(const char*);// If we want to change the Nextion CheckBox Object Name at run time #define MENU_SUB_OBJECT_CHK
    int AddItem(void (*_CallBackFunction)(),bool (*_checkEnabledFunction)(),const char*, const char* ,const char* ,int);
    int AddItem(void (*_CallBackFunction)(),bool (*_checkEnabledFunction)(),const char*, const char* ,const char* ,int, bool (*_checkStatusFunction)());
    int AddItem(void (*_CallBackFunction)(),bool (*_checkEnabledFunction)(),const char*, const char* ,const char* ,EasyNextionMenus*);
    int AddItem(void (*_CallBackFunction)(),bool (*_checkEnabledFunction)(),const char*, const char* ,const char* ,int, int);
    int AddItem(void (*_CallBackFunction)(),bool (*_checkEnabledFunction)(),const char*, const char* ,const char* ,int, const char*);

    void Reinitialize(void);

    Menu_Item *GetItemRef(int);
    Menu_Item *GetItemRef(void);

    void MenuDisplay(bool);
    void Refresh();
    void Select(uint8_t);

    Menu_Item menu_items[MAX_MENU_ITEMS];
    int selected_item = 0;
    
    private:
        void WriteNextionItemStr(const char *,int, const char*);
        void WriteNextionItemNum(const char *,int, int);
        void WriteNextionItemVisible(const char * ,int , int );
        void WriteNextionItemEnabled(const char * ,int , int );
        void WriteNextionItemAttribute(const char * ,int , const char* ,int );
        void WriteNextionItemAttribute(const char * ,int , const char* ,const char* );
        //void WriteNextionItemAttributeNum(const char * ,int , const char* ,int );
        //void WriteNextionItemAttributeStr(const char * ,int , const char* ,const char*  );
        void WriteNextionItemCmd(const char * ,int , const char* ,int  );
        void WriteNextionExecute(const char *);
        
        const char* menu_nxt_obj_name = nullptr;
        const char* menu_nxt_check_name = nullptr;
        const char* menu_nxt_sep_name = nullptr;
        int number_of_recorded_items = 0;
        int max_number_of_items = 0;
        int menu_type = ENM_MAIN;
        bool must_draw_overlay = false; // If subitem is an overlay page, must write all the main menu on the left prior to selecting it.
        void (*DisplayCallBack) () = nullptr;
        EasyNex *nextion_display = nullptr;
};

#endif