#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_audio.h"
#include "sokol_time.h"
#include "sokol_glue.h"
#define VECMATH_GENERICS
#include "../shaders/out.h"
#include <string.h>
#include "kxcore.h"
#include "kxmath.h"
#include "kxgui.h"
#include <math.h>
#include "kxgui_font.h"

static struct
{
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
}
state;

#include "kxgui_sokol.h"

struct
{
    fvec2 mouse_pos;
    fvec2 mouse_scroll;
    u8 mouse_down;
    u8 keys[SAPP_MAX_KEYCODES];
    u8 mouse_released;
    u8 mouse_pressed;
    u8 typed_char;
}
kx_input;


static void init(void) {
    
    stm_setup(); // set up sokol_time.h
    
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    /*
    state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .size = 8192 * sizeof(float) * 10,
        .usage.dynamic_update = true,
        .label = "vertex-buffer"
    });
    */
    
    kxgui_sokol_init();

    
    sg_shader shd = sg_make_shader(triangle_shader_desc(sg_query_backend()));
    
    state.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .layout = {
            .attrs = { // 44 BYTES per vertex !!!
                [ATTR_triangle_position].format = SG_VERTEXFORMAT_FLOAT3, // 12 bytes
                [ATTR_triangle_color]  .format = SG_VERTEXFORMAT_FLOAT4,  // 16 bytes
                [ATTR_triangle_uv]  .format = SG_VERTEXFORMAT_FLOAT2,     // 8 bytes
                [ATTR_triangle_textured]  .format = SG_VERTEXFORMAT_FLOAT,// 4 bytes
                [ATTR_triangle_tex_slice].format = SG_VERTEXFORMAT_FLOAT, // 4 bytes
            }
        },
        .label = "triangle-pipeline",
        .colors[0].blend.enabled = 1,
        .colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
        .colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .colors[0].blend.op_rgb = SG_BLENDOP_ADD,
        .colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE,
        .colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .colors[0].blend.op_alpha = SG_BLENDOP_ADD
    });
    
    state.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = {0, 0, 0, 1}
        }
    };
    
    
}

struct
{
    u64 now;
    u64 last;
    u64 delta;
    u64 fcount;
    u64 ftimer;
    u64 fps;
}
kx_time = {0};

/*  this function is the main loop
 *
 */
void frame() {
    
    /* timing */
    kx_time.last = kx_time.now;
    kx_time.now = stm_now();
    kx_time.delta = kx_time.now - kx_time.last;
    kx_time.fcount++;
    kx_time.ftimer += kx_time.delta;
    if (stm_ms(kx_time.ftimer) >= 1000) {
        kx_time.fps = kx_time.fcount;
        kx_time.fcount = 0;
        kx_time.ftimer = 0;
    }
    
    /* gui */
    static kxgui_context ctx = {0};
    ctx.padding = 15;
    ctx.mouse_down = kx_input.mouse_down;
    ctx.mouse_released = kx_input.mouse_released;
    ctx.mouse_pressed = kx_input.mouse_pressed;
    ctx.mouse_scroll = kx_input.mouse_scroll;
    ctx.keys = kx_input.keys;
    ctx.typed_char = (i8)kx_input.typed_char;
    ctx.mouse_position = kx_input.mouse_pos;
    ctx.screen_size = FVEC2(sapp_widthf(), sapp_heightf());
    
    kxgui_begin_frame
    (
        &ctx
    );
    
    kx_input.mouse_scroll = (fvec2){0};
    kx_input.typed_char = 0;

    /* RANDOM BOXES TEST GUI */

    static textfield_t tf = {0};

    kxgui_textfield(&tf);
    
    kxgui_example_component((ivec2){100,50},(fvec4){1.0, 0.0, 0.0, 1.0});
    kxgui_example_component((ivec2){50,100},(fvec4){0.0, 1.0, 0.0, 1.0});
    
    static kxgui_container container1 = {
        .external = (fvec2){200,100},
        .internal = (fvec2){200,500},
        .bg_color = (fvec4){0.0, 1.0, 1.0, 1.0},
    };
    
    kxgui_begin_container(&container1);
        kxgui_example_component((ivec2){25,30},(fvec4){0.0, 0.5, 1.0, 1.0});
        kxgui_example_component((ivec2){30,25},(fvec4){1.0, 1.0, 0.0, 1.0});
    
        static kxgui_container container2 = {
            .external = (fvec2){100,100},
            .internal = (fvec2){100,500},
            .bg_color = (fvec4){0.0, 1.0, 0.0, 1.0},
            .flags = KXGUI_CONTAINER_FLOATING
        };
    
        kxgui_begin_container(&container2);
            kxgui_example_component((ivec2){15,15},(fvec4){1.0, 1.0, 0.0, 1.0});
            kxgui_example_component((ivec2){15,15},(fvec4){1.0, 1.0, 0.0, 1.0});
            kxgui_example_component((ivec2){15,15},(fvec4){1.0, 1.0, 0.0, 1.0});
            kxgui_example_component((ivec2){15,15},(fvec4){1.0, 1.0, 0.0, 1.0});
            kxgui_example_component((ivec2){15,15},(fvec4){1.0, 1.0, 0.0, 1.0});
        kxgui_end_container();
        kxgui_example_component((ivec2){30,25},(fvec4){1.0, 1.0, 0.0, 1.0}); 
    kxgui_end_container();
    kxgui_example_component((ivec2){50,25},(fvec4){1.0, 1.0, 1.0, 1.0});
    
    
    char fps_strbuf[16];
    sprintf(fps_strbuf, "fps %5llu", (unsigned long long)kx_time.fps);
    kxgui_label(fps_strbuf);
    
    static float lagamt = 0;
    kxgui_slider_float(&lagamt, 0, 100);
    
    for (int i = 0; i < (int)lagamt; i++) {
        kxgui_example_component(IVEC2(20, 20), FVEC4(1, 0, 0, 1));
    }
    
    kxgui_button_once("hiii");

    sg_begin_pass(&(sg_pass){ .action = state.pass_action, .swapchain = sglue_swapchain() });

    sg_apply_pipeline(state.pip);

    sg_end_pass();
    
    kxgui_sokol_draw_pass(&ctx);
    
    sg_commit();
}

void cleanup(void) {
    sg_shutdown();
}

void event_cb(const sapp_event* e) {
    
    static int audio_started = 0;
    if ((e->type == SAPP_EVENTTYPE_KEY_DOWN || e->type == SAPP_EVENTTYPE_MOUSE_DOWN) && !audio_started) {
        //start_audio_on_gesture();
        audio_started = 1;
    }
    
    switch (e->type) {
        case SAPP_EVENTTYPE_MOUSE_MOVE:
        {
            kx_input.mouse_pos = (fvec2){e->mouse_x, e->mouse_y};
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_DOWN:
        {
            kx_input.mouse_down = 1;
            kx_input.mouse_pressed = 1;
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP:
        {
            kx_input.mouse_down = 0;
            kx_input.mouse_released = 1;
            break;    
        }
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
        {
            kx_input.mouse_scroll.y = e->scroll_y;
        }
        case SAPP_EVENTTYPE_KEY_DOWN:
        {
            kx_input.keys[e->key_code] = 1;
            break;    
        }
        case SAPP_EVENTTYPE_KEY_UP:
        {
            kx_input.keys[e->key_code] = 0;
            break;
        }
        case SAPP_EVENTTYPE_CHAR:
        {
            kx_input.typed_char = e->char_code;
            break;    
        }
        case SAPP_EVENTTYPE_TOUCHES_BEGAN:
        {
            sapp_touchpoint touch = e->touches[0];
            kx_input.mouse_down = 1;
            kx_input.mouse_pressed = 1;
            kx_input.mouse_pos = (fvec2){touch.pos_x, touch.pos_y};
            break;
        }    
        case SAPP_EVENTTYPE_TOUCHES_MOVED:
        {
            sapp_touchpoint touch = e->touches[0];
            kx_input.mouse_pos = (fvec2){touch.pos_x, touch.pos_y};
            break;
        }
        case SAPP_EVENTTYPE_TOUCHES_ENDED:
        {
            kx_input.mouse_down = 0;
            kx_input.mouse_released = 1;
            kx_input.mouse_pos = (fvec2){-1, -1};
            break;
        }
        default:
            break;
    }
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event_cb,
        .width = 640,
        .height = 480,
        .window_title = "kx app",
        .icon.sokol_default = true,
        .swap_interval = 0,
        .logger.func = slog_func,
    };
}

