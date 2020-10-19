/*
    GLES2 menu
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <freetype-gl.h>  // links against libfreetype-gl

#include "defines.h"

extern vec4 color; // GLES_rects.c

extern GLuint shader;
extern texture_atlas_t *atlas;
extern mat4             model, view, projection;
extern vec2 resolution;
// freetype-gl pass last composed Text_Length in pixel, we use to align text!
extern float tl;

// to json-simple.c
vertex_buffer_t *text_buffer[4];
texture_font_t  *sub_font = NULL;
// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;    // position (3f)
    float s, t;       // texture  (2f)
    float r, g, b, a; // color    (4f)
} vertex_t;


// the pngs we turn into VBOs + textures
const char *settings_menu[] =
{
    "Settings",
    "Loaded from app",
    "Temp path",
    "/user/app",
    "CDNURL",
    "http...",
    "Background Path",
    "null...",
    "Save settings",
    "IP: blablabla"
};

const char *selection_menu[] =
{
    "Selection Menu",
    "Back to the Store(?)",
    "Fan control",
    "Page Selector"
};

#define lines_num (sizeof(settings_menu) / sizeof(settings_menu[0]))


void GLES2_render_submenu_text( int num )
{
    if(text_buffer[num] == NULL) return;

    glUseProgram( shader );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture  ( GL_TEXTURE_2D, atlas->id ); // rebind glyph atlas
    glEnable       ( GL_BLEND );
    glBlendFunc    ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDisable      ( GL_CULL_FACE );
    {
        glUniform1i       ( glGetUniformLocation( shader, "texture" ),    0 );
        glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ), 1, 0, projection.data);
        /* draw whole VBO item array */
        vertex_buffer_render( text_buffer[num], GL_TRIANGLES );
    }   
    glUseProgram( 0 );
}

// init VBOs for menu texts
void GLES2_init_submenu( void )
{
    // each vbo acts as temporary placeholder, per view
    // commons
    text_buffer[0] = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    // ON_SUBMENU
    text_buffer[1] = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    // ON_SUBMENU_2
    text_buffer[2] = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    // ON_ITEM_PAGE
    text_buffer[3] = NULL;//vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );

    sub_font = texture_font_new_from_memory(atlas, 26, _hostapp_fonts_zrnic_rg_ttf,
                                                       _hostapp_fonts_zrnic_rg_ttf_len);
    vec2 pen = (0); // init pen: 0,0 is lower left
    vec4 col = (vec4)( 1. );
    pen.y    = (resolution.y - 400) /2 + 400 + /*displace*/ 10;

    // Settings menu
    for(int i=0; i < lines_num; i++)
    {
        texture_font_load_glyphs( sub_font, settings_menu[i] );   
        pen.x  = (resolution.x - tl) /2;  // use Text_Length to center pen.x
        pen.y -= 400 /lines_num;//sub_font->height + 1;            // reset pen, 1 line down

        switch(i) // line
        {   // red header
            case 0:  col = (vec4){ 1., 0., 0., 1. }; break;
            // grey
            case 1:
            case 3:
            case 5:
            case 7:
            case 9:  col = (vec4){ .7, .7, .8, 1. }; break;
            default: col = (vec4)( 1. );             break;
        }
        add_text( text_buffer[1], sub_font, settings_menu[i], &col, &pen );  // set vertexes
    }
    
    // Selection menu
    pen.y = (resolution.y - 400) /2 + 400 + /*displace*/ 10;
    for(int i=0; i < sizeof(selection_menu) / sizeof(selection_menu[0]); i++)
    {
        texture_font_load_glyphs( sub_font, selection_menu[i] );   
        pen.x  = (resolution.x - tl) /2;  // use Text_Length to center pen.x
        pen.y -= 40;//sub_font->height + 1;         // reset pen, 1 line down

        switch(i) // line
        {   // red header
            case 0:  col = (vec4){ 1., 0., 0., 1. }; break;
            default: col = (vec4)( 1. );             break;
        }
        add_text( text_buffer[2], sub_font, selection_menu[i], &col, &pen );  // set vertexes
    }
    
    // Download button
    pen.y  = (resolution.y - 400) /2 + /*displace*/ 10;
    //texture_font_load_glyphs( sub_font, "Download fpkg" ); 
    pen.x  = (resolution.x - tl) /2;
    add_text( text_buffer[0], sub_font, "Download fpkg", &col, &pen );  // set vertexes
}

#if 0
void GLES2_RenderPageForItem(page_item_t *item)
{
    for(int i = 0; i < NUM_OF_USER_TOKENS; i++)
    {
        static char tmp[256];
        // get the indexed token value
        snprintf(&tmp[0], item->token[i].len + 1, "%s", item->token[i].off);

        texture_font_load_glyphs( font, &tmp[0] );        // set textures
        /* append to VBO */
        vec4 white = (vec4) ( 1.f ),
             color = (vec4) { .7, .7, .8, 1. };
        // same position as our rects
        pen = (vec2){ 100. + i * (100. + 2. /*border*/),
                      200. - i * 18. },
        add_text( text_buffer[2], font, &tmp[0], &white, &pen);

}
#endif
