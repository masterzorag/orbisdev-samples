/*
    liborbis implementation, based off libMiniAPI included Shader viewer sample

    2019, masterzorag 
 */
/*****************************************************************************
 * ==> Shader viewer --------------------------------------------------------*
 *****************************************************************************
 * Description : A shader viewer tool                                        *
 * Developer   : Jean-Milost Reymond                                         *
 * Copyright   : 2015 - 2018, this file is part of the Minimal API. You are  *
 *               free to copy or redistribute this file, modify it, or use   *
 *               it for your own projects, commercial or not. This file is   *
 *               provided "as is", without ANY WARRANTY OF ANY KIND          *
 *****************************************************************************/

// std
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


// mini API
#include "MiniCommon.h"
#include "MiniGeometry.h"
#include "MiniVertex.h"
#include "MiniShapes.h"
#include "MiniShader.h"
#include "MiniRenderer.h"

//pc_MAPI_samples # clang egl.c MAPI_shader_viewer.c -I$PS4SDK/include/MiniAPI -L.. -lMiniAPI -lm -lGL -lEGL -lX11 -D_MAPI_
#include "defines.h"

#define ICHANNEL0_TEXTURE_FILE  "iChannel0.png"

//------------------------------------------------------------------------------
// one _shared_ generic vertex shader program
const char *g_pVertexShader =
    "precision mediump float;"
    "attribute vec3  mini_aPosition;"
    // glslsandbox uniforms!
    "uniform float time;"
    "uniform vec2  resolution;"
    "uniform vec2  mouse;"
/*    "uniform   float mini_uTime;"
    "uniform   vec2  mini_uResolution;"
    "uniform   vec2  mini_uMouse;"
    "varying   float iTime;"
    "varying   vec2  iResolution;"
    "varying   vec2  iMouse;"
    ""
*/  "void main(void)"
    "{"
//    "    iResolution = mini_uResolution;"
//    "    iTime       = mini_uTime;"
//    "    iMouse      = mini_uMouse;"
    "    gl_Position = vec4(mini_aPosition, 1.0);"
    "}";

// a first fragment shader (iTime sample)
const char *g_pshadersource1 =
    "precision mediump float; \
     varying float iTime; \
     varying vec2  iResolution; \
     void main() \
     { \
         vec2 uv = gl_FragCoord.xy / iResolution.xy; \
         vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4)); \
         gl_FragColor = vec4(col,1.0); \
     }";

// a second fragment shader (lightning)
// By: Brandon Fogerty
const char *g_pshadersource =
       "precision mediump float;\
        varying float iTime;\
        varying vec2  iResolution;\
        varying vec2  iMouse;\
        \
        const float count = 10.1;\
        const float speed = 10.1;\
        \
        float Hash( vec2 p, in float s)\
        {\
            vec3 p2 = vec3(p.xy,27.0 * abs(sin(s)));\
            return fract(sin(dot(p2,vec3(27.1,61.7, 12.4)))*2.1);\
        }\
        \
        float noise(in vec2 p, in float s)\
        {\
            vec2 i = floor(p);\
            vec2 f = fract(p);\
            f *= f * (3.0-2.0*f);\
            \
            return mix(mix(Hash(i + vec2(0.,0.), s), Hash(i + vec2(1.,0.), s),f.x),\
                       mix(Hash(i + vec2(0.,1.), s), Hash(i + vec2(1.,1.), s),f.x),\
                       f.y) * s;\
        }\
        \
        float fbm(vec2 p)\
        {\
            float v = 0.0;\
            v += noise(p*1., 0.35);\
            return v;\
        }\
        \
        void main( void )\
        {\
            float worktime = iTime * speed;\
            \
            vec2 uv = ( gl_FragCoord.xy / iResolution.xy ) * 2.0 - 1.0;\
            uv.x *= iResolution.x/iResolution.y;\
            uv *= 0.5;\
            \
            vec3 finalColor = vec3( 0.0 );\
            \
            for( float i=1.; i < count; ++i )\
            {\
                float t = abs(1.0 / ((uv.x + fbm( uv + worktime/i)) * (i*50.0)));\
                finalColor +=  t * vec3( i * 0.075 +0.1, 0.5, 2.0 );\
            }\
         gl_FragColor = vec4( finalColor, 1.0 );\
       }";
//------------------------------------------------------------------------------
#include "menu.h"
#define NUM_OF_PROGRAMS  (2+1)  // CLOSED and MAIN, + LOADED

MINI_Shader        g_Shader            [NUM_OF_PROGRAMS];
GLuint             g_ShaderProgram     [NUM_OF_PROGRAMS];
MINI_VertexFormat  g_VertexFormat      [NUM_OF_PROGRAMS];
float*             g_pSurfaceVB        [NUM_OF_PROGRAMS];
unsigned int       g_SurfaceVertexCount[NUM_OF_PROGRAMS];

// shared between shaders
const float        g_SurfaceWidth     = 10.0f;
const float        g_SurfaceHeight    = 12.5f;
float              g_Time             = 0.0f;
GLuint             g_TimeSlot         = 0;
GLuint             g_ResolutionSlot   = 0;
MINI_Vector2       g_Resolution;      // dimension (x, y)
GLuint             g_MouseSlot        = 0;
MINI_Vector2       g_MousePos;        // position (x, y)

/// second shader can use iChannel0.png as Texture
GLuint             g_TextureIndex     = GL_INVALID_VALUE;
GLuint             g_TexSamplerSlot   = 0; // iChannel0

int                g_SceneInitialized = 0;
//unsigned long long g_PreviousTime   = 0L;
//------------------------------------------------------------------------------
static void CreateViewport(float w, float h)
{
    // create the OpenGL viewport
    glViewport(0, 0, w, h);

    // set the screen resolution
    g_Resolution.m_X = w;
    g_Resolution.m_Y = h;
}


//------------------------------------------------------------------------------
int InitScene_1(int i, int w, int h, const char *pShader, int binary)
{
    g_pSurfaceVB[i]         = 0;
    g_SurfaceVertexCount[i] = 0;
    // we will use pad to hook mouse coord: reset center position
    g_MousePos.m_X = g_MousePos.m_Y = 0.0f;

    // compile, link and use shader
#if 0

    #if 1 // HAVE_SHACC
      simpleProgram = BuildProgram(vs, fs);
    #else
      simpleProgram = CreateProgramFromBinary("host0:compiled/vert.sb", "host0:compiled/frag.sb");
    #endif
    printf("simpleProgram: %u\n", simpleProgram);

 
    if(binary == 1)
    {
        g_ShaderProgram[i] = CreateProgramFromBinary("host0:compiled/g_pVertexShader.sb", pShader);
    }
    else // use shacc to build from source
#endif
    {
        unsigned char *data = NULL;
                 char *name = NULL;

        switch(i) // address fragment program source by scene_num
        {
            case CLOSED: pShader = (const char *)g_pshadersource1; break;
            case MAIN:   pShader = (const char *)g_pshadersource;  break;
            case LOADED: /* the custom loaded */ 
               { name = strdup(pShader);
                 data = orbisFileGetFileContent(pShader); // read the shader content
                 if(data)
                   pShader = data;
               } break;
        }

        g_ShaderProgram[i] = BuildProgram(g_pVertexShader, pShader);

        fprintf(DEBUG, "g_ShaderProgram[%d]: %d\n", i, g_ShaderProgram[i]);
    
        if(i == LOADED) /// dump those now, in .sb format
        {
        /*  DumpShader(fragmentShader, name); */
            if(name) free(name), name = NULL;
            if(data) free(data), data = NULL;
        }
    }

    if(!g_ShaderProgram[i])
    {
        fprintf(INFO, "g_ShaderProgram[%d]: %d\n", i, g_ShaderProgram[i]);
        sleep(3);
        return 0;
    }

    glUseProgram(g_ShaderProgram[i]);  // we setup this one

    // get shader attributes
    g_Shader[i].m_VertexSlot = glGetAttribLocation (g_ShaderProgram[i], "mini_aPosition");
    g_TimeSlot               = glGetUniformLocation(g_ShaderProgram[i], "time");
    g_ResolutionSlot         = glGetUniformLocation(g_ShaderProgram[i], "resolution");
    g_MouseSlot              = glGetUniformLocation(g_ShaderProgram[i], "mouse");
    g_TexSamplerSlot         = glGetAttribLocation (g_ShaderProgram[i], "iChannel0"); // for texture

    // create the viewport
    CreateViewport(w, h);
    //fprintf(DEBUG, "g_ResolutionSlot: %f %f\n", g_Resolution.m_X, g_Resolution.m_Y);

    // notify shader about screen size
    glUniform2f(g_ResolutionSlot, g_Resolution.m_X, g_Resolution.m_Y);

    // initialize the mouse (or finger) position in the shader
    glUniform2f(g_MouseSlot, 0, 0);

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    g_VertexFormat[i].m_UseNormals  = 0;
    g_VertexFormat[i].m_UseTextures = 0;
    g_VertexFormat[i].m_UseColors   = 0;

    // generate surface
    miniCreateSurface(&g_SurfaceWidth,
                      &g_SurfaceHeight,
                       0xFFFFFFFF,
                      &g_VertexFormat[i],
                      &g_pSurfaceVB[i],
                      &g_SurfaceVertexCount[i]);
#if USE_PNG
    if(i > LOADED)
    {
        // load iChannel0 texture
        if (g_TextureIndex == GL_INVALID_VALUE)
            g_TextureIndex = load_png_asset_into_texture(ICHANNEL0_TEXTURE_FILE);
        fprintf(DEBUG, "load_png_asset_into_texture ret: %d\n", g_TextureIndex);

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(g_TexSamplerSlot, GL_TEXTURE0);
    }
#endif

    g_SceneInitialized = 1;
    return 1;
}

//------------------------------------------------------------------------------
void DeleteScene_1(int i)
{
    if (g_pSurfaceVB[i]) // delete surface vertices
    {
        free(g_pSurfaceVB[i]); g_pSurfaceVB[i] = 0;
    }

    // iChannel0
    if (g_TextureIndex != GL_INVALID_VALUE)
        glDeleteTextures(1, &g_TextureIndex);

    g_TextureIndex = GL_INVALID_VALUE;

    // delete shader program
    if (g_ShaderProgram[i])
        glDeleteProgram(g_ShaderProgram[i]);

    g_ShaderProgram[i] = 0;
}

int InitScene_reload(int i, char *pShader, int binary)
{
    if (g_pSurfaceVB[i]) // delete surface vertices
    {
        free(g_pSurfaceVB[i]); g_pSurfaceVB[i] = 0;
    }
    // delete shader program
    if (g_ShaderProgram[i])
        glDeleteProgram(g_ShaderProgram[i]);

    g_ShaderProgram[i] = 0;
    
    //sceKernelUsleep(1000);

    int ret = InitScene_1(i, g_Resolution.m_X, g_Resolution.m_Y, pShader, binary);
    fprintf(DEBUG, "InitScene_load(%d, %p) ret: %d\n", i, pShader, ret);

    return ret;
}

//------------------------------------------------------------------------------
static float frame = 0.;
extern float p1_pos_x, p1_pos_y; // from main()
void UpdateScene_1(int i, double elapsedTime)
{
    elapsedTime = frame + 0.02f;

    g_Time += (float)( elapsedTime * 1.f );  // calculate next time

    glUseProgram(g_ShaderProgram[i]);

    // notify shader about elapsed time
    glUniform1f(g_TimeSlot, g_Time);

    #ifdef __PS4__
    // notify shader about mouse position
    glUniform2f(g_MouseSlot, g_MousePos.m_X, g_MousePos.m_Y);
    //fprintf(DEBUG, "g_Time = %f %f\n", g_Time, sin(g_Time));

    #else
    // notify shader about mouse position
    glUniform2f(g_MouseSlot, p1_pos_x, p1_pos_y);

    //printf("g_Time = %f %f\n", g_Time, sin(g_Time));
    //printf("update_pos: %f %f\n", p1_pos_x, p1_pos_y);

    #endif
}
//------------------------------------------------------------------------------
int scene_num = 0;
void DrawScene_1(v4i *pos)
{
    // we already clean

    if (pos)
    {
        if(!g_ShaderProgram[pos->z]) pos->z = CLOSED; // if no program, fallback to default
    }

    int i = 2;//scene_num %NUM_OF_PROGRAMS;
    //int i = pos->z;

    glUseProgram(g_ShaderProgram[i]);

    if(i > 1)
    {
        // refresh state for this shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_TextureIndex);
    }

    // configure OpenGL depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRangef(0.0f, 1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    //miniBeginScene(0.0f, 0.0f, 0.0f, 1.0f);// we already clean in main render loop

    // draw the surface on which the shader will be executed
    miniDrawSurface(g_pSurfaceVB[i],
                    g_SurfaceVertexCount[i],
                   &g_VertexFormat[i],
                   &g_Shader[i]);

    //miniEndScene(); // a stub, does nothing

    // we already flip/swap
}
//------------------------------------------------------------------------------
#ifdef __PS4__
// hook pad over miniMouse vec2(x, y)
// we get feedback on each keypress
#define STEP  (0.005)  // a small positive delta
void pad_special(int special)
{
    switch (special)
    {
        case 0: //_KEY_LEFT:
            if(g_MousePos.m_X > -1.000) g_MousePos.m_X -= STEP;
            break;
        case 1: //_KEY_RIGHT:
            if(g_MousePos.m_X < 1.000)  g_MousePos.m_X += STEP;
            break;
        case 2: //_KEY_UP:
            if(g_MousePos.m_Y < 1.000)  g_MousePos.m_Y += STEP;
            /*scene_num++;
            sceKernelUsleep(100000);*/
            //fprintf(DEBUG, "scene_num = %d %d\n", scene_num, scene_num %2);
            break;
        case 3: //_KEY_DOWN:
            if(g_MousePos.m_Y > -1.000) g_MousePos.m_Y -= STEP;
            break;
        /*
        case 4: //_BUTTON_X:
            DeleteScene_1();
            InitScene_1(ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);
            fprintf(DEBUG, "Re-Init Scene!\n");
            break;
        */
        default:
            /*do nothing*/
            break;
   }

   sceKernelUsleep(4096);
   fprintf(DEBUG, "update_pos: %f %f\n", g_MousePos.m_X, g_MousePos.m_Y);
}
#endif
