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

typedef vec2  my_Point;
typedef vec4  my_FRect;

extern int selected_icon;   // from icons.c

// takes point count
int ORBIS_RenderDrawLines(//SDL_Renderer *renderer,
    const vec2 *points, int count)
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
    for (idx = 0; idx < count; idx += 2) {
#if 0
        printf("%d, %d, %d %.3f,%.3f,%.3f,%.3f\n", 
            idx, count, count/2,
            points[idx   ].x, points[idx   ].y,
            points[idx +1].x, points[idx +1].y);
#endif
        /* (x, y) for 2 points: 4 vertices */
        vertices[0] = points[idx   ].x;  vertices[1] = points[idx   ].y;
        vertices[2] = points[idx +1].x;  vertices[3] = points[idx +1].y;
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
    const my_FRect *rects, int count)
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
        const my_FRect *rect = &rects[idx];

        GLfloat xMin = rect->x,  xMax = rect->z,
                yMin = rect->y,  yMax = rect->w;
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

#define COUNT  8
void ORBIS_RenderFillRects_rndr(void)
{
    //SDL_FRect r[COUNT];
    my_FRect r[COUNT];
    vec2 n, s;
    // fill in some pos, and size
    for (int i = 0; i < COUNT; ++i)
    {
        // position in px
        vec2 p = (vec2) { 32. + i * (100. + 1. /*border*/),  
                         100. + 4 *  20. },
        /* convert to normalized coordinates */
        n = px_pos_to_normalized(&p);
        //r[i].x = n.x, r[i].y = n.y;
        r[i].xy = n;

        s  = (vec2) { 100.,  100. };  // size in px
        s += p;
        /* convert to normalized coordinates */
        n  = px_pos_to_normalized(&s);
        //r[i].w = n.x - r[i].x, r[i].h = n.y - r[i].y;
        r[i].zw = n;
    }
    // gles render all rects
    ORBIS_RenderFillRects(r, COUNT);

#if defined TETRIS
    // test tetris primlib
    filledRect(1000, 400, 1010, 410, 255, 0, 0); // p1, p2
#endif

// test lines
    my_Point p[2];
    p[0].x = -.4, p[0].y = .4;
    p[1].x = -.2, p[1].y = .6;
    ORBIS_RenderDrawLines(&p[0], 2);

    // draw a box around selected rect
    vec4 curr_color = color;
    color = (vec4) { 1., 1., 1., 1. }; // set white
    my_Point b[8];

    vec2 v[4];
    int i   = selected_icon;
    v[0]    = r[i].xy;
    v[1]    = r[i].xw;
    v[2]    = r[i].zw;
    v[3]    = r[i].zy;
    // 1 line for each 2 points
    b[0] = v[0],  b[1] = v[1];
    b[2] = v[1],  b[3] = v[2];
    b[4] = v[2],  b[5] = v[3];
    b[6] = v[3],  b[7] = v[0];

    ORBIS_RenderDrawLines(&b[0*2], 8);
    // restore current color
    color = curr_color;
}

void ORBIS_RenderFillRects_fini(void)
{
    if (simpleProgram) glDeleteProgram(simpleProgram), simpleProgram = 0;
}

