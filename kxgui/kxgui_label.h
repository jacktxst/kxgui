//
// Created by jack lewis on 1/18/26.
//

#ifndef KXGUI_TEXT_H
#define KXGUI_TEXT_H

#include "kxgui.h"
#include "kxgui_font.h"

const fvec2 text_scale = {2,4};

static void kxgui_label(const char * text)
{
    KXGUI_GETCTX()
    const u32 len = strlen(text);
    kxgui_begin_component((fvec2){len * KXGUI_CHAR_W * text_scale.x, KXGUI_CHAR_H * text_scale.y});
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
    kxgui_end_component();
}

#endif //KXGUI_TEXT_H
