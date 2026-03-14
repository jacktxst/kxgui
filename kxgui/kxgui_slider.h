//
// Created by jack lewis on 1/17/26.
//

#ifndef KXGUI_SLIDER_H
#define KXGUI_SLIDER_H

#include "kxgui.h"

static void kxgui_slider_float(float * value, float min, float max) {

    KXGUI_GETCTX();
    
    float thumb_width = 32;
    float slider_height = 32;
    float slider_width = 128;

    kxgui_begin_component((fvec2){slider_width,slider_height});
    
    if (ctx->mouse_down && kxgui_mouse_inside(0, 0, slider_width, slider_height)) {
        
        float mouse_x = kxgui_mouse_position().x;
        float thumb_x = mouse_x - thumb_width / 2;

        thumb_x = thumb_x < 0 ? 0 : thumb_x;
        thumb_x = thumb_x > (slider_width - thumb_width) ? slider_width - thumb_width : thumb_x;

        *value = (max-min)*thumb_x/(slider_width-thumb_width)+min;
        
    }
    
    kxgui_rect(0, 0, slider_width, slider_height);
    kxgui_z(0.1);
    kxgui_fill_color((fvec4){1,1,1,1});

    kxgui_z(0);
    kxgui_rect(
        (*value - min)/(max-min) * (slider_width-thumb_width),
        0,
        thumb_width,
        slider_height);
    
    if (kxgui_mouse_inside(0, 0 , slider_width, slider_height)) {
        kxgui_fill_color((fvec4){1.0,0.0,0.0,1.0});
    }
    else kxgui_fill_color((fvec4){0.0,0.0,0.5,1.0});
    
    
    kxgui_end_component();
    
}


#endif //KXGUI_SLIDER_H
