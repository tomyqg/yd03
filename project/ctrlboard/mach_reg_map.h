#include "global.h"

static bool reg_voice_write(uint8_t *content, reg_item_t reg)
{
    if (reg == 0x20) level0 = *content;
    if (reg == 0x21) level1 = *content;
    return true;
}

static bool reg_voice_read(uint8_t *content, reg_item_t reg)
{
    *content = reg;
    return true;
}

const reg_map_t mach_reg_map[] = {
    {0x20, reg_voice_read, reg_voice_write},
    {0x21, reg_voice_read, reg_voice_write},
};
