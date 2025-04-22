#include "Helpers.h"


// Removes the duplicates "/" from the string provided as an argument
void remove_duplicates_slash(char *str)
{
  int i,j,len,len1;

  /*calculating length*/
  for(len=0; str[len]!='\0'; len++);

  /*assign 0 to len1 - length of removed characters*/
  len1=0;

  /*Removing consecutive repeated characters from string*/
  for(i=0; i<(len-len1);)
  {
      if((str[i]==str[i+1]) && (str[i] == '/'))
      {
          /*shift all characters*/
          for(j=i;j<(len-len1);j++)
              str[j]=str[j+1];
          len1++;
      }
      else
      {
          i++;
      }
  }
}

// count number of items in a list
// the end of a list has a nullptr
uint8_t Helpers::count_items(const char * const * list) {
    uint8_t list_size = 0;
    if (list != nullptr) {
        while (list[list_size]) {
            list_size++;
        }
    }
    return list_size;
}

// count number of items in a list of lists
// the end of a list has a nullptr
uint8_t Helpers::count_items(const char * const ** list) {
    uint8_t list_size = 0;
    if (list != nullptr) {
        while (list[list_size]) {
            list_size++;
        }
    }
    return list_size;
}


// returns char pointer to translated description or fullname
// if force_en is true always take the EN non-translated word
const char * Helpers::translated_word(const char * const * strings, uint8_t language_index) {
    uint8_t index          = 0; // default en

    if (!strings) {
        return ""; // no translations
    }

    // see how many translations we have for this entity. if there is no translation for this, revert to EN
    if (Helpers::count_items(strings) >= language_index + 1 && strlen(strings[language_index])) {
        index = language_index;
    }

    return strings[index];
}
