/* kxgui core v0.9
 * last modified
 * mar 13 2026
 * 
 * hello, welcome to my imgui library. i began passionately working on
 * this thing in nov. 2025. i took a bit of inspiration from, and would
 * like to give kudos to, the clay library as well as the Dear ImGui library.
 * i've not used either of these, and i barely know what their apis
 * look like, but their popularity was certainly the source of my
 * inspiration.
 *
 * this is a minimalistic, cross platform gui library that allows you
 * to build simple graphical applications in c using an immediate mode
 * api.
 *
 * this gui library is capable of running over any i/o backend and
 * any graphical backend, whether it be OpenGL, Vulkan, or something
 * else entirely.
 *
 * on every frame, you issue function calls like
 * 
 *  kxgui_label("hello world");
 *  if (kxgui_button("press me to quit")) quit_program();
 *
 * kxgui automatically does layout for these components and adapts to changing screen sizes. 
 * at the end of a frame, kxgui outputs a list of simplified render commands.
 * it could look something like this:
 *      RECTANGLE 200, 200, 400, 400, COLOR RED
 *      RECTANGLE 200, 300, 4, 50, TEXTURE 0, UV SCALE 2, 2
 *
 * a renderer function is necessary for converting this list into
 * the appropriate memory management and draw calls for a specific
 * rendering backend.
 * 
 * i have already implemented a renderer that uses sokol_gfx.h as
 * a backend. see kxgui_sokol.h
 *
 * IMPORTANT - A NOTE ON IMMEDIATE MODE GUI - WHAT IS IT AND WHY?
 *
 *   #  # #### #   #    ##### ####    #   # #### #####
 *   #  # #  # # # #      #   #  #    #   # #    #
 *   #### #  # # # #      #   #  #    #   # #### ###
 *   #  # #  # # # #      #   #  #    #   #    # #
 *   #  # #### #####      #   ####    ##### #### #####
 * 
 * 1 - SETTING UP A CONTEXT
 * 
 * you will need a static or global { struct kxgui_context }
 * that you can reference from within your main loop.
 * this is where all the data for the gui will live.
 * depending on which gui components you use,
 * the context struct may also contain some pointers to
 * dynamically allocated data. kxgui should manage
 * this automatically for the most part, but keep it in
 * mind.
 *
 * example:
 *
 * void main_loop() {
 *  static kxgui_context ctx = kxgui_init_context( (kxgui_context_desc) {
 *
 *
 *
 *  });
 *  // more code here
 *  opengl_draw();
 * }
 *
 * BEGINNING A FRAME
 * before you can use kxgui to draw a frame, it needs to
 * be provided with some contextual information about the
 * running program to know how to produce the frame correctly.
 * some of these parameters change frame to frame,
 * so the values are shared with kxgui every frame.
 * this is part of how kxgui is able to interface with any
 * program backend.
 * 
 * void main_loop() {
 *  static kxgui_context ctx = kxgui_init_context((kxgui_context_desc){...});
 *  kxgui_begin_frame(&ctx, (kxgui_frame_desc){...} )         <-------- 
 *  opengl_draw();
 * }
 * 
 * so, for every frame you'll need this "kxgui_frame_desc" struct.
 *
 * void main_loop() {
 *  static kxgui_context ctx = {0};
 *  kxgui_begin_frame(&ctx, (kxgui_frame_desc) {
 *
 *
 *
 *  } )          
 *  opengl_draw();
 * }
 */

// todo list of DOOM

// 1 - bugfix - elements will "wrap" even when it doesnt help.
// 2 - gradient rectangles
// 3 - tooltip api
// 4 - sideways scroll
// 5 - drag scroll
// 6 - drag support
//       some way to globally keep track of a dragged element,
//       so it can continue being dragged even when the mouse
//       flies off of it.. very important!
//       note to self - these objects just need to remember that theyve been clicked even when the mouse is off of them
// 7 - container flags
// 8 - resizable windows
// 9 - rectangle borders
// A - seems like we need to track z





















#ifndef KXGUI_H
#define KXGUI_H

// theres definitely a better place to put this... somewhere... anywhere but here in my cross platform library
#define SAPP_KEYCODE_DELETE 261
#define SAPP_KEYCODE_BACKSPACE 259
#define SAPP_KEYCODE_ENTER 257
#define SAPP_KEYCODE_DOWN 264
#define SAPP_KEYCODE_UP 265
#define SAPP_KEYCODE_RIGHT 262
#define SAPP_KEYCODE_LEFT 263

#include "kxmath.h"
#include <stdlib.h>

/* structs */

/*   these rects use a coordinate system that starts at the top left of the display
 *   in contrast to the inverted y-axis system present in many graphics apis. they
 *   are not normalized device coordinates, so a change by 1 represents a displacement
 *   of 1 pixel, unless some global scale factor other than 1 is being applied.
 */

struct kxgui_rect
{
    f32 x, y, width, height;
};

/*  this struct represents a single element that is produced as a result of beginning a kxgui frame and
 *  drawing components. each component can create several elements of type kxgui_render_cmd. the render
 *  commands are platform-abstract simplified instructions that get fed to a renderer which is
 *  responsible for translating them into platform specific draw calls.
 */

typedef struct kxgui_render_cmd
{
    struct kxgui_rect rect;
    fvec4 color;
    fvec2 uv_scale;
    fvec2 uv_offset;
    f32 z_layer;
    u32 texture; // a value of 0 means untextured. any other value refers to a texture that must be provided through the renderer
    u32 texture_slice; // used to select a particular texture from a texture array
}
kxgui_render_cmd;

/* since kxgui attempts to maintain as little hidden state as possible
 * the user must manage memory for certain kinds of components and containers
 * that need to remember changes of state between frames which may be caused by
 * user input or animation. 
 */

typedef struct
{
    fvec4 bg_color;
    fvec2 pos;      // for floating window position
    fvec2 internal; 
    fvec2 external; // the visible size of the container
    fvec2 scroll;
    u32   flags;
}
kxgui_container;

/* to enable multiple nested layers of containers, kxgui internally uses a stack
 * to allow it to isolate the rendering of the child contents of a container
 * before restoring layout state to draw the next elements which are siblings
 * of the container.
 */

typedef struct kxgui_stackframe
{
    struct kxgui_rect _parent_window;
    struct kxgui_rect _clip;
    f32 _tallest;
    ivec2 _cursor;
    ivec2 _next_cursor;

    kxgui_container * container_ptr;
}
kxgui_stackframe;

typedef struct 
{

   // tracks the height of the tallest element in a line
   // so that the cursor is moved down by an appropriate
   // amount on the next new line.

   f32 _tallest;
   ivec2 _cursor; 
   ivec2 _next_cursor;
   struct kxgui_rect _parent_window;

   // screen space clip rect 
   // all drawn rects will be 
   // clipped to the bounds
   // of the clip rect.

   struct kxgui_rect _clip; 

   // hold the parameters
   // of a currently in
   // progress draw command.

   struct kxgui_rect _rect;
   f32 _z;
    
   // stack allows multiple
   // levels of containers

   struct 
   {
       kxgui_stackframe * base;
       u32 i;
       u32 max;
   } 
   _stack;
   
   struct 
   {
       struct kxgui_render_cmd * commands;
       u32 capacity;
       u32 n;
   } 
   _render_output;

   /* public */
   f32 padding;
   fvec2 screen_size;
   fvec2 scale;
   fvec2 mouse_position;
   fvec2 mouse_scroll;
   u8 mouse_pressed;
   u8 mouse_released;
   u8 mouse_down;
   char typed_char;
   u8 * keys;

} 
kxgui_context;

static kxgui_context * _kxgui_active_ctx = 0;

#define KXGUI_GETCTX() kxgui_context * ctx = _kxgui_active_ctx;

static void kxgui_init_context()
{
    KXGUI_GETCTX()
    ctx->_stack.base = malloc(sizeof(kxgui_stackframe) * 64);
    ctx->_stack.max = 64;
    ctx->_render_output.commands = malloc(sizeof(struct kxgui_render_cmd) * 8192);
    ctx->_render_output.capacity = 8192;
}

static void kxgui_begin_frame(kxgui_context * ctx)
{
    _kxgui_active_ctx = ctx;
    /* allocate stuff if not allocated */
    if (!ctx->_stack.base || !ctx->_render_output.commands)
    {
        kxgui_init_context();
    }

    ctx->_stack.i = 0;
    ctx->_z = 0.5;
    ctx->_render_output.n = 0;
    ctx->_cursor = (ivec2){ctx->padding, ctx->padding};
    ctx->_clip = (struct kxgui_rect){0, 0, ctx->screen_size.x, ctx->screen_size.y};
    ctx->_parent_window = (struct kxgui_rect){0, 0, ctx->screen_size.x, ctx->screen_size.y};
    ctx->_tallest = 0;
}

/* component input api 
















*/

static fvec2 kxgui_mouse_position()
{
    KXGUI_GETCTX();

    if
    (
        ctx->mouse_position.x < ctx->_clip.x ||
        ctx->mouse_position.y < ctx->_clip.y ||
        ctx->mouse_position.x > ctx->_clip.x+ctx->_clip.width ||
        ctx->mouse_position.y > ctx->_clip.y+ctx->_clip.height
    )
    return (fvec2){-1,-1};
    
    return (fvec2)
    {
        ctx->mouse_position.x - ctx->_parent_window.x - ctx->_cursor.x,
        ctx->mouse_position.y - ctx->_parent_window.y - ctx->_cursor.y,
    };
}
static int kxgui_mouse_inside(f32 x, f32 y, f32 width, f32 height) {
    KXGUI_GETCTX();
    fvec2 mouse_position = kxgui_mouse_position();
    if
    (
        mouse_position.x > x &&
        mouse_position.y > y &&
        mouse_position.x < x+width &&
        mouse_position.y < y+height
    )
    return 1;
    return 0;
}
static int kxgui_mouse_pressed();
static int kxgui_mouse_held();
static int kxgui_mouse_released();

/* component draw api */


















static void kxgui_rect        ( const f32 x, const f32 y, const f32 width, const f32 height )
{ 
    KXGUI_GETCTX(); 
    ctx->_rect =
    (struct kxgui_rect)
    {
        x + ctx->_parent_window.x + ctx->_cursor.x,
        y + ctx->_parent_window.y + ctx->_cursor.y,
        width,
        height
    };
}
static void kxgui_z           ( f32 z ) 
{ 
    KXGUI_GETCTX(); 
    ctx->_z = z;
}
static void kxgui_fill_color  ( fvec4 color )
{
    KXGUI_GETCTX();
    ctx->_render_output.commands[ctx->_render_output.n++] =
    (struct kxgui_render_cmd)
    {
        .color = color,
        .rect = kxgui_intersect_rects(ctx->_clip, ctx->_rect),
        .texture = 0,
        .z_layer = ctx->_z
    };
};

static void kxgui_fill_texture( const u32 texture, const u32 slice, fvec2 uv_scale, fvec2 uv_offset )
{
    KXGUI_GETCTX();
    ctx->_render_output.commands[ctx->_render_output.n++] =
    (struct kxgui_render_cmd)
    {
        .color = (fvec4){1.0,1.0,1.0,1.0},
        .rect = kxgui_intersect_rects(ctx->_clip, ctx->_rect),
        .texture = texture,
        .z_layer = ctx->_z,
        .uv_scale = uv_scale,
        .uv_offset = uv_offset
    };
}

static void kxgui_newline() 
{
    KXGUI_GETCTX();
    ctx->_cursor.x  = ctx->padding;
    ctx->_cursor.y += ctx->_tallest + ctx->padding;
    ctx->_tallest   = 0;
}

// this function is nearly perfect imo, at least functionally
static void kxgui_begin_component( const fvec2 size ) 
{
    KXGUI_GETCTX();
    float next_cursor_x = ctx->_cursor.x + size.x + ctx->padding;
    /* this condition is confusing.
     * it successfully prevents a component
     * from rendering too close to the right edge,
     * but it's not explicit about that at all
     */
    if (next_cursor_x > ctx->_parent_window.width) 
    {
        kxgui_newline();
        next_cursor_x = size.x + ctx->padding * 2;
    }
    if (size.y > ctx->_tallest) 
    {
        ctx->_tallest = size.y;
    }
    ctx->_next_cursor.x = next_cursor_x;
    ctx->_next_cursor.y = ctx->_cursor.y;
}

static void kxgui_end_component() 
{
    KXGUI_GETCTX();
    ctx->_cursor = ctx->_next_cursor;
}

/* 
 *
 *
 *
 *
 *
 *
 *
 * ============================================================================
 * 
 * CCC  OOO  N   N TTTTT  AAA  I  N  N  EEEE  R       SS 
 * C    O O  NN  N   T    A A  I  NN N  E  E  R RR   S  
 * C    O O  N N N   T    AAA  I  N NN  EEEE  RR  R   SS
 * CCC  OOO  N  NN   T    A A     N  N  E     R         S
 *           N   N                       EEE  R       SS
 * ============================================================================
 *
 *
 *
 *
 *
 *
 *
 * 
 */

/* helpers */

static inline float kx_min(float a, float b) { return a < b ? a : b; }
static inline float kx_max(float a, float b) { return a > b ? a : b; }

static struct kxgui_rect kxgui_intersect_rects (struct kxgui_rect a, struct kxgui_rect b)
{
    float x1 = kx_max(a.x, b.x);
    float y1 = kx_max(a.y, b.y);

    float x2 = kx_min(a.x + a.width,  b.x + b.width);
    float y2 = kx_min(a.y + a.height, b.y + b.height);

    if (x2 <= x1 || y2 <= y1) {
        return (struct kxgui_rect){ 0, 0, 0, 0 };
    }

    return (struct kxgui_rect){
        .x = x1,
        .y = y1,
        .width  = x2 - x1,
        .height = y2 - y1,
    };
}

#define KXGUI_CONTAINER_FLOATING 1
// 
#define KXGUI_CONTAINER_RESIZABLE 2

/*  note that input for a container i.e. dragging and scrolling
 *  is not handles in this function but is instead handled in
 *  kxgui_end_container(). first, we need to see if any
 *  of the children components of a container are willing to accept
 *  the user input, so we must wait until kxgui_end_container()
 */
static void kxgui_begin_container(kxgui_container * container) 
{
    KXGUI_GETCTX();

    fvec2 * external = &(container->external);
    fvec2 * internal = &(container->internal);
    fvec2 * scroll   = &(container->scroll);
    u32   * flags    = &(container->flags);
    fvec4 * bg_color = &(container->bg_color);
    
    /* a container is actually just a component that gets interrupted
     * another entire gui can render in the middle of rendering this component.
     * all relevant state is stored on the stack while the child components
     * are drawn, and state is restored in order for the container component
     * to be finished up.
     */

    u8 floating = *flags & KXGUI_CONTAINER_FLOATING;

    ivec2 sto_cursor = ctx->_cursor;
    if (floating)
    {
        ctx->_cursor = (ivec2){container->pos.x, container->pos.y};
    }

    /* begin the container as a component */
    
    kxgui_begin_component(*external);
    kxgui_rect(0, 0, external->x, external->y);
    kxgui_fill_color(*bg_color);

    /* store state on the stack, interrupting the component */
    
    ctx->_stack.base[ctx->_stack.i] = 
    (kxgui_stackframe) 
    {
        .container_ptr = container,        
        ._clip = ctx->_clip,
        ._tallest = ctx->_tallest,
        ._cursor = floating ? sto_cursor : ctx->_cursor,
        ._next_cursor = ctx->_next_cursor,
        ._parent_window = ctx->_parent_window,
    };
    if (++ctx->_stack.i == ctx->_stack.max) {printf("stack overflow\n"); exit(0);}
    
    /* composite clip rect */

    ctx->_clip = kxgui_intersect_rects(ctx->_clip, (struct kxgui_rect){ctx->_parent_window.x+ctx->_cursor.x, ctx->_parent_window.y+ctx->_cursor.y, external->x, external->y});

    ctx->_parent_window = 
        (struct kxgui_rect) 
        {
            ctx->_cursor.x + ctx->_parent_window.x - (scroll ? scroll->x : 0),
            ctx->_cursor.y + ctx->_parent_window.y - (scroll ? scroll->y : 0),
            internal->x, internal->y
        };
    
    /* reset cursor */
    ctx->_cursor = IVEC2(ctx->padding, ctx->padding);
    ctx->_tallest = 0;
    
}

static void kxgui_end_container()
{
    KXGUI_GETCTX();
    if (ctx->_stack.i < 1) {printf("stack underflow\n"); exit(0);}
    ctx->_stack.i--;
    
    ctx->_clip =          ctx->_stack.base[ctx->_stack.i]._clip;
    ctx->_tallest =       ctx->_stack.base[ctx->_stack.i]._tallest;
    ctx->_next_cursor =   ctx->_stack.base[ctx->_stack.i]._next_cursor;
    ctx->_cursor =        ctx->_stack.base[ctx->_stack.i]._cursor;
    ctx->_parent_window = ctx->_stack.base[ctx->_stack.i]._parent_window;

    fvec2 * scroll   =   & ctx->_stack.base[ctx->_stack.i].container_ptr->scroll;
    fvec2 * external =   & ctx->_stack.base[ctx->_stack.i].container_ptr->external;
    u32   * flags    =   & ctx->_stack.base[ctx->_stack.i].container_ptr->flags;
    fvec2 * internal =   & ctx->_stack.base[ctx->_stack.i].container_ptr->internal;
    fvec2 * pos =   & ctx->_stack.base[ctx->_stack.i].container_ptr->pos;

    u8 floating = *flags & KXGUI_CONTAINER_FLOATING;

    ivec2 sto_cursor = ctx->_cursor;
    if (floating) {
        ctx->_cursor = (ivec2){pos->x, pos->y};
    }
    
    if (scroll && kxgui_mouse_inside(0, 0, external->x, external->y)) {
        /* show scroll bars */
        float scroll_height = external->y / internal->y * external->y;
        kxgui_rect
        (
            0,
            scroll->y / internal->y * (external->y - scroll_height),
            8,
            scroll_height
        );
        kxgui_fill_color((fvec4){1.0, 0.0, 1.0, 1.0});
        scroll->x += ctx->mouse_scroll.x;
        scroll->y += ctx->mouse_scroll.y;
        scroll->y = scroll->y < 0 ? 0 : scroll->y;
        scroll->y = scroll->y > internal->y ? internal->y : scroll->y;
        ctx->mouse_scroll.x = 0;
        ctx->mouse_scroll.y = 0;
    }

    if (floating) {
        ctx->_cursor = sto_cursor;
        ctx->_next_cursor = ctx->_cursor;
    }
    
    kxgui_end_component();
}

/* 
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * 
 *
 *
 *
 *
 *
 */

static void kxgui_example_component(const ivec2 size, const fvec4 color) {
    kxgui_begin_component((fvec2){size.x,size.y});

    kxgui_rect(0, 0, size.x, size.y);

    if (kxgui_mouse_inside(0, 0 , size.x, size.y)) {
        kxgui_fill_texture(1, 0, (fvec2){1,1},(fvec2){0,0});
    }
    else kxgui_fill_color(color);
    
    kxgui_end_component();
}

#include "kxgui_slider.h"
#include "kxgui_label.h"
#include "kxgui_textfield.h"
#include "kxgui_hexedit.h"
#include "kxgui_button.h"

#endif //KXGUI_H
