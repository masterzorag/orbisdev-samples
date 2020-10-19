#pragma once

/*
  here are the parts
*/

#if defined (__ORBIS__)

#include <ps4sdk.h>
#include <debugnet.h>
#define  fprintf  debugNetPrintf
#define  ERROR    DEBUGNET_ERROR
#define  DEBUG    DEBUGNET_DEBUG
#define  INFO     DEBUGNET_INFO
#include <orbisGl.h>

#else

#include <stdio.h>
#define  debugNetPrintf  fprintf
#define  ERROR           stderr
#define  DEBUG           stdout
#define  INFO            stdout
#include <GLES2/gl2.h>
//#include <orbisAudio.h>

#endif // defined (__ORBIS__)

#include "zrnic_rg.h"

#include <stddef.h>

/// from fileIO.c
unsigned char *orbisFileGetFileContent( const char *filename );
extern size_t _orbisFile_lastopenFile_size;


/// from shader-common.c
GLuint create_vbo  (const GLsizeiptr size, const GLvoid *data, const GLenum usage);
//GLuint BuildShader (const char *source, GLenum shaderType);
/// build (and dump) from shader source code
GLuint BuildProgram(const char *vShader, const char *fShader);
//GLuint CreateProgramFromBinary(const char *vShader, const char *fShader);


// from cmd_build.c
char **build_cmd(char *cmd_line);


#if defined FT_DEMO || FT_DEMO_2 // overlay alike, for testing
/// from demo-font.c
#include <freetype-gl.h>
int  es2init_text (int width, int height);
void add_text( vertex_buffer_t *buffer,
                texture_font_t *font,
                          char *text,
                          vec4 *color,
                          vec2 *pen );
void render_text  (void);
void es2sample_end(void);
#endif

 #if defined TEXT_ANI
  /// from text_ani.c
  #include "text_ani.h"
  void es2init_text_ani(int width, int height);
  void es2rndr_text_ext(fx_entry_t *_ani);
  void es2updt_text_ani(double elapsedTime);
  void es2fini_text_ani(void);
#endif


// Clang Extended Vectors
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float vec4 __attribute__((ext_vector_type(4)));


/// from png.c
GLuint load_png_asset_into_texture(const char *relative_path);
extern vec2 tex_size; // last loaded png size as (w, h)


/// from sprite.c
void on_GLES2_Init_sprite  (int view_w, int view_h);
void on_GLES2_Size_sprite  (int view_w, int view_h);
void on_GLES2_Render_sprite(int num);
void on_GLES2_Update_sprite(int frame);
void on_GLES2_Final_sprite (void);


/// from pl_mpeg.c
int  es2init_pl_mpeg  (int window_width, int window_height);
void es2render_pl_mpeg(float dt);
void es2end_pl_mpeg   (void);


/// from timing.c
unsigned int get_time_ms(void);


#if defined HAVE_NFS
/// from user_nfs.c
#include <nfsc/libnfs.h>
int    user_init (void);
size_t user_stat (void);
struct
nfsfh *user_open (const char *filename);
void   user_seek (long offset, int whence);
size_t user_read (unsigned char *dst, size_t size);
void   user_close(void);
void   user_end  (void);
#endif

// rects tests tetris primlib
void filledRect(int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b);

/// for icons.c, sprite.c
#define NUM_OF_TEXTURES  (8)
#define NUM_OF_SPRITES   (6)

/// from ls_dir()
int ls_dir(char *dirpath);

int get_item_count(void);
typedef struct
{
    char  *name;
    //size_t size;
} entry_t;
entry_t *get_item_entries(char *dirpath);


// from my_rects.c
vec2 px_pos_to_normalized(vec2 *pos);


// from lines_and_rects
void es2rndr_lines_and_rect( void );
void es2init_lines_and_rect( int width, int height );
void es2fini_lines_and_rect( void );


/// from icons.c
void on_GLES2_Init_icons(int view_w, int view_h);
void on_GLES2_Final (void);
void on_GLES2_Update(double frame);
void on_GLES2_Render(int index);


/// GLES2_rects.c
void ORBIS_RenderFillRects_init(int width, int height);
void ORBIS_RenderFillRects_rndr(void);
void ORBIS_RenderFillRects_fini(void);

/// from GLES2_*
void ORBIS_RenderDrawLines(const vec2 *points, int count);
void ORBIS_RenderFillRects(const vec4 *rects,  int count);
void ORBIS_RenderDrawBox  (const vec4 *rect);

// reuse this type to index texts around!
typedef struct
{
    char *off;
    int   len;
} item_idx_t;


enum views
{
    ON_TEST_ANI = -1,
    ON_MAIN_SCREEN,
    ON_SUBMENU,
    ON_SUBMENU_2,
    ON_ITEM_PAGE
};

// from GLES2_ani.c
void GLES2_ani_init  (int width, int height);
void GLES2_ani_update(double time);
void GLES2_ani_fini  ( void );


// from GLES2_textures.c
void on_GLES2_Init_icons(int view_w, int view_h);
void on_GLES2_Update(double time);
//void on_GLES2_Render_icons(int num);
void on_GLES2_Render_icon(GLuint texture, int num, vec4 *frect);
void on_GLES2_Render_box(vec4 *frect);
void on_GLES2_Final(void);


// from GLES2_scene.c
void GLES2_scene_init( int w, int h );
void GLES2_scene_render(void);
void GLES2_scene_on_pressed_button(int button);


// from pixelshader.c
void pixelshader_render( void );
void pixelshader_init( int width, int height );
void pixelshader_fini( void );


// from json_simple.c
