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
typedef vec4  my_FRect; // ( p1.xy, p2.xy )

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
    GLfloat vertices[8]; // (4 float pairs!)

    glUseProgram(simpleProgram);
    //glDisable(GL_CULL_FACE);
    // enable alpha
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* emit a triangle strip for each rectangle */
    for(int i = 0; i < count; ++i)
    {
        const my_FRect *rect = &rects[i];

        GLfloat xMin = rect->x,  xMax = rect->z,
                yMin = rect->y,  yMax = rect->w;
        /* (x, y) for 4 points: 8 */
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
    // release VBO, texture, program, ...
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

// useless
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
    vec2 n; // 2 points .xy pair
    // (-1,-1) is BOTTOMLEFT, (1,1) is UPRIGHT
    n.xy  =  2. / resolution * (*pos) - 1.;
    n.y  *= -1.;  // flip Y axis!
//  printf("n(%f, %f)\n", n.x, n.y);
    return n;
}

#define COUNT  8
void ORBIS_RenderFillRects_rndr(void)
{
    my_FRect r[COUNT];
    vec2 s;
    // fill in some pos, and size
    for (int i = 0; i < COUNT; ++i)
    {
        // position in px
        vec2 p = (vec2) { 32. + i * (100. + 1. /*border*/),  
                         100. + 4 *  20. };
        /* convert to normalized coordinates */
        r[i].xy = px_pos_to_normalized(&p);
        // size in px
        s  = (vec2) { 100.,  100. };
        // turn size into destination point!
        s += p;
        /* convert to normalized coordinates */
        r[i].zw = px_pos_to_normalized(&s);
    }
    // gles render all rects
    ORBIS_RenderFillRects(r, COUNT);

#if defined TETRIS
    // test tetris primlib
    filledRect(1000, 400, 1010, 410, 255, 0, 0); // p1, p2
#endif

    /* now test a line, same color */
    my_Point p[2];
    p[0].x = -.4, p[0].y = .4;
    p[1].x = -.2, p[1].y = .6;
    ORBIS_RenderDrawLines(&p[0], 2);


    /* draw a white box around selected rect */
    vec4 curr_color = color;
    // vector splat 1 -> set white
    color = (vec4) ( 1. );
    // a box is 4 segments joining 2 points
    vec2 b[4 * 2];
    // 1 line for each 2 points
    int i = selected_icon;
    b[0]  = r[i].xy,  b[1] = r[i].xw;
    b[2]  = r[i].xw,  b[3] = r[i].zw;
    b[4]  = r[i].zw,  b[5] = r[i].zy;
    b[6]  = r[i].zy,  b[7] = r[i].xy;

    ORBIS_RenderDrawLines(&b[0], 8);
    // restore current color
    color = curr_color;

    /* ... */
}

void ORBIS_RenderFillRects_fini(void)
{
    if (simpleProgram) glDeleteProgram(simpleProgram), simpleProgram = 0;
}

