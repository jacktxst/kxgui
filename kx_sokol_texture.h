//
// Created by jack lewis on 1/22/26.
//

// this is the implementation of the kx pal texture api for the sokol backend

#ifndef KX_SOKOL_TEXTURE_H
#define KX_SOKOL_TEXTURE_H

#include "kxcore.h"
#include "sokol_fetch.h"

#define KX_SOKOL_TEXTURE_MAX 256

typedef enum {
    KX_TEXKIND_NONE = 0,
    KX_TEXKIND_2D,
    KX_TEXKIND_ARRAY,
} kx_tex_type;

typedef struct {
    sg_image image;
    sg_view  view;
    uvec3    size;
    kx_tex_type kind;
} KxSokolTextureSlot;

struct
{
    KxSokolTextureSlot slots[KX_SOKOL_TEXTURE_MAX];
    u32 next_tex_id;
}
kx_sokol_textures = {0};

TextureHandle Texture_Create(Bitmap32 bitmap) 
{
    u32 index = kx_sokol_textures.next_tex_id;
    if (index >= KX_SOKOL_TEXTURE_MAX) {
        return 0;
    }
    KxSokolTextureSlot* slot = &kx_sokol_textures.slots[index];
    
    printf("test1\n");
    slot->image = sg_make_image(&(sg_image_desc){
        .type = SG_IMAGETYPE_2D,
        .width = bitmap.width,
        .height = bitmap.height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .sample_count = 1,
        .label = "mytexture",
        .usage = (sg_image_usage){.dynamic_update=true}
    });
    sg_update_image
    (
        slot->image, 
        &(sg_image_data)
        {
            .mip_levels[0] = (sg_range){bitmap.pixels,bitmap.height*bitmap.width*sizeof(u32)}
        }
    );
    printf("test2\n");
    slot->view = sg_make_view(&(sg_view_desc){
        .texture = { .image = slot->image },
        .label = "mytexture-view",
    });
    slot->kind = KX_TEXKIND_2D;
    slot->size = (uvec3){bitmap.width,bitmap.height,1};
    kx_sokol_textures.next_tex_id++;
    /* handles are 1-based, 0 is "invalid / untextured" */
    return index + 1;
}

void Texture_Data(TextureHandle tex_handle, Bitmap32 bmp32) 
{
    fprintf(stderr, "test3\n");
    
    if (tex_handle == 0) return;
    u32 index = tex_handle - 1;
    if (index >= kx_sokol_textures.next_tex_id) return;
    KxSokolTextureSlot* slot = &kx_sokol_textures.slots[index];
    if (slot->kind != KX_TEXKIND_2D) return;
    sg_image img = slot->image;
    sg_update_image
    (
        img, 
        &(sg_image_data)
        {
            .mip_levels[0] = (sg_range){bmp32.pixels, bmp32.width*bmp32.height*sizeof(u32)}
        }
    );
}

ivec2 Texture_GetSize(TextureHandle tex_handle)
{
    if (tex_handle == 0) return (ivec2){-1,-1};
    u32 index = tex_handle - 1;
    if (index >= kx_sokol_textures.next_tex_id) return (ivec2){-1,-1};
    KxSokolTextureSlot* slot = &kx_sokol_textures.slots[index];
    if (slot->kind != KX_TEXKIND_2D) return (ivec2){-1,-1};
    return (ivec2){ (i32)slot->size.x, (i32)slot->size.y }; 
}

void Texture_Destroy(TextureHandle * tex_handle) 
{
    if (!tex_handle || *tex_handle == 0) return;
    u32 index = *tex_handle - 1;
    if (index >= kx_sokol_textures.next_tex_id) {
        *tex_handle = 0;
        return;
    }
    /* destroy and compact slots so remaining handles stay contiguous;
       user should treat destroyed handles as invalid */
    sg_destroy_image(kx_sokol_textures.slots[index].image);
    /* shift down higher slots */
    if (index + 1 < kx_sokol_textures.next_tex_id) {
        memmove(
            &kx_sokol_textures.slots[index],
            &kx_sokol_textures.slots[index + 1],
            sizeof(KxSokolTextureSlot) * (kx_sokol_textures.next_tex_id - index - 1)
        );
    }
    kx_sokol_textures.next_tex_id--;
    *tex_handle = 0;
}

/* helpers for renderer / batching */
static inline kx_tex_type Texture_GetKind(TextureHandle handle)
{
    if (handle == 0) return KX_TEXKIND_NONE;
    u32 index = handle - 1;
    if (index >= kx_sokol_textures.next_tex_id) return KX_TEXKIND_NONE;
    return kx_sokol_textures.slots[index].kind;
}

static inline sg_view Texture_GetView(TextureHandle handle)
{
    sg_view invalid = {0};
    if (handle == 0) return invalid;
    u32 index = handle - 1;
    if (index >= kx_sokol_textures.next_tex_id) return invalid;
    return kx_sokol_textures.slots[index].view;
}


uvec3 TextureArray_GetSize(TextureHandle texarray_handle)
{
    if (texarray_handle == 0) return (uvec3){(u32)-1,(u32)-1,(u32)-1};
    u32 index = texarray_handle - 1;
    if (index >= kx_sokol_textures.next_tex_id) return (uvec3){(u32)-1,(u32)-1,(u32)-1};
    KxSokolTextureSlot* slot = &kx_sokol_textures.slots[index];
    if (slot->kind != KX_TEXKIND_ARRAY) return (uvec3){(u32)-1,(u32)-1,(u32)-1};
    return slot->size; 
}

TextureHandle TextureArray_Create(Bitmap32_Array bmp32arr) 
{
    u32 index = kx_sokol_textures.next_tex_id;
    if (index >= KX_SOKOL_TEXTURE_MAX) {
        return 0;
    }
    uvec3 size = (uvec3){ bmp32arr.width, bmp32arr.height, bmp32arr.num_slices };
    KxSokolTextureSlot* slot = &kx_sokol_textures.slots[index];
    slot->image = sg_make_image(&(sg_image_desc){
        .type = SG_IMAGETYPE_ARRAY,
        .width = size.x,
        .height = size.y,
        .num_slices = size.z,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .sample_count = 1,
        .label = "mytexture",
        .usage = (sg_image_usage){.dynamic_update = true}
    });
    sg_update_image
    (
        slot->image, 
        &(sg_image_data)
        {
            .mip_levels[0] = (sg_range){bmp32arr.pixels, bmp32arr.width*bmp32arr.height*bmp32arr.num_slices*sizeof(u32)}
        }
    );
    slot->view = sg_make_view(&(sg_view_desc){
        .texture = { .image = slot->image },
        .label = "mytexture-view",
    });
    slot->kind = KX_TEXKIND_ARRAY;
    slot->size = size;
    kx_sokol_textures.next_tex_id++;
    return index + 1;
}

void TextureArray_Data(TextureHandle texarray_handle, Bitmap32_Array bmp32arr) 
{
    if (texarray_handle == 0) return;
    u32 index = texarray_handle - 1;
    if (index >= kx_sokol_textures.next_tex_id) return;
    KxSokolTextureSlot* slot = &kx_sokol_textures.slots[index];
    if (slot->kind != KX_TEXKIND_ARRAY) return;
    sg_image img = slot->image;
    sg_update_image
    (
        img, 
        &(sg_image_data)
        {
            .mip_levels[0] = (sg_range){bmp32arr.pixels, bmp32arr.width*bmp32arr.height*bmp32arr.num_slices*sizeof(u32)}
        }
    );
};

void TextureArray_Destroy(TextureHandle array_handle) 
{
    TextureHandle h = array_handle;
    Texture_Destroy(&h);
}




#endif //KX_SOKOL_TEXTURE_H

