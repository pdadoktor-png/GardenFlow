#pragma once
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_font_t lv_font_de_14;

#ifdef __cplusplus
}
#endif

static inline const lv_font_t* gardenFlowFont()
{
    return &lv_font_de_14;
}
