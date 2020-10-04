/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <freetype-gl.h>  // links against libfreetype-gl

#include "defines.h"

// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;    // position (3f)
//    float s, t;       // texture  (2f)
    float r, g, b, a; // color    (4f)
} vertex_t;

typedef  vertex_t  point_t;

// ------------------------------------------------------- global variables ---
static vertex_buffer_t *rects_buffer;
static GLuint shader,
           mz_shader;
static mat4 model, view, projection;
//static GLuint g_TimeSlot = 0;
static vec2 resolution;

// from main.c
extern double u_t;
extern GLfloat p1_pos_x,
               p1_pos_y;
// ---------------------------------------------------------------- display ---
void pixelshader_render( void )
{
    glUseProgram( mz_shader );
    {
        /* glslsandbox uniforms
uniform float time;
uniform vec2  resolution;
uniform vec2  mouse;
*/
        // notify shader about screen size
        glUniform2f       ( glGetUniformLocation( mz_shader, "resolution" ), resolution.x, resolution.y);
        glUniform2f       ( glGetUniformLocation( mz_shader, "mouse" ),      p1_pos_x, p1_pos_y);
        glUniform1f       ( glGetUniformLocation( mz_shader, "time" ),       u_t); // notify shader about elapsed time
        // ft-gl style: MVP
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
        case 0: fShader = (void*) orbisFileGetFileContent( "shaders/v3f-c4f.frag" );         break;
        case 1: fShader = (void*) orbisFileGetFileContent( "shaders/mz.frag" );              break;
        case 2: fShader = (void*) orbisFileGetFileContent( "/hostapp/assets/ps_signs.shader" ); break;
        case 3: fShader = (void*) orbisFileGetFileContent( "shaders/iTime.frag" ); break;
    }
    // use shader_common.c
    GLuint programID = BuildProgram(vShader, fShader);

    /* don't leak what we readed from files! */
    if (vShader) free(vShader), vShader = NULL;
    if (fShader) free(fShader), fShader = NULL;
#endif

    if (!programID) { fprintf(stderr, "program creation failed\n"); sleep(2); }
    // feedback
    printf( "[%s] program_id=%d (0x%08x)\n", __FUNCTION__, programID, programID);

    return programID;
}

// ------------------------------------------------------------------- init ---
void pixelshader_init( int width, int height )
{
    resolution   = (vec2) { width, height };
    rects_buffer = vertex_buffer_new( "vertex:3f,color:4f" );
    vec4   color = { 1, 0, 0, 1 };
    float r = color.r, g = color.g, b = color.b, a = color.a;

#define TEST_1  (0)

#if TEST_1
    #warning "splitted rects pixelshader"
    vec2 pen = { 100, 100 };
    for (int i = 0; i < 10; ++i)
    {
        int  x0 = (int)( pen.x + i * 2 );
        int  y0 = (int)( pen.y + 20 );
        int  x1 = (int)( x0 + 64 );
        int  y1 = (int)( y0 - 64 );
#else
    #warning "one fullstreen pixelshader"
    int  x0 = (int)( 0 );
    int  y0 = (int)( 0 );
    int  x1 = (int)( width );
    int  y1 = (int)( height );

#endif
    GLuint idx[6] = {0,1,2,  0,2,3}; // (two triangles)

    /* VBO is setup as: "vertex:3f, color:4f */ 
    vertex_t vtx[4] = { { x0,y0,0,  r,g,b,a },
                        { x0,y1,0,  r,g,b,a },
                        { x1,y1,0,  r,g,b,a },
                        { x1,y0,0,  r,g,b,a } };
    vertex_buffer_push_back( rects_buffer, vtx, 4, idx, 6 );

#if TEST_1
        pen     += (vec2){ 72., -32. };
        color.g -= i * 0.1; // to show some difference: less green
    }
#endif
    /* compile, link and use shader */
    /* load from file */
    //shader    = CreateProgram(0);
    mz_shader = CreateProgram(1);

    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );

    reshape(width, height);
}

void pixelshader_fini( void )
{
    vertex_buffer_delete(rects_buffer);    rects_buffer = NULL;

    if(shader)    glDeleteProgram(shader),    shader    = 0;
    if(mz_shader) glDeleteProgram(mz_shader), mz_shader = 0;
}