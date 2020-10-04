/*
    mimic an SDL_Rect

    standard GLES2 code to DrawLines and FillRects
*/

#include "defines.h"


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
/// 1. use passed u_color
    "precision mediump float;"
    "varying   vec4    fragColor;"
    ""
    "void main(void)"
    "{"
    "  gl_FragColor = fragColor;"
    "}";

static GLuint simpleProgram = 0;
static vec2   resolution;
       vec4   color = { 1., 0., .5, 1. }; // current RGBA color
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

// takes point count
void ORBIS_RenderDrawLines(const vec2 *points, int count)
{
    GLfloat vertices[4];
    int idx;

    glUseProgram(simpleProgram);

    //glDisable(GL_CULL_FACE);
    // enable alpha
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* emit a line for each point pair */
    for (idx = 0; idx <= count /2; idx+=2) {
        const vec2 *p1 = &points[idx   ],
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
}

void ORBIS_RenderFillRects(const vec4 *rects, int count)
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
        const vec4 *rect = &rects[idx];

        GLfloat xMin = rect->x,  xMax = (rect->x + rect->z),
                yMin = rect->y,  yMax = (rect->y + rect->w);
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


vec4 px_pos_to_normalized2(vec2 *pos, vec2 *size)
{
    vec4 n; // 2 points .xy pair: (x, y),  (x + texture.w, y + texture.h)

    n.xy  = 2. / resolution * (*pos) - 1.; // (-1,-1) is BOTTOMLEFT, (1,1) is UPRIGHT
    n.zw  = 2. / resolution * (*size);
    n.yw *= -1.; // flip Y axis!
//  printf("%f,%f,%f,%f\n", n.x, n.y, n.w, n.w);
    return n;
}

vec2 px_pos_to_normalized(vec2 *pos)
{
    vec2 n; // 2 points .xy pair: (x, y)

    n.xy  =  2. / resolution * (*pos) - 1.; // (-1,-1) is BOTTOMLEFT, (1,1) is UPRIGHT
    n.y  *= -1.; // flip Y axis!
//  printf("%f,%f,%f,%f\n", n.x, n.y, n.w, n.w);
    return n;
}

#define COUNT  6
void ORBIS_RenderFillRects_rndr(void)
{
    vec4 r[COUNT];
    vec2 s;
    // fill in some pos, size
    for (int i = 0; i < COUNT; ++i)
    {
        vec2 p = (vec2) { 100. + i * 20.,  100. + i * 20. },  // position in px
        /* convert to normalized coordinates */
        n = px_pos_to_normalized(&p);
        r[i].x = n.x, r[i].y = n.y;

        s  = (vec2) { 10. + i * 10,  10. + i * 20. };  // size in px
        s += p;
        /* convert to normalized coordinates */
        n  = px_pos_to_normalized(&s);
        r[i].z = n.x - r[i].x, r[i].w = n.y - r[i].y;
    }
    // gles render all rects
    ORBIS_RenderFillRects(r, COUNT);

#if defined TETRIS
    // test tetris primlib
    filledRect(1000, 400, 1010, 410, 255, 0, 0); // p1, p2
#endif

// test lines
    vec2 p[2];
    p[0].x = -.4, p[0].y = .4;
    p[1].x = -.2, p[1].y = .6;
    ORBIS_RenderDrawLines(&p[0], 2);
}

void ORBIS_RenderFillRects_fini(void)
{
    if (simpleProgram) glDeleteProgram(simpleProgram), simpleProgram = 0;
}
