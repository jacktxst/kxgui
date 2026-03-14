//
// Created by jack lewis on 1/22/26.
//

// this is the implementation of a very rudimentary pixel image editor 

#ifndef KXGUI_PAINTER_H
#define KXGUI_PAINTER_H

#include "kxgui.h"

typedef struct  
{
    u8 focused;
    u32 layer;
    u32 texture_array_handle;
    Bitmap32 image;
    fvec4 color;
    kxgui_container container;
} 
kxgui_painter_t;

void kxgui_painter(kxgui_painter_t * this)
{
    KXGUI_GETCTX()  
    
    this->container.internal = (fvec2){500, 500};
    this->container.external = (fvec2){500, 500};
    this->container.bg_color = (fvec4){0.1, 0.1, 0.1, 1.0};
    
    kxgui_begin_container(&(this->container));
        
        // change layer
        // add layer
        // create layer
        // copy entire layer
        // paste entire layer
    
        kxgui_begin_component((fvec2){16,16});
            if (kxgui_mouse_inside(0, 0, 16, 16))
            {
                if (ctx->mouse_pressed)
                {
                    this->focused = 1;
                }  
            } 
            else 
            { 
                if (ctx->mouse_pressed)
                {
                    this->focused = 0;
                }  
            }
            
        kxgui_rect(0, 0, 16, 16);
        kxgui_fill_texture(this->texture);
        kxgui_end_component();
    
    kxgui_end_container();
}

#endif //KXGUI_PAINTER_H
