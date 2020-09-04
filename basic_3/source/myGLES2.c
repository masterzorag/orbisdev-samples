/*
    standard GLES2 renderer to sample and display
    an RGB565, 2byte,16bitmap and show fullscreen

    2019, 2020, masterzorag
*/


#include <stdlib.h> // calloc
#include <string.h> // memcpy, memset, strlen
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "mySDL.h" // custom: minimal for the surface ref


#if defined (__ORBIS__)

#include <ps4sdk.h>
#include <debugnet.h>
#define  fprintf  debugNetPrintf
#define  ERROR    DEBUGNET_ERROR
#define  DEBUG    DEBUGNET_DEBUG
#define  INFO     DEBUGNET_INFO

#else // on linux

#include <stdio.h>
#include <string.h>
#define  debugNetPrintf  fprintf
#define  ERROR           stderr
#define  DEBUG           stdout
#define  INFO            stdout

#endif // (__ORBIS__)


// Clang Extended Vectors
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float vec4 __attribute__((ext_vector_type(4)));


/* simple shaders */
static const char *vs =
    "precision mediump float;"
    "attribute vec4  a_Position;"
    "attribute vec2  a_TextureCoordinates;"
    "varying   vec2  v_TextureCoordinates;"
    "void main(void)"
    "{"
    "  v_TextureCoordinates = a_TextureCoordinates;"
    "  gl_Position          = a_Position;"
    "}";

static const char *fs =
	// default, use texture color
    "precision mediump float;"
    "uniform   sampler2D u_TextureUnit;"
    "varying   vec2      v_TextureCoordinates;"
    ""
    "void main(void)"
    "{"
    "  vec4 col = texture2D(u_TextureUnit, v_TextureCoordinates);"
    "  vec4 c   = col.brga;"  //argb (swap some color channels), unused!
    "  gl_FragColor   = col;"
   	"  gl_FragColor.a = 1.;"
    "}";


static GLuint simpleProgram;
static GLuint texture;
static GLuint buffer;
#define BUFFER_OFFSET(i) ((void*)(i))

// shader locations
static GLint a_position_location;
static GLint a_texture_coordinates_location;
static GLint u_texture_unit_location;

// a fullscreen texture: position (X, Y), texture (U, V)
static const float rect[] = { -1.0f, -1.0f, 0.0f, 1.0f,
                              -1.0f,  1.0f, 0.0f, 0.0f,
                               1.0f, -1.0f, 1.0f, 1.0f,
                               1.0f,  1.0f, 1.0f, 0.0f}; // not Y flipped!
//static vec2 resolution;  // (constant)

static GLuint update_texture(
    const GLsizei width, const GLsizei height,
    const GLenum  type,  const GLvoid *data)
{
    GLuint texture_object_id;
    // create new OpenGL texture
    glGenTextures(1, &texture_object_id);
	// assert(texture_object_id != 0);
    glBindTexture(GL_TEXTURE_2D, texture_object_id);
    // set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate texture from bitmap data
    glTexImage2D(GL_TEXTURE_2D, 0, 
                 	type,      // internalformat: GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA. 
                 	width, height,
                 	0,
                 	type,      // Must match internalformat
                 	GL_UNSIGNED_SHORT_5_6_5, // data type of the texel data: GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, and GL_UNSIGNED_SHORT_5_5_5_1
                 	data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture_object_id;
}

void GLES2_Init_video(int view_w, int view_h)
{
    //resolution = (vec2){ view_w, view_h }; // setup resolution for next setup_texture_position()
    buffer = create_vbo(sizeof(rect), rect, GL_STATIC_DRAW); // fullscreen background

    #if 1//HAVE_SHACC
      simpleProgram = BuildProgram(vs, fs);
    #else
      simpleProgram = 0;//CreateProgramFromBinary("host0:compiled/vert.sb", "host0:compiled/frag.sb");
    #endif
    fprintf(INFO, "simpleProgram: %u, texture:%d\n", simpleProgram, texture);

    glUseProgram(simpleProgram);
    a_position_location            = glGetAttribLocation (simpleProgram, "a_Position");
    a_texture_coordinates_location = glGetAttribLocation (simpleProgram, "a_TextureCoordinates");
    u_texture_unit_location        = glGetUniformLocation(simpleProgram, "u_TextureUnit");
 
    glViewport(0, 0, view_w, view_h);
}

void GLES2_Update_video(SDL_Texture *user_texture)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    // destroy old texture!
    if(texture) glDeleteTextures(1, &texture), texture = 0;
    // resample a new texture from rgba_data, as RGB565
    texture = update_texture(user_texture->w, user_texture->h, GL_RGB, user_texture->pixels);
}

void GLES2_Render_video(int unused)
{
    // we already clean

    glUseProgram(simpleProgram);

    glDisable(GL_CULL_FACE);
    // enable alpha for png textures
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // select requested texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture  (GL_TEXTURE_2D, texture);
    glUniform1i    (u_texture_unit_location, 0); // tell to shader
    glBindBuffer   (GL_ARRAY_BUFFER, buffer);    // bind requested VBO
    // setup attr
    glVertexAttribPointer(a_position_location,
        2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), BUFFER_OFFSET(0));
    glVertexAttribPointer(a_texture_coordinates_location,
        2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), BUFFER_OFFSET(2 * sizeof(GL_FLOAT)));
    // pin variables
    glEnableVertexAttribArray(a_position_location);
    glEnableVertexAttribArray(a_texture_coordinates_location);
 	// draw binded VBO buffer
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    // revert state back
    glDisable(GL_BLEND);
    // release VBO, texture and program
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glActiveTexture(0); // error on piglet !!
    glUseProgram(0);

    // we already flip/swap
}
