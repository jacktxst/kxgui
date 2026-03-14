//
// Created by jack lewis on 1/22/26.
//

#ifndef KXGUI_BUTTON_H
#define KXGUI_BUTTON_H

#include "kxgui.h"
#include <string.h>

fvec2 kxgui_text_size(u32 len) {
  return (fvec2){len * KXGUI_CHAR_W * text_scale.x, KXGUI_CHAR_H * text_scale.y};
}

void kxgui_text(char * text, u32 len) {
    for (int i = 0; i < len; i++)
    {
        const u32 glyph_index = text[i] - 32;
        const u32 col = glyph_index % KXGUI_FONT_COLS;
        const u32 row = glyph_index / KXGUI_FONT_COLS;
        const fvec2 glyph_size = {1.0 / KXGUI_FONT_COLS, 1.0 / KXGUI_FONT_ROWS};
        const fvec2 uv_offset = {col * glyph_size.x, row * glyph_size.y};
        kxgui_rect(i * KXGUI_CHAR_W * text_scale.x, 0, KXGUI_CHAR_W * text_scale.x, KXGUI_CHAR_H * text_scale.y);
        kxgui_fill_texture(1, 0, glyph_size, uv_offset);
    }
}

u8 kxgui_button_once(char * text){
    KXGUI_GETCTX()
    const u32 len = strlen(text);
    const fvec4 bg_color = {0,0.5,0.4,1.0};
    const fvec4 bg_color_hover = {0.7, 0.7, 0.7, 1.0};
    fvec2 text_size = kxgui_text_size(len);
    kxgui_begin_component(text_size);
        kxgui_z(0.6);
        kxgui_rect(0,0,text_size.x, text_size.y);
        if (kxgui_mouse_inside(0, 0, text_size.x, text_size.y))
        {
            kxgui_fill_color(bg_color_hover);
        }
        else
        {
            kxgui_fill_color(bg_color);
        }
        kxgui_z(0.5);
        kxgui_text(text, len);
    kxgui_end_component();
    return 0;
}

#endif //KXGUI_BUTTON_H
