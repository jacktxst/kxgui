//
// Created by jack lewis on 1/18/26.
//

#ifndef KXGUI_SOKOL_H
#define KXGUI_SOKOL_H

#include "kxcore.h"

u32 gui_vcount;

typedef struct vertex
{
    fvec3 position;
    fvec4 color;
    fvec2 uv;
    float textured;
}
kxgui_sokol_vertex;

typedef struct vertex_list
{
    struct vertex * data;
    size_t size;
    size_t capacity;
}
kxgui_sokol_vertex_list;

// converts screen coordinate rectangle to normalized device coordinates for rendering
// very bad function
void quad(kxgui_sokol_vertex_list * dest, const fvec4 rect, const fvec4 color, float z, u8 textured, fvec2 uvsc, fvec2 uvof) {
    const fvec2 SCREEN_SIZE = {sapp_widthf(),sapp_heightf()};
    // hmm. this works but is not so great. need a vector math lib
    fvec2 a = (fvec2){rect.x,rect.y};
    fvec2 b = (fvec2){rect.x + rect.z, rect.y};
    fvec2 c = (fvec2){rect.x + rect.z, rect.y + rect.w};
    fvec2 d = (fvec2){rect.x             , rect.y + rect.w};
    fvec2 a_ndc = fvec2_to_ndc(a,(fvec2){SCREEN_SIZE.x,SCREEN_SIZE.y});
    fvec2 b_ndc = fvec2_to_ndc(b,(fvec2){SCREEN_SIZE.x,SCREEN_SIZE.y});
    fvec2 c_ndc = fvec2_to_ndc(c,(fvec2){SCREEN_SIZE.x,SCREEN_SIZE.y});
    fvec2 d_ndc = fvec2_to_ndc(d,(fvec2){SCREEN_SIZE.x,SCREEN_SIZE.y});
    if (dest->size + 6 >= dest->capacity) {
        exit(0);
    }
    dest->data[dest->size++] = (kxgui_sokol_vertex){{a_ndc.x, a_ndc.y, z}, color, {0+uvof.x,0+uvof.y}, textured};
    dest->data[dest->size++] = (kxgui_sokol_vertex){{b_ndc.x, b_ndc.y, z}, color, {1*uvsc.x+uvof.x,0+uvof.y}, textured};
    dest->data[dest->size++] = (kxgui_sokol_vertex){{c_ndc.x, c_ndc.y, z}, color, {1*uvsc.x+uvof.x,1*uvsc.y+uvof.y}, textured};
    dest->data[dest->size++] = (kxgui_sokol_vertex){{c_ndc.x, c_ndc.y, z}, color, {1*uvsc.x+uvof.x,1*uvsc.y+uvof.y}, textured};
    dest->data[dest->size++] = (kxgui_sokol_vertex){{d_ndc.x, d_ndc.y, z}, color, {0+uvof.x,1*uvsc.y+uvof.y}, textured};
    dest->data[dest->size++] = (kxgui_sokol_vertex){{a_ndc.x, a_ndc.y, z}, color, {0+uvof.x,0+uvof.y}, textured};
};


/* updates a vertex buffer to draw the gui every frame */
void kxgui_sokol_writebuffer(kxgui_context * ctx) {
    
    /* RANDOM BOXES TEST GUI
    kxgui_example_component((ivec2){100,50},(fvec4){1.0, 0.0, 0.0, 1.0});
    kxgui_example_component((ivec2){50,100},(fvec4){0.0, 1.0, 0.0, 1.0});
    kxgui_container_flags(KXGUI_CONTAINER_SCROLLABLE);
    static fvec2 container_scroll;
    kxgui_begin_container(&(fvec2){100,400},&(fvec2){100,100},(fvec4){0,0,1,1}, &container_scroll);
        kxgui_example_component((ivec2){25,30},(fvec4){0.0, 1.0, 1.0, 1.0});
        kxgui_example_component((ivec2){30,25},(fvec4){1.0, 1.0, 0.0, 1.0});
        static fvec2 container2_scroll;
        kxgui_begin_container(&(fvec2){50,400},&(fvec2){50,50},(fvec4){0,1,1,1}, &container2_scroll);
            kxgui_example_component((ivec2){15,15},(fvec4){1.0, 1.0, 0.0, 1.0});
            kxgui_example_component((ivec2){15,15},(fvec4){1.0, 1.0, 0.0, 1.0});
            kxgui_example_component((ivec2){15,15},(fvec4){1.0, 1.0, 0.0, 1.0});
            kxgui_example_component((ivec2){15,15},(fvec4){1.0, 1.0, 0.0, 1.0});
            kxgui_example_component((ivec2){15,15},(fvec4){1.0, 1.0, 0.0, 1.0});
        kxgui_end_container();
        kxgui_example_component((ivec2){30,25},(fvec4){1.0, 1.0, 0.0, 1.0}); 
    kxgui_end_container();
    kxgui_example_component((ivec2){50,25},(fvec4){1.0, 1.0, 1.0, 1.0});
    */
    
    /* kxgui / sokol renderer */
    
    static kxgui_sokol_vertex_list vl = {
        .data = 0,
        .size = 0,
        .capacity = 8192
    };
    if (!vl.data) {vl.data = malloc(sizeof(kxgui_sokol_vertex)*8192);}
    vl.size = 0;
    
    for (int i = 0; i < ctx->_render_output.n; i++) {
        struct kxgui_render_cmd cmd = ctx->_render_output.commands[i];
        struct kxgui_rect rect = cmd.rect;
        quad
        (
            &vl,
            (fvec4){rect.x, rect.y, rect.width, rect.height},
            ctx->_render_output.commands[i].color,
            ctx->_render_output.commands[i].z,
            cmd.texture,
            cmd.uv_scale,
            cmd.uv_offset
        );
    }
    gui_vcount = vl.size;
    sg_update_buffer(state.bind.vertex_buffers[0], &(sg_range){vl.data, vl.size * sizeof(kxgui_sokol_vertex)});
}

void kxgui_sokol_create_font_texture()
{
    Bitmap32 bmp = kxgui_font_atlas_bmp32(0x0, 0xFFFFFFFF);
    sg_image img = sg_make_image(&(sg_image_desc){
        .width = bmp.width,
        .height = bmp.height,
        .data.mip_levels[0] = (sg_range){bmp.pixels, bmp.width * bmp.height * sizeof(u32)},
        .label = "texcube-image",
    });
    state.bind.views[VIEW_tex] = sg_make_view(&(sg_view_desc){
        .texture = { .image = img },
        .label = "texcube-texture-view",
    });
    state.bind.samplers[SMP_smp] = sg_make_sampler(&(sg_sampler_desc){
        .label = "texcube-sampler"
    });
}

#endif //KXGUI_SOKOL_H
