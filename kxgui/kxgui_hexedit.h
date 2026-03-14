//
// Created by jack lewis on 1/20/26.
//

#ifndef KXGUI_HEXEDIT_H
#define KXGUI_HEXEDIT_H

typedef struct
{
  void * data;
  int focused;
  int cursor_cell;
  int cursor_i;
  size_t size;
  int row_length;
}
kxgui_hexedit_t;

char *hexconvertbyte[256] = {
  "00","01","02","03","04","05","06","07","08","09","0A","0B","0C","0D","0E","0F",
  "10","11","12","13","14","15","16","17","18","19","1A","1B","1C","1D","1E","1F",
  "20","21","22","23","24","25","26","27","28","29","2A","2B","2C","2D","2E","2F",
  "30","31","32","33","34","35","36","37","38","39","3A","3B","3C","3D","3E","3F",
  "40","41","42","43","44","45","46","47","48","49","4A","4B","4C","4D","4E","4F",
  "50","51","52","53","54","55","56","57","58","59","5A","5B","5C","5D","5E","5F",
  "60","61","62","63","64","65","66","67","68","69","6A","6B","6C","6D","6E","6F",
  "70","71","72","73","74","75","76","77","78","79","7A","7B","7C","7D","7E","7F",
  "80","81","82","83","84","85","86","87","88","89","8A","8B","8C","8D","8E","8F",
  "90","91","92","93","94","95","96","97","98","99","9A","9B","9C","9D","9E","9F",
  "A0","A1","A2","A3","A4","A5","A6","A7","A8","A9","AA","AB","AC","AD","AE","AF",
  "B0","B1","B2","B3","B4","B5","B6","B7","B8","B9","BA","BB","BC","BD","BE","BF",
  "C0","C1","C2","C3","C4","C5","C6","C7","C8","C9","CA","CB","CC","CD","CE","CF",
  "D0","D1","D2","D3","D4","D5","D6","D7","D8","D9","DA","DB","DC","DD","DE","DF",
  "E0","E1","E2","E3","E4","E5","E6","E7","E8","E9","EA","EB","EC","ED","EE","EF",
  "F0","F1","F2","F3","F4","F5","F6","F7","F8","F9","FA","FB","FC","FD","FE","FF"
};


static void kxgui_hexedit(kxgui_hexedit_t * hexedit)
{
    KXGUI_GETCTX();
    
    int row_length = hexedit->row_length;
    int focused = hexedit->focused;
    int cursor_i= hexedit->cursor_i;
    int cursor_cell = hexedit->cursor_cell;
    void * data = hexedit->data;
    size_t size = hexedit->size;

    int rows = size / row_length + 1; 
    fvec2 component_size = {row_length * 3 * KXGUI_CHAR_W * text_scale.x, rows * KXGUI_CHAR_H * text_scale.y};
    kxgui_begin_component(component_size);

    int i_row = 0, i_col = 0;
    /* render */
    for (int i = 0; i < size; i++) {
      char hex_byte[3] = {
        hexconvertbyte[((unsigned char *)data)[i]][0],
        hexconvertbyte[((unsigned char *)data)[i]][1],
        ' '
      };
      for (int j = 0; j < 3; j++) {
        const u32 glyph_index = hex_byte[j] - 32;
        const u32 col = glyph_index % KXGUI_FONT_COLS;
        const u32 row = glyph_index / KXGUI_FONT_COLS;
        const fvec2 glyph_size = {1.0 / KXGUI_FONT_COLS, 1.0 / KXGUI_FONT_ROWS};
        const fvec2 uv_offset = {col * glyph_size.x, row * glyph_size.y};
        kxgui_z(0.9);
        kxgui_rect((i_col*3 + j) * KXGUI_CHAR_W * text_scale.x, i_row * KXGUI_CHAR_H * text_scale.y, KXGUI_CHAR_W * text_scale.x, KXGUI_CHAR_H * text_scale.y);
        kxgui_fill_texture(1, 0, glyph_size, uv_offset);
        if (focused && cursor_i == i && cursor_cell == j) {
          kxgui_z(0.5);
          kxgui_rect((i_col*3 + j) * KXGUI_CHAR_W * text_scale.x, i_row * KXGUI_CHAR_H * text_scale.y, KXGUI_CHAR_W * text_scale.x, KXGUI_CHAR_H * text_scale.y);
          kxgui_fill_color((fvec4){1,1,1,1});
        }
      }
      
      if (++i_col == row_length) {
        i_col = 0;
        ++i_row;
      }
      
    }

    /* mouse interaction */
    static int wasMouseClicked = 0;
    if (kxgui_mouse_inside(0, 0, component_size.x, component_size.y)) {
        if (ctx->mouse_down && !wasMouseClicked) {

            printf("test1\n");
            
            fvec2 mpos = kxgui_mouse_position();

            printf("test2\n");
            
            int cursor_y = (mpos.y) / (KXGUI_CHAR_H*text_scale.y);
            int cursor_x = (mpos.x) / (KXGUI_CHAR_W*text_scale.x);

            printf("%d %d\n", cursor_y, cursor_x);

            hexedit->cursor_cell = cursor_x % 3;

          
            
            if (hexedit->cursor_cell == 2) hexedit->cursor_cell = 1;
            
            hexedit->cursor_i = cursor_y * hexedit->row_length + cursor_x / 3;
          
            hexedit->focused = 1;
        }
        wasMouseClicked = 0;
        if (ctx->mouse_down) wasMouseClicked = 1;
    } else if (ctx->mouse_down && !wasMouseClicked) {hexedit->focused  = 0;;}
  
    /* actions performed while focused */

    if (!focused) {kxgui_end_component(); return;}
    int should_advance = 0;
    if ('0' <= ctx->typed_char && ctx->typed_char <= '9' ) {
        
        u8 nybble_value = ctx->typed_char - '0';
        should_advance = 1;
        
        u8 nyb1 = cursor_cell ? ((u8*)data)[cursor_i]>>4 : nybble_value;
        u8 nyb2 = cursor_cell ? nybble_value : ((u8*)data)[cursor_i] & 0x0F;
            
        ((u8*)data)[cursor_i] = nyb1<<4 | nyb2;
    }
    if ('A' <= ctx->typed_char && ctx->typed_char <= 'F' ) {
        u8 nybble_value = ctx->typed_char - 'A' + 10;
        should_advance = 1;

        u8 nyb1 = cursor_cell ? ((u8*)data)[cursor_i]>>4 : nybble_value;
        u8 nyb2 = cursor_cell ? nybble_value : ((u8*)data)[cursor_i] & 0x0F;
            
        ((u8*)data)[cursor_i] = nyb1<<4 | nyb2;
    }
    if ('a' <= ctx->typed_char && ctx->typed_char <= 'f' ) {
        u8 nybble_value = ctx->typed_char - 'a' + 10;
        should_advance = 1;


        u8 nyb1 = cursor_cell ? ((u8*)data)[cursor_i]>>4 : nybble_value;
        u8 nyb2 = cursor_cell ? nybble_value : ((u8*)data)[cursor_i] & 0x0F;
            
        ((u8*)data)[cursor_i] = nyb1<<4 | nyb2;
        
    }

    if (should_advance) {
        ctx->typed_char = 0;
        if (cursor_cell == 1)
        {
            if (cursor_i + 1 == size)
            {
                // do nothing
            }
            else
            {
                hexedit->cursor_i++;
                hexedit->cursor_cell = 0;
            }
        }
        else
        {
            hexedit->cursor_cell++;    
        }
    }

    if (ctx->keys[SAPP_KEYCODE_DOWN]) {
        ctx->keys[SAPP_KEYCODE_DOWN] = 0;
        if (cursor_i + row_length < size) {
            hexedit->cursor_i += row_length;
        }  
    }
    else if (ctx->keys[SAPP_KEYCODE_UP]) {
        ctx->keys[SAPP_KEYCODE_UP] = 0;
        if (cursor_i - row_length >= 0) {
            hexedit->cursor_i -= row_length;
        }  
    }
    else if (ctx->keys[SAPP_KEYCODE_LEFT]) {
        ctx->keys[SAPP_KEYCODE_LEFT] = 0;
        if (cursor_cell == 0) {
            if (cursor_i == 0) {
            // do nothing
            } else {
                hexedit->cursor_i--;
                hexedit->cursor_cell = 1;
            }
        } else {
          hexedit->cursor_cell--;
        }
    }
    else if (ctx->keys[SAPP_KEYCODE_RIGHT])
    {
        ctx->keys[SAPP_KEYCODE_RIGHT] = 0;
        if (cursor_cell == 1)
        {
            if (cursor_i + 1 == size)
            {
              // do nothing
            }
            else
            {
                hexedit->cursor_i++;
                hexedit->cursor_cell = 0;
            }
        }
        else
        {
            hexedit->cursor_cell++;
        }
    }
    kxgui_end_component();
}

#endif //KXGUI_HEXEDIT_H
