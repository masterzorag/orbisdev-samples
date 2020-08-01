/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdio.h>
#include <string.h>

#include <freetype-gl.h>  // links against libfreetype-gl

#if defined (__PS4__)

#include <ps4sdk.h>
#include <debugnet.h>
#define  fprintf  debugNetPrintf
#define  ERROR    DEBUGNET_ERROR
#define  DEBUG    DEBUGNET_DEBUG
#define  INFO     DEBUGNET_INFO


#elif defined HAVE_LIBAO // on pc

#include <stdio.h>
#define  debugNetPrintf  fprintf
#define  ERROR           stderr
#define  DEBUG           stdout
#define  INFO            stdout

#include "defines.h"

#endif


// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;    // position
    vec4 color;; // color
} vertex_t;

typedef struct {
    float x, y, z;
    vec4 color;
} point_t;


// ------------------------------------------------------- global variables ---
texture_atlas_t *atlas;
vertex_buffer_t *text_buffer;
static vertex_buffer_t *line_buffer;
static vertex_buffer_t *point_buffer;
static vertex_buffer_t *rects_buffer;
static GLuint shader,
           mz_shader;
static mat4 model, view, projection;
// ---------------------------------------------------------------- display ---
void es2rndr_lines_and_rect( void )
{
    /*int viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );*/
    /*glClearColor(1,1,1,1);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );*/
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable( GL_CULL_FACE );
#if 0
    glUseProgram( text_shader );
    {
        glUniform1i       ( glGetUniformLocation( text_shader, "texture" ),    0 );
        glUniformMatrix4fv( glGetUniformLocation( text_shader, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( text_shader, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( text_shader, "projection" ), 1, 0, projection.data);
        /* draw whole VBO item array */
        vertex_buffer_render( text_buffer, GL_TRIANGLES );
    }
#endif
    //glPointSize( 10.0f );

    glUseProgram( shader );
    {
        glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ), 1, 0, projection.data);
        /* draw whole VBO item arrays */
        vertex_buffer_render( line_buffer,  GL_LINES );
        vertex_buffer_render( point_buffer, GL_POINTS );
    }

    glUseProgram( mz_shader );
    {
        glUniformMatrix4fv( glGetUniformLocation( mz_shader, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( mz_shader, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( mz_shader, "projection" ), 1, 0, projection.data);
        /* draw whole VBO item arrays */
        vertex_buffer_render( rects_buffer, GL_TRIANGLES );
    }

    glUseProgram( 0 );
}

// ---------------------------------------------------------------- reshape ---
static void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}


// ------------------------------------------------------ freetype-gl shaders ---
static GLuint CreateProgram(int program)
{
#if 1 /* we can use OrbisGl wrappers, or MiniAPI ones */
    GLchar  *vShader = (void*) orbisFileGetFileContent( "shaders/v3f-c4f.vert" );
    GLchar  *fShader = NULL;

    switch(program)
    {
        case 0: fShader = (void*) orbisFileGetFileContent( "shaders/v3f-c4f.frag" ); break;
        case 1: fShader = (void*) orbisFileGetFileContent( "shaders/mz.frag" );      break;
    }
    GLuint programID = BuildProgram(vShader, fShader); // shader_common.c

    /* don't leak what we readed from files! */
    if (vShader) free(vShader), vShader = NULL;
    if (fShader) free(fShader), fShader = NULL;

#endif
    if (!programID) { /* failed! */ sleep(2); }
    // feedback
    printf( "program_id=%d (0x%08x)\n", programID, programID);
    return programID;
}


// freetype-gl pass last composed Text_Length in pixel, we use to align text
extern float tl;

// ------------------------------------------------------------------- init ---
void es2init_lines_and_rect( int width, int height )
{
    vec4 blue  = {{0,0,1,1}};
    vec4 black = {{0,0,0,1}};

    /* load .ttf in memory */
    void *ttf  = orbisFileGetFileContent("/hostapp/fonts/zrnic_rg.ttf");

    atlas = texture_atlas_new( 512, 512, 1 );
    texture_font_t *big   = texture_font_new_from_memory(atlas, 400, ttf, _orbisFile_lastopenFile_size);
    texture_font_t *small = texture_font_new_from_memory(atlas,  18, ttf, _orbisFile_lastopenFile_size);
    texture_font_t *title = texture_font_new_from_memory(atlas,  32, ttf, _orbisFile_lastopenFile_size);

    text_buffer  = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    line_buffer  = vertex_buffer_new( "vertex:3f,color:4f" );
    point_buffer = vertex_buffer_new( "vertex:3f,color:4f" );
    rects_buffer = vertex_buffer_new( "vertex:3f,color:4f" );

    vec2 pen, origin;

    texture_glyph_t *glyph  = texture_font_get_glyph( big, "g" );
    origin.x = width/2  - glyph->offset_x - glyph->width/2;
    origin.y = height/2 - glyph->offset_y + glyph->height/2;
//    add_text( text_buffer, big, "g", &black, &origin );

    // title
    pen.x = 50;
    pen.y = height - 50;
//    add_text( text_buffer, title, "Glyph metrics", &black, &pen );

    // lines
    point_t vertices[] =
        {   // Baseline
            {0.1*width, origin.y, 0, black},
            {0.9*width, origin.y, 0, black},

            // Top line
            {0.1*width, origin.y + glyph->offset_y, 0, black},
            {0.9*width, origin.y + glyph->offset_y, 0, black},

            // Bottom line
            {0.1*width, origin.y + glyph->offset_y - glyph->height, 0, black},
            {0.9*width, origin.y + glyph->offset_y - glyph->height, 0, black},

            // Left line at origin
            {width/2-glyph->offset_x-glyph->width/2, 0.1*height, 0, black},
            {width/2-glyph->offset_x-glyph->width/2, 0.9*height, 0, black},

            // Left line
            {width/2 - glyph->width/2, .3*height, 0, black},
            {width/2 - glyph->width/2, .9*height, 0, black},

            // Right line
            {width/2 + glyph->width/2, .3*height, 0, black},
            {width/2 + glyph->width/2, .9*height, 0, black},

            // Right line at origin
            {width/2-glyph->offset_x-glyph->width/2+glyph->advance_x, 0.1*height, 0, black},
            {width/2-glyph->offset_x-glyph->width/2+glyph->advance_x, 0.7*height, 0, black},

            // Width
            {width/2 - glyph->width/2, 0.8*height, 0, blue},
            {width/2 + glyph->width/2, 0.8*height, 0, blue},

            // Advance_x
            {width/2-glyph->width/2-glyph->offset_x, 0.2*height, 0, blue},
            {width/2-glyph->width/2-glyph->offset_x+glyph->advance_x, 0.2*height, 0, blue},

            // Offset_x
            {width/2-glyph->width/2-glyph->offset_x, 0.85*height, 0, blue},
            {width/2-glyph->width/2, 0.85*height, 0, blue},

            // Height
            {0.3*width/2, origin.y + glyph->offset_y - glyph->height, 0, blue},
            {0.3*width/2, origin.y + glyph->offset_y, 0, blue},

            // Offset y
            {0.8*width, origin.y + glyph->offset_y, 0, blue},
            {0.8*width, origin.y , 0, blue},

        };
    GLuint indices [] = {  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,
                          13,14,15,16,17,18,19,20,21,22,23,24,25};
    vertex_buffer_push_back( line_buffer, vertices, 26, indices, 26 );

    // points
    GLuint i = 0;
    point_t p;
    p.color = black;

    // Origin point
    p.x = width/2  - glyph->offset_x - glyph->width/2;
    p.y = height/2 - glyph->offset_y + glyph->height/2;
    vertex_buffer_push_back( point_buffer, &p, 1, &i, 1 );

    // Advance point
    p.x = width/2  - glyph->offset_x - glyph->width/2 + glyph->advance_x;
    p.y = height/2 - glyph->offset_y + glyph->height/2;
    vertex_buffer_push_back( point_buffer, &p, 1, &i, 1 );


#if 1 // rects
vec4 color = {{ 1., 1., 1., 1. }};
for (int i = 0; i < 10; ++i)
{
    int  x0 = (int)( pen.x + i * 2 );
    int  y0 = (int)( pen.y + 20 );
    int  x1 = (int)( x0 + 64 );
    int  y1 = (int)( y0 - 64 );

    GLuint indices2[6] = {0,1,2, 0,2,3}; // (two triangles)

    /* VBO is setup as: "vertex:3f, vec4 color */ 
    vertex_t vertices2[4] = { { x0,y0,0,   color },
                              { x0,y1,0,   color },
                              { x1,y1,0,   color },
                              { x1,y0,0,   color } };
    vertex_buffer_push_back( rects_buffer, vertices2, 4, indices2, 6 );
    pen.x   += 72.;
    pen.y   -= 32.;
    color.g -= i * 0.1;
}
#endif
    
    /* compile, link and use shader */
    /* load from file */
    shader    = CreateProgram(0);
    mz_shader = CreateProgram(1);

    //if(!shader) { fprintf(stderr, "program creation failed\n"); }

    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );

    reshape(width, height);
}

void es2fini_lines_and_rect( void )
{
//    texture_atlas_delete(atlas);        atlas        = NULL;
    vertex_buffer_delete(text_buffer);  text_buffer  = NULL;
    vertex_buffer_delete(line_buffer);  line_buffer  = NULL;
    vertex_buffer_delete(point_buffer); point_buffer = NULL;

    if(shader) glDeleteProgram(shader), shader = 0;
}