#ifndef POOLMASTER_HELPERS_H
#define POOLMASTER_HELPERS_H

#include "PoolMaster.h"

class Helpers {
  public:
    static uint8_t count_items(const char * const ** list);
    static uint8_t count_items(const char * const * list);

    static const char * translated_word(const char * const * strings, uint8_t language_index);
};


#endif