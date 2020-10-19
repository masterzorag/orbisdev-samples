/*
    mimic an SDL_Rect

    standard GLES2 code to DrawLines and FillRects
*/

#include "defines.h"


// Clang Extended Vectors
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float vec4 __attribute__((ext_vector_type(4)));


enum GLSL_programs
{
    USE_COLOR,
    USE_UTIME,
    NUM_OF_PROGRAMS
};

/* glsl programs */
static GLuint glsl_Program[NUM_OF_PROGRAMS];
static GLuint curr_Program;  // the current one

/* simple shaders */
static const char *vs[NUM_OF_PROGRAMS] =
{
    "precision mediump float;"
    "attribute vec4    a_Position;"
    "uniform   vec4    u_color;"
    "uniform   float   u_time;"
    // use your own output instead of gl_FrontColor
    "varying   vec4 fragColor;"
    ""
    "void main(void)"
    "{"
    "  fragColor    = u_color;"
    "  gl_Position  = a_Position;"
    "} "
    ,
    /// 2.
    "precision mediump float;"
    "attribute vec4    a_Position;"
    "uniform   vec4    u_color;"
    "uniform   float   u_time;"
    // use your own output instead of gl_FrontColor
    "varying   vec4 fragColor;"
    ""
    "void main(void)"
    "{"
    "  fragColor    = u_color;"
    "  gl_Position  = a_Position;"
    // apply a little of zooming
    "  gl_Position.w -= ( abs(sin(u_time)) * .009 );"
    "} "
} ;

static const char *fs[NUM_OF_PROGRAMS] =
{
/// 1. use passed u_color
    "precision mediump float;"
    "varying   vec4    fragColor;"
    "uniform   float   u_time;"
    ""
    "void main(void)"
    "{"
    "  gl_FragColor    = fragColor;"
     "} "
    ,
    /// 2.
    "precision mediump float;"
    "varying   vec4    fragColor;"
    "uniform   float   u_time;"
    ""
    "void main(void)"
    "{"
    "  gl_FragColor    = fragColor;"
    "  gl_FragColor.a *= abs(sin(u_time));"
    "} "
} ;

extern vec2   resolution;
static vec4   color = { 1., 0., .5, 1. }; // current RGBA color
// shaders locations
static GLint  a_position_location;
static GLint  u_color_location;
static GLint  u_time_location;
 // from main.c
extern double u_t;
extern int    selected_icon;
extern ivec4  rela_pos;

typedef vec2  my_Point;
typedef vec4  my_FRect; // ( p1.xy, p2.xy )

#define COUNT  8
vec4 r[COUNT];


// old way, tetris uses it
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
   // n.y  *= -1.;  // flip Y axis!
//  printf("n(%f, %f)\n", n.x, n.y);
    return n;
}


void ORBIS_RenderFillRects_init( int width, int height )
{
    resolution = (vec2){ width, height };

    for(int i = 0; i < NUM_OF_PROGRAMS; ++i)
    {
        glsl_Program[i] = BuildProgram(vs[i], fs[i]);

        curr_Program = glsl_Program[i];

        glUseProgram(curr_Program);
        // gles2 attach shader locations
        a_position_location = glGetAttribLocation (curr_Program, "a_Position");
        u_color_location    = glGetUniformLocation(curr_Program, "u_color");
        u_time_location     = glGetUniformLocation(curr_Program, "u_time");
    }
    curr_Program = glsl_Program[0];
    // select for setup
    glUseProgram(curr_Program);
    // reshape
    glViewport(0, 0, width, height);
}


void ORBIS_RenderFillRects_fini(void)
{
    for(int i = 0; i < NUM_OF_PROGRAMS; ++i)
    {
        if (glsl_Program[i]) glDeleteProgram(glsl_Program[i]), glsl_Program[i] = 0;
    }
}


// takes point count
void ORBIS_RenderDrawLines(//SDL_Renderer *renderer,
    const vec2 *points, int count)
{
    GLfloat vertices[4];

    glUseProgram(curr_Program);
    // enable alpha
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* emit a line for each point pair */
    for (int i = 0; i < count; i += 2) {
#if 0
        printf("%d, %d, %d %.3f,%.3f,%.3f,%.3f\n", 
            idx, count, count/2,
            points[idx   ].x, points[idx   ].y,
            points[idx +1].x, points[idx +1].y);
#endif
        /* (x, y) for 2 points: 4 vertices */
        vertices[0] = points[i   ].x;  vertices[1] = points[i   ].y;
        vertices[2] = points[i +1].x;  vertices[3] = points[i +1].y;
        /* each (vec2)point comes from pairs of floats */
        glVertexAttribPointer    (a_position_location, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(a_position_location);
        /* write color to use to the shader location */
        glUniform4f(u_color_location, color.r, color.g, color.b, color.a);
        /* write time to use to the shader location */
        glUniform1f(u_time_location, u_t);
        /* floats pairs for points from 0-4 */
        glDrawArrays(GL_LINES, 0, 2);
    }
    // revert state back
    glDisable(GL_BLEND);
    // release program
    glUseProgram(0);
}


/* draw a white box around selected rect */
void ORBIS_RenderDrawBox(const vec4 *r)
{
    vec4 curr_color = color;
    // vector splat 1 -> set white
    color = (vec4) ( 1. );
    // a box is 4 segments joining 2 points
    vec2 b[4 * 2];
    // 1 line for each 2 points
    b[0] = r->xy,  b[1] = r->xw;
    b[2] = r->xw,  b[3] = r->zw;
    b[4] = r->zw,  b[5] = r->zy;
    b[6] = r->zy,  b[7] = r->xy;
    // select shader to use
    curr_Program = glsl_Program[USE_UTIME];
    // gles render all lines
    ORBIS_RenderDrawLines(&b[0], 8);
    // restore current color
    color = curr_color;
}

void ORBIS_RenderFillRects(const vec4 *rects, int count)
{
    GLfloat vertices[8]; // (4 float pairs!)

    glUseProgram(curr_Program);
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
}


// used in line_and_rects sample
void ORBIS_RenderFillRects_rndr(void)
{
    vec2 p, s;
    // fill in some pos, and size
    for (int i = 0; i < COUNT; ++i)
    {
        // position in px
        p = (vec2) { 100. + i * (100. + 2. /*border*/),  
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
    // select shader to use
    curr_Program = glsl_Program[USE_UTIME];
    // gles render all rects
    ORBIS_RenderFillRects(r, COUNT);

#if defined TETRIS
    // test tetris primlib
    filledRect(1000, 400, 1010, 410, 255, 0, 0); // p1, p2
#endif

    /* now test a line, same color */
    my_Point q[2];
    q[0].x = -.4, q[0].y = .4;
    q[1].x = -.2, q[1].y = .6;
    ORBIS_RenderDrawLines(&q[0], 2);

    /* ... */
}



/* pos = rect.xy, size = rect.zw */
void ORBIS_RenderSubMenu(int num)
{
    vec4 r;
    // a background alpha blended rectangle
    switch(num)
    {
        default:
        case ON_SUBMENU:
        case ON_SUBMENU_2:
        // position and size in px
        r = (vec4){ (1280 - 320) /2, // pos x (center)
                     (720 - 400) /2, // pos y (center)
                      320,  400  };  // size w, h
        break;
        case ON_ITEM_PAGE:
        r = (vec4){ 466, 110,        // pos  x, y
                    600, 190+256 };  // size w, h
        break;
    }
    // rectangle: position, size
    vec2 p = r.xy,
         s = r.xy + r.zw;
    /* convert to normalized coordinates */
      r.xy = px_pos_to_normalized(&p);
      r.zw = px_pos_to_normalized(&s);
    // save current color and set another one
    vec4 curr_color = color;
    //* draw an alpha blended rect
    color = (vec4) { 0., 0., 0., .7 };
    // select shader to use
    curr_Program = glsl_Program[USE_COLOR];
    // gles render all rects
    ORBIS_RenderFillRects(&r, 1);

    // draw selection rectangle, use normalized coordinates!
    vec4 b = r;
    b.yw /= 10;
    switch(num)
    {
        default: /* skip the box */ return;

        /* compute the selection box (shrink vertically to position) */
        case ON_SUBMENU:   {
        b.yw -= r.yy
              + (b.w - b.y) /2.
              + (b.w - b.y)
              * ( 2   // start from
                + (2  // even/odd items
                   * abs(rela_pos.y %4))); // 4 choices
        } break;
        case ON_SUBMENU_2: {
        b.yw -= r.yy
              + (b.w - b.y) /2.
              + (b.w - b.y)
              * ( 1   // start from
                + (1  // all items
                   * abs(rela_pos.y %3))); // 3 choices
        } break;
        case ON_ITEM_PAGE: {
        b.yw -= r.yy
              + (b.w - b.y) /2.  // half bar height
              + (b.w - b.y)      // one bar height
              * 9;               // start from

        /* the alpha blended box under the app icon */
        // position and size in px
        vec4 r = { 200, 110, 256, 180 };
        vec2 p = r.xy,
             s = r.xy + r.zw;  // turn size into the second point
        // compute the normalized frect
          r.xy = px_pos_to_normalized(&p);
          r.zw = px_pos_to_normalized(&s);
        // gles render our rect
        ORBIS_RenderFillRects(&r, 1);

        // recompute the selection box from this rect
        b = r;
        vec4 o = (vec4){ .055f, .16f, -.055f, -.21f };
        b += o;
        //b    *= .9f;
        //b.yw -= (b.w - b.y);
              //+ (b.w - b.y) *1;

        // Download button
        GLES2_render_submenu_text(0);
        } break;
    }
    // the glowing selection box
    ORBIS_RenderDrawBox(&b);

    // texts
    GLES2_render_submenu_text(num);
}
