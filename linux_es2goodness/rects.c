/*
    mimic an SDL_Rect

    standard GLES2 code to draw FillRects
*/

#if defined (__PS4__)

#include <ps4sdk.h>
#include <debugnet.h>
#include <orbisGl.h>
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

// Clang Extended Vectors
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float vec4 __attribute__((ext_vector_type(4)));

/* simple shaders */
static const char *vs =
    "precision mediump float;"
    "attribute vec4 a_Position;"
    "uniform   vec4 u_color;"
    //use your own output instead of gl_FrontColor
    "varying   vec4 fragColor;"
    ""
    "void main(void)"
    "{"
    "  fragColor   = u_color;"
    "  gl_Position = a_Position;"
    "}";

static const char *fs =
/// 1. default, use texture color
    "precision mediump float;"
    "varying   vec4    fragColor;"
    ""
    "void main(void)"
    "{"
    "  gl_FragColor = fragColor;"
    "}";

static GLuint simpleProgram = 0;
static vec2   resolution;
       vec4   color = { 1., 0., .5, 1. }; // RGBA
// shaders locations
static GLint  a_position_location;
static GLint  u_color_location;

typedef struct
{
    float x;
    float y;
} SDL_FPoint;

typedef struct
{
    float h;
    float w;
    float x;
    float y;
} SDL_FRect;


int ORBIS_RenderDrawLines(//SDL_Renderer *renderer,
    const SDL_FPoint *points, int count)
{
    GLfloat vertices[4];
    int idx;

    glUseProgram(simpleProgram);

    //glDisable(GL_CULL_FACE);
    // enable alpha
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* emit a line loop for each point pair */
    for (idx = 0; idx < count /2; idx+=2) {
        const SDL_FPoint *p1 = &points[idx   ],
                         *p2 = &points[idx +1];
        //printf("%f,%f,%f,%f\n", p1->x, p1->y, p2->x, p2->y);
        /* (x, y) for 2 points: 4*/
        vertices[0] = p1->x;  vertices[1] = p1->y;
        vertices[2] = p2->x;  vertices[3] = p2->y;
        /* each (vec2)point comes from pairs of floats */
        glVertexAttribPointer    (a_position_location, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(a_position_location);
        /* write color to use to the shader location */
        glUniform4f(u_color_location, color.r, color.g, color.b, color.a);
        /* floats pairs for points from 0-4 */
        glDrawArrays(GL_LINES, 0, 2);
    }

    // revert state back
    glDisable(GL_BLEND);

    // release VBO, texture and program
    glUseProgram(0);

    return 0;
}

int ORBIS_RenderFillRects(
    // SDL_Renderer *renderer,
    const SDL_FRect *rects, int count)
{
    GLfloat vertices[8];
    int idx;

    glUseProgram(simpleProgram);

    //glDisable(GL_CULL_FACE);
    // enable alpha
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* emit a triangle strip for each rectangle */
    for (idx = 0; idx < count; ++idx) {
        const SDL_FRect *rect = &rects[idx];

        GLfloat xMin = rect->x,  xMax = (rect->x + rect->w),
                yMin = rect->y,  yMax = (rect->y + rect->h);
        /* (x, y) for 4 points: 8*/
        vertices[0] = xMin;  vertices[1] = yMin;
        vertices[2] = xMax;  vertices[3] = yMin;
        vertices[4] = xMin;  vertices[5] = yMax;
        vertices[6] = xMax;  vertices[7] = yMax;
        /* each (vec2)point comes from pairs of floats */
        glVertexAttribPointer    (a_position_location, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(a_position_location);
        /* write color to use to the shader location */
        glUniform4f(u_color_location, color.r, color.g, color.b, color.a);
        /* floats pairs for points from 0-4 */
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // revert state back
    glDisable(GL_BLEND);

    // release VBO, texture and program
    glUseProgram(0);

    return 0;
}

void ORBIS_RenderFillRects_init( int width, int height )
{
    resolution = (vec2){ width, height };

    simpleProgram = BuildProgram(vs, fs);
    // select for setup
    glUseProgram(simpleProgram);
    // gles2 attach shader locations
    a_position_location = glGetAttribLocation (simpleProgram, "a_Position");
    u_color_location    = glGetUniformLocation(simpleProgram, "u_color");
    // reshape
    glViewport(0, 0, width, height);
}


vec4 px_pos_to_normalized(vec2 *pos, vec2 *size)
{
    vec4 n; // 2 points .xy pair: (x, y),  (x + texture.w, y + texture.h)

    n.xy  = -1. + 2. / resolution * (*pos); // (-1,-1) is BOTTOMLEFT, (1,1) is UPRIGHT
    n.zw  = 2. * *size / resolution;
    n.yw *= -1.; // flip Y axis!
//  printf("%f,%f,%f,%f\n", n.x, n.y, n.w, n.w);
    return n;
}

#define COUNT  6
void ORBIS_RenderFillRects_rndr(void)
{
    SDL_FRect r[COUNT];
    vec4 nr;
    // fill in some pos, size
    for (int i = 0; i < COUNT; ++i)
    {
        vec2 p = (vec2) { 100. + i * 20., 100. + i * 20. },  // position in px
             s = (vec2) {  10. + i * 10,   10. + i * 20. };  // size in px
        /* convert to normalized coordinates */
            nr = px_pos_to_normalized( &p, &s );
        r[i].x = nr.x, r[i].y = nr.y;
        r[i].w = nr.z, r[i].h = nr.w;
    }
    // gles render all rects
    ORBIS_RenderFillRects(r, COUNT);

// test lines
    SDL_FPoint p[2];
    p[0].x = -.4, p[0].y = .4;
    p[1].x = -.2, p[1].y = .6;
    ORBIS_RenderDrawLines(&p[0], 2);
}

void ORBIS_RenderFillRects_fini(void)
{
    if (simpleProgram) glDeleteProgram(simpleProgram), simpleProgram = 0;
}
