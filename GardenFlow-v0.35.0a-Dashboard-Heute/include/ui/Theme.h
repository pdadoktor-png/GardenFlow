#pragma once
#include <lvgl.h>

namespace Theme
{
    inline lv_color_t background() { return lv_color_hex(0x0C1218); }
    inline lv_color_t header()     { return lv_color_hex(0x111B24); }
    inline lv_color_t panel()      { return lv_color_hex(0x17232D); }
    inline lv_color_t panelAlt()   { return lv_color_hex(0x1D2B36); }
    inline lv_color_t border()     { return lv_color_hex(0x344955); }
    inline lv_color_t text()       { return lv_color_hex(0xF4F7F8); }
    inline lv_color_t textDim()    { return lv_color_hex(0x9FB0BA); }
    inline lv_color_t open()       { return lv_color_hex(0x20B26B); }
    inline lv_color_t closed()     { return lv_color_hex(0xD84A4A); }
    inline lv_color_t pulse()      { return lv_color_hex(0xF0A12A); }
    inline lv_color_t accent()     { return lv_color_hex(0x4DA3FF); }
    constexpr int CARD_RADIUS = 12;
}
