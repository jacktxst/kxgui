//
// Created by jack lewis on 1/18/26.
//

#ifndef KXGUI_TEXTFIELD_H
#define KXGUI_TEXTFIELD_H

#include "kxgui.h"
#include "kxgui_font.h"

typedef struct  {
  unsigned num_lines;
  char ** lines;
  unsigned * lengths;
  unsigned longest;
  int cursor_x;
  int cursor_y;
  u8 focused;
  ivec2 rect; // unused?
} textfield_t;

static void kxgui_textfield(textfield_t * tf) {
  
  KXGUI_GETCTX();
  
  /* initialization */
  if (!tf->num_lines) {
    tf->focused = 1;
    tf->lines = malloc(sizeof(char *));
    tf->lines[0] = malloc(1);
    tf->lines[0][0] = '\0';
    tf->lengths = calloc(1, sizeof(unsigned));
    tf->num_lines = 1;
  }

  /* rendering - pretty solid */
  float charw = KXGUI_CHAR_W * text_scale.x;
  float charh = KXGUI_CHAR_H * text_scale.y;

  fvec2 textfield_size = {tf->longest * KXGUI_CHAR_W * text_scale.x, tf->num_lines * KXGUI_CHAR_H * text_scale.y};
  textfield_size.x = textfield_size.x > charw ? textfield_size.x : charw;
  textfield_size.y = textfield_size.y > charh ? textfield_size.y : charh;


  kxgui_begin_component(textfield_size);
  printf("x: %f y: %f\n", textfield_size.x, textfield_size.y);

  for (int i = 0; i < tf->num_lines; i++) {
    
      u32 len = strlen(tf->lines[i]);
      for (int j = 0; j < len; j++)
      {
          /* put char */
          const u32 glyph_index = tf->lines[i][j] - 32;
          const u32 col = glyph_index % KXGUI_FONT_COLS;
          const u32 row = glyph_index / KXGUI_FONT_COLS;
          const fvec2 glyph_size = {1.0 / KXGUI_FONT_COLS, 1.0 / KXGUI_FONT_ROWS};
          const fvec2 uv_offset = {col * glyph_size.x, row * glyph_size.y};
          kxgui_z(0.9);
          kxgui_rect(j * KXGUI_CHAR_W * text_scale.x, i * KXGUI_CHAR_H * text_scale.y, KXGUI_CHAR_W * text_scale.x, KXGUI_CHAR_H * text_scale.y);
          kxgui_fill_texture(1, 0, glyph_size, uv_offset);
      }

      if (tf->focused && tf->cursor_y == i)
      {
          kxgui_z(0.5);
          kxgui_rect(tf->cursor_x * KXGUI_CHAR_W * text_scale.x, tf->cursor_y * KXGUI_CHAR_H * text_scale.y, 15, KXGUI_CHAR_H * text_scale.y);
          kxgui_fill_color((fvec4){1.0,1.0,1.0,1.0});
      }
  }
  
  /* mouse interaction */
  static u8 wasMouseClicked = 0;

  

  if (kxgui_mouse_inside(0, 0, textfield_size.x, textfield_size.y)) {
    if (ctx->mouse_down && !wasMouseClicked) {

      fvec2 mpos = kxgui_mouse_position();
      
      tf->cursor_y = (mpos.y) / (KXGUI_CHAR_H*text_scale.y);
      tf->cursor_x = (mpos.x) / (KXGUI_CHAR_W*text_scale.x);
      if (tf->cursor_x > tf->lengths[tf->cursor_y]) tf->cursor_x = tf->lengths[tf->cursor_y]; 
      tf->focused = 1;
    }
    wasMouseClicked = 0;
    if (ctx->mouse_down) wasMouseClicked = 1;
  } else if (ctx->mouse_down && !wasMouseClicked) {tf->focused  = 0;}

  if (!tf->focused) {kxgui_end_component(); return;}
  /* actions performed while focused */
  unsigned row = tf->cursor_y;
  unsigned col = tf->cursor_x;
  
  unsigned length = tf->lengths[row];
  if (length > tf->longest) tf->longest = length;
  
  /* typing a normal character */
  if (ctx->typed_char && ctx->typed_char != '\r' && ctx->typed_char != '\n' && ctx->typed_char != '\b' && ctx->typed_char != '\x7F' ) {
    tf->lines[row] = (char *) realloc(tf->lines[row], length + 2); 
    memmove(tf->lines[row] + tf->cursor_x + 1, tf->lines[row] + tf->cursor_x, length + 1 - tf->cursor_x);
    (tf->lines[row])[tf->cursor_x] = ctx->typed_char;
    tf->cursor_x++; 
    tf->lengths[row]++;
    length++;
  }
  
  /* typing a backspace */
  if (ctx->keys[SAPP_KEYCODE_BACKSPACE] || ctx->keys[SAPP_KEYCODE_DELETE]) {
    if (tf->cursor_x > 0) {
      tf->cursor_x --;
      ctx->keys[SAPP_KEYCODE_BACKSPACE] = 0;
      ctx->keys[SAPP_KEYCODE_DELETE] = 0;
      memmove(tf->lines[row] + tf->cursor_x, tf->lines[row] + tf->cursor_x + 1, length - tf->cursor_x);
      tf->lines[row] = (char *) realloc(tf->lines[row], length);
      tf->lengths[row]--;
      length--;
    } else if (tf->cursor_y > 0){
      ctx->keys[SAPP_KEYCODE_BACKSPACE] = 0;
      ctx->keys[SAPP_KEYCODE_DELETE] = 0;
      unsigned prev_len = tf->lengths[row - 1];
      unsigned cur_len = tf->lengths[row];
      tf->lines[row - 1] = realloc(tf->lines[row - 1], prev_len + cur_len + 1);
      memcpy(tf->lines[row - 1] + prev_len, tf->lines[row], cur_len);
      tf->lines[row - 1][prev_len + cur_len] = '\0';
      tf->lengths[row - 1] = prev_len + cur_len;
      free(tf->lines[row]);
      memmove(tf->lines + row, tf->lines + row + 1, (tf->num_lines - row - 1) * sizeof(char *));
      memmove(tf->lengths + row, tf->lengths + row + 1, (tf->num_lines - row - 1) * sizeof(unsigned));
      tf->num_lines--;
      tf->cursor_y--;
      tf->cursor_x = prev_len;
      row = tf->cursor_y;
      col = tf->cursor_x;
      length = tf->lengths[row];
    }
    /* recalculate longest line */
    u32 new_longest = 0;
    for (u32 line_i = 0; line_i < tf->num_lines; line_i++) {
       u32 line_len = strlen(tf->lines[line_i]);
       new_longest = line_len > new_longest ? line_len : new_longest;
    }
    tf->longest = new_longest;
  }
  
  /* newline */
  if (ctx->keys[SAPP_KEYCODE_ENTER]) {
    unsigned tail = length - tf->cursor_x;
    tf->num_lines++;
    tf->lines = realloc(tf->lines, tf->num_lines * sizeof(char *));
    tf->lengths = realloc(tf->lengths, tf->num_lines * sizeof(unsigned));
    if (row + 1 < tf->num_lines - 1) {
      memmove(tf->lines + row + 2, tf->lines + row + 1, (tf->num_lines - row - 2) * sizeof(char *));
      memmove(tf->lengths + row + 2, tf->lengths + row + 1, (tf->num_lines - row - 2) * sizeof(unsigned));
    }
    tf->lines[row + 1] = malloc(tail + 1);
    memmove(tf->lines[row + 1], tf->lines[row] + tf->cursor_x, tail);
    tf->lines[row + 1][tail] = '\0';
    tf->lines[row][tf->cursor_x] = '\0';
    tf->lines[row] = realloc(tf->lines[row], tf->cursor_x + 1);
    tf->lengths[row] = tf->cursor_x;
    tf->lengths[row + 1] = tail;
    tf->cursor_x = 0;
    tf->cursor_y++;
    ctx->keys[SAPP_KEYCODE_ENTER] = 0;
  }
  if (ctx->keys[SAPP_KEYCODE_RIGHT] && tf->cursor_x < length ) { tf->cursor_x ++; ctx->keys[SAPP_KEYCODE_RIGHT] = 0; } 
  if (ctx->keys[SAPP_KEYCODE_LEFT] && tf->cursor_x > 0) {tf->cursor_x --; ctx->keys[SAPP_KEYCODE_LEFT] = 0;}
  if (ctx->keys[SAPP_KEYCODE_UP] && tf->cursor_y > 0) {
    tf->cursor_y --;
    if (tf->cursor_x > tf->lengths[row-1]) tf->cursor_x = tf->lengths[row-1];
    ctx->keys[SAPP_KEYCODE_UP] = 0;
  }
  if (ctx->keys[SAPP_KEYCODE_DOWN] && tf->cursor_y+1 < tf->num_lines) {
    tf->cursor_y ++;
    if (tf->cursor_x > tf->lengths[row+1]) tf->cursor_x = tf->lengths[row+1];
    ctx->keys[SAPP_KEYCODE_DOWN] = 0;
  }

    kxgui_end_component();
  
}

#endif //KXGUI_TEXTFIELD_H
