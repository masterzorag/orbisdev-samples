#include <stdio.h>
#include <string.h>

#if defined(__ORBIS__)
    #include <debugnet.h>
    #include <orbisFile.h>
#endif

#include <freetype-gl.h>

#include "defines.h"

#include "json.h"

// share from here resolution or other?

extern double u_t;
extern int    selected_icon;
       ivec4  menu_pos = (0);

static page_info_t * page[2];


void GLES2_scene_init( int w, int h )
{
    pixelshader_init(w, h);

    // UI: icons and texts
    page[0] = compose_page(1);

#if defined PNG_ICONS
    on_GLES2_Init_icons(w, h);
    // set viewport
//    on_GLES2_Size_icons((int)window_width, (int)window_height);
#endif

    // additions
    GLES2_ani_init  (w, h);
}


// don't break the main renderloop, don't swap any frame here
void GLES2_scene_render(void)
{
    // new PS as background
    pixelshader_render();

#if defined PNG_ICONS
    /// update the time
    on_GLES2_Update(u_t);

    // UI: pngs
    for(int i = 0; i < NUM_OF_TEXTURES; i++) on_GLES2_Render_icons(i);
#endif
    // UI: selection box
    on_GLES2_Render_box(selected_icon);

    if(menu_pos.y > -1)
    {
        // UI: texts
        GLES2_render_page(page[0]);
    }
    else
    {
        // additions
        GLES2_ani_test(NULL);
        GLES2_ani_update(u_t);
    }
}

#define UP   (111)
#define DOW  (116)
#define LEF  (113)
#define RIG  (114)
#define CRO  ( 53)
#define CIR  ( 54)
#define TRI  (1)
#define SQU  (2)

int page_num,
    page_cur;

// deal with menu position/actions
void GLES2_scene_on_pressed_button(int button)
{
    /* keep bounds like rotating */
    int x = selected_icon %NUM_OF_TEXTURES;
    if(x<0) x = NUM_OF_TEXTURES -1;
    selected_icon = x;
    printf("selected_icon %d %d\n", x, selected_icon); 

    //printf("button: %d\n", button);
    switch(button)
    {
        // move in menu
        case UP : menu_pos.y++; break;
        case DOW: menu_pos.y--; break;
        case LEF: menu_pos.x--; break;
        case RIG: menu_pos.x++; break;
        // actions
        case CRO: {
                json_get_token_test(&page[0]->item[selected_icon].token[0]);
                } break;
        case CIR: break;
        case TRI: break;
        case SQU: break;
        default:  break;
    }

    if(menu_pos.y > 0)
    {
        //item_idx_t *item = &page[0]->item[selected_icon];
    }

    page_num = menu_pos.x /NUM_OF_TEXTURES;
    printf("(%d, %d) page:%d, stride:%d\n", menu_pos.x, menu_pos.y, 
            page_num,
            menu_pos.x %NUM_OF_TEXTURES );

    // trigger a test to create next page and test cleanup
    if(page_num != page_cur)
    {
        page[1] = compose_page(1);
        //page[0] = page[1];
        //page[1]
        destroy_page(page[1]); 
    };

    page_cur = page_num;
}
