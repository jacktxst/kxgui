//
// Created by jack lewis on 1/18/26.
//

// this is a hardware accelerated gui renderer that sits between kxgui and sokol_gfx.h


#ifndef KXGUI_SOKOL_H
#define KXGUI_SOKOL_H
#define KXGUI_SOKOL_TEXTURE_MAX 256
#define KXGUI_SOKOL_MAX_VERTS_PER_BATCH 8192
#include "kxcore.h"
#include "kx_sokol_texture.h"

typedef struct kxgui_sokol_vertex
{
    fvec3 position;
    fvec4 color;
    fvec2 uv;
    float textured;
    float tex_slice;
}
kxgui_sokol_vertex;

typedef struct kxgui_sokol_vertex_list
{
    kxgui_sokol_vertex * data;
    size_t size;
    size_t capacity;
}
kxgui_sokol_vertex_list;

struct
{
    sg_bindings bind;
    sg_pipeline pip;
    sg_pass_action pass_action;
}
kxgui_sokol_render_state;

struct
{
    TextureHandle           texture_handles[KXGUI_SOKOL_TEXTURE_MAX];
    sg_buffer               buffers[KXGUI_SOKOL_TEXTURE_MAX];
    u8                      texture_is_array[KXGUI_SOKOL_TEXTURE_MAX];
    u32                     texture_slots_used;
    kxgui_sokol_vertex_list vertex_lists[KXGUI_SOKOL_TEXTURE_MAX];
}
kxgui_sokol_batches = {0};

TextureHandle kxgui_sokol_fallback_texture;
TextureHandle kxgui_sokol_fallback_texture_array;

void kxgui_sokol_init()
{
    Bitmap32 font_bitmap32 = kxgui_font_atlas_bmp32(0x0, 0xFFFFFFFF);
    TextureHandle font_texture = Texture_Create(font_bitmap32);

    Bitmap32 white = {
        .width = 1,
        .height = 1,
        .pixels = (u32[]){0xFFFFFFFF}
    };

    Bitmap32_Array whitearr = {
        .width = 1,
        .height = 1,
        .num_slices = 1,
        .pixels = (u32[]){0xFFFFFFFF}
    };
    
    kxgui_sokol_fallback_texture = Texture_Create(white);
    kxgui_sokol_fallback_texture_array = TextureArray_Create(whitearr);
    
    kxgui_sokol_batches.texture_handles[0] = kxgui_sokol_fallback_texture;

    
    
    
    kxgui_sokol_render_state.bind.samplers[SMP_smp] = sg_make_sampler(&(sg_sampler_desc){
        .label = "texcube-sampler"
    });

    kxgui_sokol_render_state.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_LOAD
        }
    };
    
    /* slot 0 is reserved for untextured rectangles */
    kxgui_sokol_render_state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .size = KXGUI_SOKOL_MAX_VERTS_PER_BATCH * sizeof(kxgui_sokol_vertex),
        .usage.dynamic_update = true,
        .label = "vertex-buffer"
    });
    
    kxgui_sokol_batches.texture_slots_used = 1;
    kxgui_sokol_batches.buffers[0] = kxgui_sokol_render_state.bind.vertex_buffers[0];
    kxgui_sokol_batches.vertex_lists[0] = (kxgui_sokol_vertex_list){
        .data = malloc(KXGUI_SOKOL_MAX_VERTS_PER_BATCH * sizeof(kxgui_sokol_vertex)),
        .size = 0,
        .capacity = KXGUI_SOKOL_MAX_VERTS_PER_BATCH
    };
}

// converts screen coordinate rectangle to normalized device coordinates for rendering
// appends it to the vertex list
void kxgui_sokol_vertex_list_add_quad(kxgui_sokol_vertex_list * dest, const fvec4 rect, const fvec4 color, float z, float textured, float tex_slice, fvec2 uvsc, fvec2 uvof) {
    const fvec2 SCREEN_SIZE = {sapp_widthf(),sapp_heightf()};
    // hmm. this works but is not so great. need a vector math lib

    fvec2 a = (fvec2){rect.x,rect.y};
    fvec2 b = (fvec2){rect.x + rect.z, rect.y};
    fvec2 c = (fvec2){rect.x + rect.z, rect.y + rect.w};
    fvec2 d = (fvec2){rect.x             , rect.y + rect.w};

    fvec2 a_ndc = fvec2_to_ndc(a,SCREEN_SIZE);
    fvec2 b_ndc = fvec2_to_ndc(b,SCREEN_SIZE);
    fvec2 c_ndc = fvec2_to_ndc(c,SCREEN_SIZE);
    fvec2 d_ndc = fvec2_to_ndc(d,SCREEN_SIZE);

    if (dest->size + 6 >= dest->capacity) {
        printf("too many vertices\n");
        exit(0);
    }
    
    dest->data[dest->size++] = (kxgui_sokol_vertex){{a_ndc.x, a_ndc.y, z}, color, {0+uvof.x,0+uvof.y}, textured, tex_slice};
    dest->data[dest->size++] = (kxgui_sokol_vertex){{b_ndc.x, b_ndc.y, z}, color, {1*uvsc.x+uvof.x,0+uvof.y}, textured, tex_slice};
    dest->data[dest->size++] = (kxgui_sokol_vertex){{c_ndc.x, c_ndc.y, z}, color, {1*uvsc.x+uvof.x,1*uvsc.y+uvof.y}, textured, tex_slice};
    dest->data[dest->size++] = (kxgui_sokol_vertex){{c_ndc.x, c_ndc.y, z}, color, {1*uvsc.x+uvof.x,1*uvsc.y+uvof.y}, textured, tex_slice};
    dest->data[dest->size++] = (kxgui_sokol_vertex){{d_ndc.x, d_ndc.y, z}, color, {0+uvof.x,1*uvsc.y+uvof.y}, textured, tex_slice};
    dest->data[dest->size++] = (kxgui_sokol_vertex){{a_ndc.x, a_ndc.y, z}, color, {0+uvof.x,0+uvof.y}, textured, tex_slice};
};

// this function reads the output of kxgui, a list of kxgui_render_cmd.
// first, it creates vertex lists which are raw data that will be uploaded to the gpu
// 
// it will ensure that there is exactly 1 vertex list for each texture handle that is used
// in the gui at that time
// 
void kxgui_sokol_draw_pass(kxgui_context * ctx) {
    
    /* kxgui / sokol renderer */

    /* part 1 - generate data
     * the first task of this function is to generate a set
     *
     
     

    /* reset per-frame vertex counts */
    for (u32 i = 0; i < kxgui_sokol_batches.texture_slots_used; i++) {
        kxgui_sokol_batches.vertex_lists[i].size = 0;
    }
    
    for (int cmd_index = 0; cmd_index < ctx->_render_output.n; cmd_index++)
    {
        const kxgui_render_cmd cmd = ctx->_render_output.commands[cmd_index];
        const struct kxgui_rect rect = cmd.rect;

        u32 batch_index = 0;
        float textured_flag = 0.0f;
        float tex_slice = 0.0f;

        if (cmd.texture > 0) {
            /* determine texture kind (2D vs array) from unified texture API */
            kx_tex_type kind = Texture_GetKind(cmd.texture);
            textured_flag = (kind == KX_TEXKIND_ARRAY) ? 2.0f : 1.0f;
            if (kind == KX_TEXKIND_ARRAY) {
                tex_slice = (float)cmd.texture_slice;
            }

            i32 found_index = -1;
            for (u32 j = 1; j < kxgui_sokol_batches.texture_slots_used; j++) {
                if (kxgui_sokol_batches.texture_handles[j] == cmd.texture) {
                    found_index = (i32)j;
                    break;
                }
            }
            if (found_index == -1) {
                if (kxgui_sokol_batches.texture_slots_used < KXGUI_SOKOL_TEXTURE_MAX) {
                    batch_index = kxgui_sokol_batches.texture_slots_used++;
                    kxgui_sokol_batches.texture_handles[batch_index] = cmd.texture;
                    kxgui_sokol_batches.texture_is_array[batch_index] = (kind == KX_TEXKIND_ARRAY) ? 1 : 0;
                    kxgui_sokol_batches.vertex_lists[batch_index] = (kxgui_sokol_vertex_list){
                        .data = malloc(KXGUI_SOKOL_MAX_VERTS_PER_BATCH * sizeof(kxgui_sokol_vertex)),
                        .size = 0,
                        .capacity = KXGUI_SOKOL_MAX_VERTS_PER_BATCH
                    };

                    kxgui_sokol_batches.buffers[batch_index] =
                    sg_make_buffer(&(sg_buffer_desc){
                    .size = KXGUI_SOKOL_MAX_VERTS_PER_BATCH * sizeof(kxgui_sokol_vertex),
                    .usage.dynamic_update = true,
                    .label = "vertex-buffer"
                    });

                    
                } else {
                    /* out of batch slots, fall back to untextured batch */
                    batch_index = 0;
                    textured_flag = 0.0f;
                    tex_slice = 0.0f;
                }
            } else {
                batch_index = (u32)found_index;
            }
        } else {
            /* untextured rects always go into batch 0 */
            batch_index = 0;
            textured_flag = 0.0f;
            tex_slice = 0.0f;
        }
        
        kxgui_sokol_vertex_list_add_quad
        (
         &kxgui_sokol_batches.vertex_lists[batch_index],
         (fvec4){rect.x, rect.y, rect.width, rect.height},
         ctx->_render_output.commands[cmd_index].color,
         ctx->_render_output.commands[cmd_index].z_layer,
         textured_flag,
         tex_slice,
         cmd.uv_scale,
         cmd.uv_offset
        );
    }

    /* part 2 
     * separate each texture batch into buffers and draw
     * free any unused buffers!!!
     */

    sg_begin_pass(&(sg_pass){ .action = kxgui_sokol_render_state.pass_action, .swapchain = sglue_swapchain() });

    sg_apply_pipeline(state.pip);
    
    for (u32 batch_index = 0; batch_index < kxgui_sokol_batches.texture_slots_used; batch_index++)
    {
        kxgui_sokol_vertex_list v_list = kxgui_sokol_batches.vertex_lists[batch_index];

        kxgui_sokol_render_state.bind.vertex_buffers[0] = kxgui_sokol_batches.buffers[batch_index];

        TextureHandle handle = kxgui_sokol_batches.texture_handles[batch_index];

        sg_view view = Texture_GetView(handle);

        kxgui_sokol_render_state.bind.views[VIEW_tex_array] = Texture_GetView(kxgui_sokol_fallback_texture_array);
        kxgui_sokol_render_state.bind.views[VIEW_tex2d] = Texture_GetView(kxgui_sokol_fallback_texture);
        
        if (kxgui_sokol_batches.texture_is_array[batch_index]) {
            kxgui_sokol_render_state.bind.views[VIEW_tex_array] = view;
        } else {
            kxgui_sokol_render_state.bind.views[VIEW_tex2d] = view;
        }

        sg_update_buffer(kxgui_sokol_render_state.bind.vertex_buffers[0], &(sg_range){v_list.data, v_list.size * sizeof(kxgui_sokol_vertex)});
        
        sg_apply_bindings(&kxgui_sokol_render_state.bind);

        sg_draw(0, v_list.size, 1);
        
    }

    sg_end_pass();
    
}

#endif //KXGUI_SOKOL_H

