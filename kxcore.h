
#ifndef CORE_H
#define CORE_H

/* basic types */

#ifndef KX_DEF_PRIMITIVES
#define KX_DEF_PRIMITIVES

	typedef uint8_t  u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;
	typedef int8_t   i8;
	typedef int16_t  i16;
	typedef int32_t  i32;
	typedef int64_t  i64;
	typedef float    f32;
	typedef enum     { X, Y, Z } Axis;

#endif

/* vectors */

#ifndef KX_DEF_VECTYPES
#define KX_DEF_VECTYPES

	typedef struct {f32 x, y;} fvec2;
	typedef struct {f32 x, y, z;} fvec3;
	typedef struct {f32 x, y, z, w;} fvec4;
	typedef struct {i32 x, y;} ivec2;
	typedef struct {i32 x, y, z;} ivec3;
	typedef struct {i32 x, y, z, w;} ivec4;
	typedef struct {u32 x, y;} uvec2;
	typedef struct {u32 x, y, z;} uvec3;
	typedef struct {u32 x, y, z, w;} uvec4;
	const fvec3 FVEC3_ZERO = {0,0,0};

#define IVEC2(a, b) (ivec2){a, b}
#define FVEC2(a, b) (fvec2){a, b}
#define FVEC4(a, b, c, d) (fvec4){a, b, c, d}
#endif

fvec2 fvec2_add(fvec2 a, fvec2 b);
fvec2 fvec2_sub(fvec2 a, fvec2 b);
fvec2 fvec2_multiply(fvec2 a, fvec2 b);
fvec2 fvec2_multiplyScalar(fvec2 a, f32 scalar);
fvec2 fvec2_abs(fvec2 a);
fvec2 fvec2_length(fvec2 a);
fvec2 fvec2_normalize(fvec2 a);

/* color */

typedef struct {u8 r, g, b, a;} color;

color Color(u32 color);

const color BLACK    = {0,   0,   0,   255};
const color WHITE    = {255, 255, 255, 255};
const color RED      = {255, 0,   0,   255};
const color GREEN    = {0,   255, 0,   255};
const color BLUE     = {0,   0,   255, 255};
const color CYAN     = {255, 255, 0,   255};
const color MAGENTA  = {255, 0,   255, 255};
const color YELLOW   = {255, 255, 0,   255};

/* basic media */

typedef struct { 
	u32 height, width;
	u32 * pixels;
} Bitmap32; 

void Bitmap32_Blit();
void Bitmap32_Crop();
void Bitmap32_Stretch();
void Bitmap32_Func();

typedef struct { 
	u32 height, width;
	u8 * pixels;
} Bitmap8; 

typedef struct {
	u32 height, width;
	u8 * bits;
} Bitmask;

typedef struct {
	i32 x, y, w, h;
} Rectangle;

u8 BitmaskGet   (Bitmask mask, ivec2 pos);
u8 BitmaskSet   (Bitmask mask, ivec2 pos, u8 value);
void  BitmaskInvert(Bitmask mask);
typedef struct {} NoiseSettings;
void GenNoisemap(Bitmap32 dest, Rectangle rect, NoiseSettings settings);
typedef struct { } BitmapFont;
Boolean GetThreshholdMask(Bitmask mask, Bitmap32 map, u8 value);

/* matrices */

typedef struct {
    fvec3 position;
    fvec3 rotation;
} Camera3D;

typedef f32 Matrix_4x4[16];

void MatrixView(Matrix_4x4 dest, Camera3D cam);
void MatrixScale(Matrix_4x4 dest, fvec3 scale);
void MatrixIdentity(Matrix_4x4 dest);
void MatrixRotation(Matrix_4x4 dest, fvec3 rotation);
void MatrixMultiply(Matrix_4x4 dest, Matrix_4x4 a, Matrix_4x4 b);
void MatrixProjection(Matrix_4x4 dest, ...);
void MatrixTranslation(Matrix_4x4 dest, fvec3 translation);

/* 3d primitives */

typedef struct {
    fvec3 normal;
    f32   distance;
} Plane;

typedef struct {
	fvec3 position;
	fvec3 normal;
	fvec2 uv;
} Vertex;

typedef struct {} VertexAttribute;

typedef struct {
	u32 a, b, c;
} Triangle;

typedef struct {
	Vertex * data;
	size_t length;
	size_t capacity;
} VertexList;

typedef struct {
	u32 * data;
	size_t length;
	size_t capacity;
} u32_List;

typedef struct {
	VertexList verts;
	u32_List   elements;
} MeshBasic;

MeshBasic MeshBasic_New(u32 vertexCapacity, u32 elementCapacity);
void MeshBasic_Box(MeshBasic mesh, fvec3 position, fvec3 size);
void MeshBasic_Copy(MeshBasic dest, MeshBasic src);
void MeshBasic_Free(MeshBasic mesh);
void MeshBasic_Merge(MeshBasic dest, MeshBasic src1, MeshBasic src2);
void MeshBasic_Plane(MeshBasic mesh, Axis axis, ivec2 subdivisions);
void MeshBasic_Transform(MeshBasic mesh, Matrix_4x4 transform);
void MeshBasic_PushVertex(MeshBasic mesh, Vertex vertex);
void MeshBasic_PushElement(MeshBasic mesh, u32 element);

/* rigging / animations */

/* lighting */

/* particles */

/* billboards */

/* intersection functions */

// aabb - aabb
// point - aabb
// separating axis theorem
// swept stuff

/* raycasting helpers */

/* brushes */

typedef struct {
    Plane p;
    ivec2 uv_offset;
    ivec2 uv_scale;
    f32   uv_theta;
    int   texture;
} BrushPlane;
typedef struct {
    BrushPlane * planes;
    u32 numPlanes;
    u32 capPlanes;
} Brush;
Brush Brush_Box  (fvec3 pos, fvec3 size);
void  Brush_Copy (Brush dest, Brush src);
Brush Brush_Clone(Brush);
void  Brush_Slice();
Brush Brush_Empty();
MeshBasic Brush_PreviewMesh(Brush);
void CompileBSP();
void CompileLightmap();
typedef struct {} BSP_Tree;

/*
 *  PLATFORM ABSTRACTION LAYER
 *
 *
 *
 */

/* input */

void on_dropFiles(int count, const char** paths);
void on_cursorMove(double x, double y);
void on_pressOrRelease();
void on_scroll(double xOffset, double yOffset);
void on_typedChar(unsigned c);
void on_keyEvent();

/* logging */

void Console_Print(char *);

/* file i/o */

i32  File_GetSize    (char * path);
i32  File_ReadBytes  (void * buffer, u32 nBytes, char * filename, u32 offset);
bool File_PromptSave (void * buffer, u32 nBytes, char * filenameSuggestion);



/* device api */



/* texture api */

typedef u32   TextureHandle;

TextureHandle Texture_Import(char *);
TextureHandle Texture_Create();
void          Texture_Destroy(TextureHandle *);
void          Texture_Data(TextureHandle, Bitmap32);
ivec2         Texture_GetSize(TextureHandle);
void          Texture_SetSize(TextureHandle, ivec2);
void          Texture_Get(TextureHandle, Bitmap32);

typedef u32   TextureArrayHandle;

TextureArrayHandle TextureArray_Create(uvec3);
void			   TextureArray_Destroy();
void			   TextureArray_SetSize(uvec3);
void			   TextureArray_Data(uvec3);
uvec3		       TextureArray_GetSize(uvec3);

/* framebuffer api */

typedef u32 DisplaySurface;
typedef u32 Framebuffer;

#define FRAMEBUFFER_MAIN 0

Framebuffer Framebuffer_Create();
void        Framebuffer_Bind(Framebuffer);
ivec2       Framebuffer_GetSize();
void        Framebuffer_SetSize(ivec2);
void        Framebuffer_SlowRead(Bitmap32);
void        Framebuffer_AttachTexture(TextureHandle);
void        Framebuffer_DetachTexture();
void        Framebuffer_Clear(color);
void        Framebuffer_Blit(Bitmap32, Rectangle, Rectangle);
void        Framebuffer_Destroy(Framebuffer);

/* geometry api */

typedef u32 GeometryHandle;

GeometryHandle Geometry_Create (MeshBasic);
void Geometry_Modify (GeometryHandle, MeshBasic);
void Geometry_Draw   (Framebuffer, GeometryHandle, int);
void Geometry_Destroy(GeometryHandle *);

static inline fvec2 fvec2_to_ndc(fvec2 point, fvec2 screen_size) {
	return (fvec2){
		( point.x / screen_size.x ) * 2.0f - 1.0f,
		( 1- (point.y / screen_size.y) ) * 2.0f - 1.0f
	};
}

#endif //CORE_H
