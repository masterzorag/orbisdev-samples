/*
  a GLES2 playground on EGL

  2019, 2020, masterzorag
*/


#include <math.h>
#include <stdbool.h> 
#include <stdlib.h> 
#include <stdio.h>
//#include <time.h>
#include <sys/time.h>
// X11
#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>
// GLES2
#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

#include "defines.h" // the parts list


extern int selected_icon;   // from icons.c

int is_facing_left;  // from  sprite.c

double dt = 0,  // delta time
      u_t = 0;  // total time

const char vertex_src [] =
"                                        \
   attribute vec4        position;       \
   varying mediump vec2  pos;            \
   uniform vec4          offset;         \
                                         \
   void main()                           \
   {                                     \
      gl_Position = position + offset;   \
      pos = position.xy;                 \
   }                                     \
";
 
 
const char fragment_src [] =
"                                                      \
   varying mediump vec2    pos;                        \
   uniform mediump float   phase;                      \
                                                       \
   void  main()                                        \
   {                                                   \
      gl_FragColor  =  vec4( 1., 0.9, 0.7, 1.0 ) *     \
        cos( 30.*sqrt(pos.x*pos.x + 1.5*pos.y*pos.y)   \
             + atan(pos.y,pos.x) - phase );            \
   }                                                   \
";
//  some more formulas to play with...
//      cos( 20.*(pos.x*pos.x + pos.y*pos.y) - phase );
//      cos( 20.*sqrt(pos.x*pos.x + pos.y*pos.y) + atan(pos.y,pos.x) - phase );
//      cos( 30.*sqrt(pos.x*pos.x + 1.5*pos.y*pos.y - 1.8*pos.x*pos.y*pos.y)
//            + atan(pos.y,pos.x) - phase );
 
 
void
print_shader_info_log (
   GLuint  shader      // handle to the shader
)
{
   GLint  length;
 
   glGetShaderiv ( shader , GL_INFO_LOG_LENGTH , &length );
 
   if ( length ) {
      char* buffer  = malloc(length); 
      glGetShaderInfoLog ( shader , length , NULL , buffer );
     
      free(buffer);
 
      GLint success;
      glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
      if ( success != GL_TRUE )   exit ( 1 );
   }
}
 
 
GLuint
load_shader (
   const char  *shader_source,
   GLenum       type
)
{
   GLuint  shader = glCreateShader( type );
 
   glShaderSource  ( shader , 1 , &shader_source , NULL );
   glCompileShader ( shader );
 
   print_shader_info_log ( shader );
 
   return shader;
}
 
 
Display    *x_display;
Window      win;
EGLDisplay  egl_display;
EGLContext  egl_context;
EGLSurface  egl_surface;
 
GLfloat
   norm_x    =  0.0,
   norm_y    =  0.0,
   offset_x  =  0.0,
   offset_y  =  0.0,
   p1_pos_x  =  0.0,
   p1_pos_y  =  0.0;
 
GLint
   phase_loc,
   offset_loc,
   position_loc;
 
 
bool update_pos = false;
 
const float vertexArray[] = {
   0.0,  0.5,  0.0,
  -0.5,  0.0,  0.0,
   0.0, -0.5,  0.0,
   0.5,  0.0,  0.0,
   0.0,  0.5,  0.0 
};
 
int  num_frames = 0;
//static float frame = 0.0;

void flip_frame(void)
{
    eglSwapBuffers ( egl_display, egl_surface ); 
}

void render()
{
#ifdef _BASE_
    static float phase = 0;
#endif

    static int donesetup = 0; 
//static XWindowAttributes gwa;
 
   //// draw
 //move on X11 init
    if ( !donesetup ) {
        XWindowAttributes  gwa;
        XGetWindowAttributes ( x_display , win , &gwa );
        glViewport ( 0 , 0 , gwa.width , gwa.height );
        glClearColor ( 0.08 , 0.06 , 0.07 , 1.);    // background color
        donesetup = 1;
   }
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

#ifdef _BASE_
    glUniform1f ( phase_loc , phase );  // write the value of phase to the shaders phase
    phase = fmodf( phase + 0.5f , 2.f * 3.141f );    // and update the local variable
#endif
    if ( update_pos ) {  // if the position of the texture has changed due to user action
        GLfloat old_offset_x  =  offset_x;
        GLfloat old_offset_y  =  offset_y;

        offset_x  =  norm_x - p1_pos_x;
        offset_y  =  norm_y - p1_pos_y;

        p1_pos_x  =  norm_x;
        p1_pos_y  =  norm_y;

        offset_x  +=  old_offset_x;
        offset_y  +=  old_offset_y;

        update_pos = false;
        printf("update_pos: %f %f\n", p1_pos_x, p1_pos_y);
      //on_GLES2_TouchMove((float)norm_x, (float)norm_y, (float)p1_pos_x, (float)p1_pos_y);
    }

 #ifdef _BASE_
    glUniform4f ( offset_loc  ,  offset_x , offset_y , 0.0 , 0.0 );
 
    glVertexAttribPointer ( position_loc, 3, GL_FLOAT, false, 0, vertexArray );
    glEnableVertexAttribArray ( position_loc );
    glDrawArrays ( GL_TRIANGLE_STRIP, 0, 5 );

#elif defined GLSLSANDBOX
    DrawScene_1(NULL);  // shader_viewer

//    render_selection_rectangle(0);
#endif

#if defined FT_DEMO
    render_text();   // freetype demo-font.c, renders text just from init_

    es2rndr_text_ext(NULL);  // freetype text_ani.c, shared VBO, draw just indexed texts
    es2updt_text_ani(1.f);

    es2rndr_fm(NULL);  // freetype text_ani.c, shared VBO, draw just indexed texts
    es2updt_fm(1.f);

#elif defined FT_DEMO_2
    // demo-font_2.c init
    render_text();

#endif

#if defined LINE_AND_RECT
//  enable for additional pixel shader
//  es2rndr_lines_and_rect();
    ORBIS_RenderFillRects_rndr();
#endif

#if defined MY_RECT
    ORBIS_RenderFillRects_rndr();
#endif

#if defined PNG_ICONS
    /// update the time
    on_GLES2_Update(u_t);
    // render all textures VBOs but first one (the fullscreen one)
    for(int i = 0; i < NUM_OF_TEXTURES; i++) on_GLES2_Render(i); // skip background
#endif

#if defined OPENTYRIAN
    // NOT reached with opentyrian!
#endif

#if defined TETRIS
    tetris_play();

#endif

#if defined WAVESOUND
    on_GLES2_Update_wavesound(0);
    on_GLES2_Render_wavesound(0);
#endif

    // end of drawing ops, flip: get the rendered buffer to the screen
    eglSwapBuffers ( egl_display, egl_surface );
}


#if defined TETRIS
extern int keyPressed;
#endif

bool quit = false;

const float
      window_width  = 1024.0,
      window_height =  480.0;

uint8_t updateController(uint8_t *p)
{
    uint8_t ret = 0;
    /* check for user input */
    while ( XPending ( x_display ) )
    { // check for events from the x-server

       XEvent  xev;
       XNextEvent( x_display, &xev );

       if ( xev.type == MotionNotify ) {  // if mouse has moved
        //cout << "move to: << xev.xmotion.x << "," << xev.xmotion.y << endl;
          GLfloat window_y  = (window_height - xev.xmotion.y) - window_height / 2.0;
          norm_y            =  window_y / (window_height / 2.0);
          GLfloat window_x  =  xev.xmotion.x - window_width / 2.0;
          norm_x            =  window_x / (window_width / 2.0);
          update_pos = true;

         /* printf("%d, %d\n", xev.xmotion.x, xev.xmotion.y); */
       }

       if ( xev.type == KeyPress ) 
       {    printf("keycode %d\n", xev.xkey.keycode);
#if defined OPENTYRIAN
            if (xev.xkey.keycode == 113)
            { printf("Left pressed\n");   p[80] = 1; p[79] = 0; return 80; }
            else
            if (xev.xkey.keycode == 114)
            { printf("Right pressed\n");  p[80] = 0; p[79] = 1; return 79; }

            if (xev.xkey.keycode == 111)
            { printf("Up pressed\n");     p[81] = 0; p[82] = 1; return 82; }
            else
            if (xev.xkey.keycode == 116)
            { printf("Down pressed\n");   p[81] = 1; p[82] = 0; return 81; }

            if (xev.xkey.keycode == 54)
            { printf("Circle pressed\n"); p[40] = 1;  return 40; }

            if (xev.xkey.keycode == 53)
            { printf("Cross pressed\n");  p[44] = 1;  return 44; }

            if (xev.xkey.keycode == 37)
            { printf("L1 pressed\n");     p[224] = 1; return 224; }

            if (xev.xkey.keycode == 64)
            { printf("R1 pressed\n");     p[226] = 1; return 226; }
#else

            switch(xev.xkey.keycode)
            {
                case 113: printf("Left pressed\n");
                        p1_pos_x -= 0.04, selected_icon--; is_facing_left = 1;
#if defined TETRIS
                keyPressed = 113;
#endif
                    break;
                case 114: printf("Right pressed\n");
                        p1_pos_x += 0.04, selected_icon++; is_facing_left = 0;
#if defined TETRIS
                keyPressed = 114;
#endif
                    break;
              case 111: printf("Up pressed\n");
                        p1_pos_y += 0.04;
#if defined TETRIS
              keyPressed = 111;
#endif
      // just on keypress
//                update_selection_rectangle(0);
                        break;
              case 116: printf("Down pressed\n");
                        p1_pos_y -= 0.04;
#if defined TETRIS
              keyPressed = 116;
#endif
                        // just on keypress
//                update_selection_rectangle(0);
                    break;

                case 39: printf("Square pressed\n");
               /* dr_mp3_Loop("main.mp3"); */  
                    break;
                case 54: printf("Circle pressed\n");          
                    break;
                case 53: printf("Cross pressed\n");
                    break;
                case 28: printf("Triangle pressed\n"); 
                    ls_dir("./");
                    //get_item_entries("./");
                    break;
                case 37: /* L-CTRL */
                    printf("L1 pressed\n");   
                    break;
                case 64: /* L_ALT */
                    printf("R1 pressed\n");
                    break;
                case 24: /* q */ quit = true; break;
                default:  break;
                  
            }
#endif
            printf("selected_icon %d\n", selected_icon); 
        }
    }
    return ret;
}
 
////////////////////////////////////////////////////////////////////////////////////////////
char *shader_file_name;
 
int main(int argc, char **argv)
{
    shader_file_name = argv[1];

    ///////  the X11 part  //////////////////////////////////////////////////////////////////
    // in the first part the program opens a connection to the X11 window manager

    // open the standard display (the primary screen)
    x_display = XOpenDisplay ( NULL );
    if ( x_display == NULL ) { printf("cannot connect to X server\n"); return 1; }

    // chensq Window root  =  DefaultRootWindow( x_display );   // get the root window (usually the whole screen)
    Window root = RootWindow(x_display, DefaultScreen(x_display));
    XSetWindowAttributes  swa;
    swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;
 
    win = XCreateWindow (   // create a window with the provided parameters
              x_display, root,
              0, 0, window_width, window_height, 0,
              CopyFromParent, InputOutput,
              CopyFromParent, CWEventMask,
              &swa );
    if(!win){ printf("XCreateWindow error\n"); return 1; }

    XSetWindowAttributes  xattr;
    Atom  atom = None;
 
    xattr.override_redirect = False;
    XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );
    atom = XInternAtom ( x_display, "_NET_WM_STATE_FULLSCREEN", True );
    if(atom==None) {
        XSizeHints sizehints;
        sizehints.min_width  = window_width;
        sizehints.min_height = window_height;
        sizehints.max_width  = window_width;
        sizehints.max_height = window_height;
        sizehints.flags      = PMaxSize | PMinSize;
        XSetWMProperties(x_display, win, NULL, NULL,
                         NULL, 0, &sizehints, NULL, NULL);
    }
    XMapWindow ( x_display , win );

    ///////  the egl part  //////////////////////////////////////////////////////////////////
    //  egl provides an interface to connect the graphics related functionality of openGL ES
    //  with the windowing interface and functionality of the native operation system (X11
    //  in our case.

    egl_display = eglGetDisplay( (EGLNativeDisplayType) x_display );
    //printf("111111111\n");
    if ( egl_display == EGL_NO_DISPLAY ) { printf ("Got no EGL display.\n"); return 1; }
 
    if ( !eglInitialize( egl_display, NULL, NULL ) ) { printf ("Unable to initialize EGL\n"); return 1; }
 
    EGLint attr[] = {       // some attributes to set up our egl-interface
        EGL_BUFFER_SIZE, 16,
        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
 
    EGLConfig ecfg;
    EGLint    num_config;
    if ( !eglChooseConfig( egl_display, attr, &ecfg, 1, &num_config ) )
        { printf ("Failed to choose config (eglError: \n"); return 1; }
 
    if ( num_config != 1 )
        { printf("Didn't get exactly one config, but %d\n", num_config); return 1; }
 
    egl_surface = eglCreateWindowSurface ( egl_display, ecfg, win, NULL );
    if ( egl_surface == EGL_NO_SURFACE )
        { printf("Unable to create EGL surface\n"); return 1; }
 
    //// egl-contexts collect all state descriptions needed required for operation
    EGLint ctxattr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    egl_context = eglCreateContext ( egl_display, ecfg, EGL_NO_CONTEXT, ctxattr );
    if ( egl_context == EGL_NO_CONTEXT )
        { printf("Unable to create EGL context\n"); return 1; }
 
    //// associate the egl-context with the egl-surface
    eglMakeCurrent( egl_display, egl_surface, egl_surface, egl_context );
 
  //const char *gl_exts = (char *) glGetString(GL_EXTENSIONS);
    printf("OpenGL ES 2.x information:\n");
    printf("  version: \"%s\"\n", glGetString(GL_VERSION));
    printf("  shading language version: \"%s\"\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("  vendor: \"%s\"\n", glGetString(GL_VENDOR));
    printf("  renderer: \"%s\"\n", glGetString(GL_RENDERER));
  //printf("  extensions: \"%s\"\n", gl_exts);
    printf("===================================\n");
    
 
    ///////  the openGL part  ///////////////////////////////////////////////////////////////
    int ret = 0;
   
//goto out;
#ifdef _BASE_
    GLuint vertexShader   = load_shader ( vertex_src , GL_VERTEX_SHADER  );     // load vertex shader
    GLuint fragmentShader = load_shader ( fragment_src , GL_FRAGMENT_SHADER );  // load fragment shader

    GLuint shaderProgram  = glCreateProgram ();                // create program object
    glAttachShader( shaderProgram, vertexShader   );           // and attach both...
    glAttachShader( shaderProgram, fragmentShader );           // ... shaders to it
 
    glLinkProgram ( shaderProgram );    // link the program
    glUseProgram  ( shaderProgram );    // and select it for usage
 
    //// now get the locations (kind of handle) of the shaders variables
    position_loc = glGetAttribLocation  ( shaderProgram , "position" );
    phase_loc    = glGetUniformLocation ( shaderProgram , "phase"    );
    offset_loc   = glGetUniformLocation ( shaderProgram , "offset"   );
    if ( position_loc < 0 || phase_loc < 0 || offset_loc < 0 )
        { printf("Unable to get uniform location\n"); return 1; }

#elif defined FT_DEMO
    // demo-font.c init
    es2init_text((int)window_width, (int)window_height);

  #if defined _SOUND_
    on_GLES2_Init_sound((int)window_width, (int)window_height);
    printf("on_GLES2_Init\n");
    // set viewport
    on_GLES2_Size_sound((int)window_width, (int)window_height);

    es2init_browser((int)window_width, (int)window_height); // browser
    //reshape2((int)window_width, (int)window_height);
  #endif
   es2init_text_ani((int)window_width, (int)window_height); // text fx

es2init_fm((int)window_width, (int)window_height); // text fx

#elif defined FT_DEMO_2
    // demo-font.c init
    es2init_text((int)window_width, (int)window_height);

#elif defined LINE_AND_RECT
    // demo-font.c init
    es2init_lines_and_rect((int)window_width, (int)window_height);
    // like SDL
    ORBIS_RenderFillRects_init((int)window_width, (int)window_height);
#endif

#if defined MY_RECT
    ORBIS_RenderFillRects_init((int)window_width, (int)window_height);
#endif

#if defined PNG_ICONS
    on_GLES2_Init_icons((int)window_width, (int)window_height);
//    printf("on_GLES2_Init\n");
    // set viewport
//    on_GLES2_Size_icons((int)window_width, (int)window_height);
#endif

#if defined TETRIS
    tetris_init((int)window_width, (int)window_height);
#endif

    // simulate sceAudioOut via libao
#if defined HAVE_LIBAO
    ret = orbisAudioInit();                         //printf("ret: %d\n", ret); //1
    ret = orbisAudioInitChannel(0, 1024, 48000, 1); //printf("ret: %d\n", ret); //0
    // start audio flow!
    orbisAudioResume(0);

    #if defined WAVESOUND
    on_GLES2_Init_wavesound((int)window_width, (int)window_height);
    #endif

    #if defined DR_MP3
    /// dr_mp3, from nfs export with audio callback
    if ( dr_mp3_Load("main2.mp3") ) dr_mp3_Play();
    #endif

#endif

    // opentyrian have sounds, so start after libao
#if defined OPENTYRIAN
    /// build GLSL shader
    GLES2_Init_video((int)window_width, (int)window_height);

    char   cmd[256] = "./egl_demo_opentyrian -j -t /home/user/Downloads/tyrian/opentyrian/tyrian21";
    char **my_argv  = build_cmd(&cmd[0]);
    _main(4, my_argv);
    // not reached: opentyrian will keep control!
#endif

#if defined GLSLSANDBOX
  //InitScene_1(int i, int w, int h, const char *pShader, 0);
  //InitScene_1(0, (int)window_width, (int)window_height, NULL, 0);
    if (shader_file_name)
        InitScene_1(2, (int)window_width, (int)window_height, shader_file_name, 0);
    else
        InitScene_1(2, (int)window_width, (int)window_height, "/hostapp/assets/quads.shader", 0);
#endif
//  init_selection_rectangle(window_width, window_height);

    // this is needed for time measuring  -->  frames per second
//    struct timezone tz;
    struct timeval  t1, t2,
                    t3, t4; // count each passed second
    gettimeofday ( &t1 , NULL );

    // reset chrono to the " run*time* "
    t4 = t3 = t1;

    while ( !quit ) { /// the main render loop
 
        /* check for user input */
        updateController(NULL);
 
        render(); // now we finally put something on the screen
 
        // timing, for fps
        if ( ++num_frames % 100 == 0 ) {
            gettimeofday( &t2, NULL );
            dt = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6;
            printf("fps: %f, %.4f seconds\n", (float)(num_frames / dt), dt);
            num_frames = 0;
            t1 = t2;
            //t4 = t2;
        }

#if defined GLSLSANDBOX
        UpdateScene_1(2, 1.);
#endif

        // timing, again
        //if( num_frames > 20 )
        {
            gettimeofday( &t3, NULL );
            // calculate delta time
            dt = t3.tv_sec - t4.tv_sec + (t3.tv_usec - t4.tv_usec) * 1e-6;
            // each passed second...
            //if (dt > 2.f)
            {
                //printf("dt = %0.4f\n", dt);
                t4 = t3;
                // sample 48000 texture
                //on_GLES2_Update_sound(dt);
            }
            // update total time
            u_t += dt;
            //printf("u_t = %0.4f\n", u_t);
        }
#if defined _MAPI_ 
      //UpdateScene(dt);
#endif 
      //usleep( 10000 );
   }

//out:
    /* destructors follows: */

#if defined _DEMO_  
    on_GLES2_Final();

#elif defined FT_DEMO
    // FT ones, sorted
    #if defined _SOUND_  
    es2fini_browser();
    #endif
    es2fini_text_ani();
    es2fini_fm();
    es2sample_end(); // demo-font.c as last one
#endif

#if defined LINE_AND_RECT
    es2fini_lines_and_rect();
    ORBIS_RenderFillRects_fini();
#endif

#if defined HAVE_LIBAO
    orbisAudioStop();
    orbisAudioFinish();
#endif

    /// cleaning up EGL, X11
    eglDestroyContext ( egl_display, egl_context );
    eglDestroySurface ( egl_display, egl_surface );
    eglTerminate      ( egl_display );
    XDestroyWindow    ( x_display, win );
    XCloseDisplay     ( x_display );

    return 0;
}
