#include <stdio.h>
#include <string.h>

/*clang *.c -I$PS4SDK/include/MiniAPI -L/Archive/PS4-work/OrbisLink/samples/pc_es2template/source -lMiniAPI -L/Archive/PS4-work/OrbisLink/samples/pc_es2template/source -lm -lGL -lEGL -lX11 -D_MAPI_ -I$PS4SDK/include/freetype2 -lfreetype -png -ggdb*/

#if defined(__APPLE__)
    #include <Glut/glut.h>
#elif defined(_WIN32) || defined(_WIN64)
    #include <GLUT/glut.h>
#elif defined(__PS4__)
    #include <debugnet.h>
    #include <orbisFile.h>
#endif


#include "defines.h"

#include "ani.h"

// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;    // position (3f)
//    float s, t;       // texture  (2f)
    float r, g, b, a; // color    (4f)
} vertex_t;

extern int selected_icon; // from main.c
// ------------------------------------------------------- global variables ---
// shader and locations
static GLuint program    = 0; // default program
static GLuint shader_fx  = 0; // text_ani.[vf]
static mat4   model, view, projection;
static float  g_Time     = 0.0f;
static GLuint g_TimeSlot = 0;
// ---------------------------------------------------------------- reshape ---
static void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}

// ---------------------------------------------------------- animation ---

static fx_entry_t *ani;          // the fx info

static vertex_buffer_t *line_buffer;

// ---------------------------------------------------------------- display ---
static void render_ani( int text_num, int type_num )
{
    // we already clean in main renderloop()!

    //type_num = 1; // which fx_entry_t test

    fx_entry_t *ani = &fx_entry[type_num];
    //*         fx  = &fx_entry[0];
    //      int t_n = ani - fx;

    program         = shader_fx;
#if 1
    if(ani->fcount >= ani->life) // looping ani_state, foreach text
    {
        switch(ani->status) // setup for next fx
        {
            case ANI_CLOSED : ani->status = ANI_IN;      ani->life  =  30.; break;
            case ANI_IN     : ani->status = ANI_DEFAULT; ani->life  = 240.; break;
            case ANI_DEFAULT: ani->status = ANI_OUT;     ani->life  =  30.; break;
            case ANI_OUT    : ani->status = ANI_CLOSED;  ani->life  =  60.; break;
            /* CLOSED reached: switch text! */   //selected  +=   0 ; break;
        }
        ani->fcount = 0; // reset framecount
    }
#endif
/*
    printf("program: %d [%d] fx state: %.1f, frame: %3d/%3.f %.3f\r", 
            program, t_n,
                     ani->status /10.,
                     ani->fcount,
                     ani->life,
                     fmod(ani->status /10. + type_num /100., .02));
*/
    glUseProgram   ( program );
//    glActiveTexture( GL_TEXTURE0 );
//    glBindTexture  ( GL_TEXTURE_2D, atlas->id ); // rebind glyph atlas
    glDisable      ( GL_CULL_FACE );
    glEnable       ( GL_BLEND );
    glBlendFunc    ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    {
//        glUniform1i       ( glGetUniformLocation( program, "texture" ),    0 );
        glUniformMatrix4fv( glGetUniformLocation( program, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( program, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( program, "projection" ), 1, 0, projection.data);
        glUniform4f       ( glGetUniformLocation( program, "meta"), 
                ani->fcount,
                ani->status /10., // we use float on SL, switching fx state
                ani->life,
                type_num    /10.);
#if 1
        if(1) /* draw whole VBO (storing all added texts) */
        {
            /* draw whole VBO item arrays */
            vertex_buffer_render( line_buffer,  GL_LINES );
           //vertex_buffer_render( buffer, GL_TRIANGLES );
        }
#else
        else /* draw a range/selection of indeces (= glyph) */
        {
            vertex_buffer_render_setup( buffer, GL_TRIANGLES ); // start draw
            
            //for(int j=0; j<itemcount; j+=1)  // draw all texts
            //int j = selected %NUM;
            int j = text_num;
            {
                // draw just text[j]
                // iterate each char in text[j]
                for(int i  = item[j].off;
                        i  < item[j].off + item[j].len;
                        i += 1)
                { // glyph by one (2 triangles: 6 indices, 4 vertices)
                    vertex_buffer_render_item ( buffer, i );
                }
            }
            
            vertex_buffer_render_finish( buffer ); // end draw
        }
#endif
    }
    glDisable( GL_BLEND ); 

    ani->fcount += 1; // increase frame counter

    // we already swapframe in main renderloop()!
}


/* wrapper from main */
void GLES2_ani_test( fx_entry_t *_ani )
{
    render_ani( 1, selected_icon %MAX_ANI_TYPE );
    //for (int i = 0; i < itemcount; ++i)
    {
    /*  render_text_extended( item_entry, fx_entry );
        read as: draw this VBO using this effect! */
//        render_ani( 0, TYPE_0 );
//        render_ani( 1, TYPE_1 );
//        render_ani( 2, TYPE_2 );
//        render_ani( 3, TYPE_3 );
    }
}



// --------------------------------------------------------- custom shaders ---
static GLuint CreateProgram( void )
{
#if 1 /* we can use OrbisGl wrappers, or MiniAPI ones */
    const GLchar  *vShader = (void*) orbisFileGetFileContent( "shaders/ani.vert" );
    const GLchar  *fShader = (void*) orbisFileGetFileContent( "shaders/ani.frag" );
          GLuint programID = BuildProgram(vShader, fShader); // shader_common.c

#endif /* else,
    include and links against MiniAPI library!
    programID = miniCompileShaders(s_vertex_shader_code, s_fragment_shader_code);
#endif */
    if (!programID) { printf( "failed!\n"); }
    // feedback
    printf( "program_id=%d (0x%08x)\n", programID, programID);
    return programID;
}


// libfreetype-gl pass last composed Text_Length in pixel, we use to align text!

// ------------------------------------------------------------------- main ---
void GLES2_ani_init(int width, int height)
{
    line_buffer  = vertex_buffer_new( "vertex:3f,color:4f" );

    vec4 color = { 1., 1., 1., 1. };

#if 0 // rects
    
    vec2 pen = { .5, .5 };
for (int i = 0; i < 10; ++i)
{
    int  x0 = (int)( pen.x + i * 2 );
    int  y0 = (int)( pen.y + 20 );
    int  x1 = (int)( x0 + 64 );
    int  y1 = (int)( y0 - 64 );

    GLuint indices[6] = {0,1,2,  0,2,3}; // (two triangles)

    /* VBO is setup as: "vertex:3f, vec4 color */ 
    printf("%d %d %d %d\n", x0, y0, x1, y1);
    vertex_t vertices[4] = { { x0,y0,0,  color },
                             { x0,y1,0,  color },
                             { x1,y1,0,  color },
                             { x1,y0,0,  color } };
    vertex_buffer_push_back( line_buffer, vertices, 4, indices, 6 );
    pen.x   += 72.;
    pen.y   -= 32.;
    color.g -= i * 0.1; // to show some difference: less green
}
#else 
 vec2 pen, origin;

    origin.x = width /2 + 100;
    origin.y = height/2 - 200;
//    add_text( text_buffer, big, "g", &black, &origin );
//vec4 black = { 0,0,0,1 };
    // title
    pen.x = 50;
    pen.y = height - 50;
//    add_text( text_buffer, title, "Glyph metrics", &black, &pen );

    printf("%f %f\n", 0.1*width, origin.y);
    float r = color.r, g = color.g, b = color.b, a = color.a;

    // lines
    vertex_t vertices[] =
        {   // Baseline
            { 10,  10, 0,  r,g,b,a},
            {100, 100, 0,  r,g,b,a},

            // Top line
            { origin.x,       origin.y + 100, 0,  r,g,b,a},
            { origin.x + 100, origin.y + 100, 0,  r,g,b,a},
            // Bottom line
            { origin.x,       origin.y      , 0,  r,g,b,a},
            { origin.x + 100, origin.y      , 0,  r,g,b,a},
            // Left line at origin
            { origin.x,       origin.y + 100, 0,  r,g,b,a},
            { origin.x,       origin.y      , 0,  r,g,b,a},
            // Right line at origin
            { origin.x + 100, origin.y + 100, 0,  r,g,b,a},
            { origin.x + 100, origin.y      , 0,  r,g,b,a},

            // Left line
            {width/2 - 20/2, .3*height, 0,  r,g,b,a},
            {width/2 - 20/2, .9*height, 0,  r,g,b,a},

            // Right line
            {width/2 + 20/2, .3*height, 0,  r,g,b,a},
            {width/2 + 20/2, .9*height, 0,  r,g,b,a},

            // Width
            {width/2 - 20/2, 0.8*height, 0,  r,g,b,a},
            {width/2 + 20/2, 0.8*height, 0,  r,g,b,a},

            // Advance_x
            {width/2-20/2-16, 0.2*height, 0,  r,g,b,a},
            {width/2-20/2-16+16, 0.2*height, 0,  r,g,b,a},

            // Offset_x
            {width/2-20/2-16, 0.85*height, 0,  r,g,b,a},
            {width/2-20/2, 0.85*height, 0,  r,g,b,a},

            // Height
            {0.3*width/2, origin.y + 18 - 24, 0,  r,g,b,a},
            {0.3*width/2, origin.y + 18, 0,  r,g,b,a},

            // Offset y
            {0.8*width, origin.y + 18, 0,  r,g,b,a},
            {0.8*width, origin.y , 0,  r,g,b,a},

        };
    GLuint indices [] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,
                         13,14,15,16,17,18,19,20,21,22,23,24,25 };
    vertex_buffer_push_back( line_buffer, vertices, 26, indices, 26 );
#endif

		/* shader program is custom, so
       compile, link and use shader */
    shader_fx = CreateProgram();

    if(!shader_fx) { printf("program creation failed\n"); }

    /* init ani effect */
    ani = &fx_entry[0];
    ani->status = ANI_CLOSED,
    ani->fcount =  0;     // framecount
    ani->life   = 20.f;   // duration in frames

    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
    // attach our "custom infos"
    g_TimeSlot = glGetUniformLocation(shader_fx, "meta");

    reshape(width, height);
}

void GLES2_ani_update(double elapsedTime)
{
    g_Time += (float)(elapsedTime * 1.f);  // adjust time
    //glUseProgram(shader_fx);
//printf("%d:%.4f %d %4G %4f\n", g_TimeSlot, g_Time, ani_status, elapsedTime, elapsedTime);
    //glUniform1f(g_TimeSlot, g_Time); // notify shader about elapsed time
}

void GLES2_ani_fini( void )
{
    //texture_atlas_delete(atlas),  atlas  = NULL;
    //vertex_buffer_delete(buffer), buffer = NULL;
    if(shader_fx) glDeleteProgram(shader_fx), shader_fx = 0;
}
