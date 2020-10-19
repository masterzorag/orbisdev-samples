/*
    GLES2 scene

    my implementation of an interactive menu, from scratch.
    (shaders
     beware)

    2020, masterzorag
*/

#include <stdio.h>
#include <string.h>

#if defined(__ORBIS__)
    #include <debugnet.h>
    #include <orbisFile.h>
#endif

#include <freetype-gl.h>

#include "defines.h"

#include "json.h"


// share from here resolution or other? :thinking:
vec2 resolution;

extern double u_t;
extern int    selected_icon;
       ivec4  menu_pos = (0),
              rela_pos = (0);

static page_info_t *page = NULL;

// we will put 2 page_info_t per view, one for row
#define NUM_OF_ROWS  (2)
static page_info_t *row0 = NULL;
static page_info_t *row1 = NULL;

int page_num,
    page_cur = 0,
    num_of_pages = 0; // we try to compose all possible pages


void GLES2_layout16_from_json(int n)
{
    if(row0) destroy_page(row0);
    if(row1) destroy_page(row1);

    row0 = compose_page(n   );
    row1 = compose_page(n +1);

    if(!row0)
    {
        printf("we are out\n");
    }

    refresh_atlas();
}

// no page 0, start from 1
void GLES2_scene_init( int w, int h )
{
    resolution = (vec2) { w, h };

    pixelshader_init(w, h);

    /* UI: icons and texts */
    page_info_t *p = NULL;
    int i = 1;
    while(1) // try to compose any page, count available ones
    {
        p = compose_page(i);
        if(!p) break;
//      fprintf(INFO, "page %2d @ %p\n", i, p);
        destroy_page(p), p = NULL;
        num_of_pages++, i++;
    }
    fprintf(INFO, "%d available pages!\n", num_of_pages);

    // now all needed glyph is cached in gpu atlas,
    // create the first screen we will show
    GLES2_layout16_from_json(1);

    on_GLES2_Init_icons(w, h);

    // UI: submenu
    ORBIS_RenderFillRects_init(w, h);
    GLES2_init_submenu(w, h);

    // additions
    GLES2_ani_init(w, h);
}


// don't break the main renderloop, don't swap any frame here
void GLES2_scene_render(void)
{
    // new PS as background
    pixelshader_render();
    // update the time
    on_GLES2_Update(u_t);

    page_info_t *selected_row;
    if(!(menu_pos.y %2)
    || !row1)
        selected_row = row0;
    else
        selected_row = row1;

    if(menu_pos.z != ON_ITEM_PAGE)
    {// UI: pngs
        on_GLES2_Render_icons(row0);
        on_GLES2_Render_icons(row1);
    }

    switch(menu_pos.z) // as view
    {
        case ON_TEST_ANI:
        {
            render_text();   // freetype demo-font.c, renders text just from init_

            GLES2_ani_test(NULL);
            GLES2_ani_update(u_t);
        }   break;

        case ON_MAIN_SCREEN:
        default: 
        {
            // UI: texts
            GLES2_render_page(selected_row);

            // UI: selection box
            on_GLES2_Render_box(NULL);
        }   break;

        // UI: submenus
        case ON_ITEM_PAGE:
        {
             vec4 r;
             vec2 p, s;
             p    = (vec2) { 200., 300. };  // position in px
             s    = (vec2) { 256., 256. };  // size in px
             s   += p;         // turn size into destination point!
             r.xy = px_pos_to_normalized(&p);
             r.zw = px_pos_to_normalized(&s);

             on_GLES2_Render_icon(selected_row->item[selected_icon].texture, 
                                  selected_icon, 
                                 &r);
        }    // don't break here!
        case ON_SUBMENU:  
        case ON_SUBMENU_2: 
             ORBIS_RenderSubMenu(menu_pos.z);
             break;
    }
    // ...
}

#define UP   (111)
#define DOW  (116)
#define LEF  (113)
#define RIG  (114)
#define CRO  ( 53)
#define CIR  ( 54)
#define TRI  ( 28)
#define SQU  ( 39)
#define OPT  ( 32)

/* deal with menu position / actions */
void GLES2_scene_on_pressed_button(int button)
{
    volatile ivec2 posi  = (ivec2)(0);
             ivec2 bounds = (0);

    switch(button)
    {   // take in account the movement
        case UP :  posi.y--;  break;
        case DOW:  posi.y++;  break;
        case LEF:  posi.x--;  break;
        case RIG:  posi.x++;  break;
    }
    // resulting position in selected menu:
    ivec2 result = (0);
    // handle the movement:
    switch(menu_pos.z) // on view
    {
        case ON_ITEM_PAGE:
        case ON_MAIN_SCREEN: {
             result = menu_pos.xy + posi;
             bounds = (ivec2){ NUM_OF_TEXTURES -1, num_of_pages -1 };
        }    break;
        case ON_SUBMENU:     {
             result = rela_pos.xy + posi;
             bounds = (ivec2){ 0, 4 -1 };
        }    break;
        case ON_SUBMENU_2:   {
             result = rela_pos.xy + posi;
             bounds = (ivec2){ 0, 3 -1 };
        }    break;
    }
    int res = -1;
    /* keep main bounds on available items */
    if(result.x < 0) result.x = bounds.x;
    if(result.y < 0) result.y = bounds.y, res = bounds.y; // refresh from max
    if(result.x > bounds.x) result.x = 0;
    if(result.y > bounds.y) result.y = 0, res = 1;

    //int selected_row = result.y %2;
   // printf("selected_row:%d\n", result.y %2);
    printf("%s: result( %d, %d)\n", __FUNCTION__, result.x, result.y);
#if 1
        // result is the current item
        page_info_t *selected_row;
        if(!(result.y %2)
        || !row1)
            selected_row = row0;
        else
            selected_row = row1;

        selected_icon = result.x %NUM_OF_TEXTURES;
#endif

        // apply the movement in current menu
        switch(menu_pos.z) // on view
        {
            case ON_ITEM_PAGE:
            {// refresh vbo for page, on press (ugly)
                //if(menu_pos.z == ON_ITEM_PAGE)
                GLES2_RenderPageForItem(&selected_row->item[selected_icon]);
            }
            case ON_MAIN_SCREEN:
            {
                menu_pos.xy = result;

                // refresh just if needed
                if(selected_row->page_num != result.y +1)
                {   // round to load from first row, each page!
                    int res = (result.y /2) *2 +1;
                    //printf("refresh from %d\n", res);
                    GLES2_layout16_from_json(res);
                    selected_row = row0;
                }

                // which item of which page (row) is selected?
                ivec2 selected_item = (ivec2)
                {
                    menu_pos.x %NUM_OF_TEXTURES,
                    menu_pos.y %NUM_OF_ROWS
                };
//                page_num      = (menu_pos.y / NUM_OF_TEXTURES *2) +1;
            }   break;
            case ON_SUBMENU:
            case ON_SUBMENU_2:
            {
                rela_pos.y = result.y;
                //fprintf(INFO, "MENU item:%d\n", rela_pos.y);
            }   break;            
        }

    // or action triggers:
    switch(button)
    {
        case CRO: {
                if(menu_pos.z == ON_MAIN_SCREEN){
                    menu_pos.z = ON_ITEM_PAGE;
                    GLES2_RenderPageForItem(&selected_row->item[selected_icon]);
                }
                if(menu_pos.z == ON_ITEM_PAGE)
                {
                    printf("package: %s\n", get_json_token_value(&selected_row->item[selected_icon], PACKAGE));
                }
//             get_json_token_test(&page->item[selected_icon].token[0]);
                if(menu_pos.z == ON_SUBMENU
                || menu_pos.z == ON_SUBMENU_2)
                    printf("execute %d\n", rela_pos.y);
                }  break;
        case CIR:  menu_pos.z = ON_MAIN_SCREEN;
                   break;
        case TRI:  menu_pos.z = ON_SUBMENU; rela_pos = (0); // reset on open
                   break;
        case SQU:  menu_pos.z = ON_TEST_ANI;
                   break;
        case OPT:  menu_pos.z = ON_SUBMENU_2; rela_pos = (0); // reset on open
                   break;
        default:   break; // unhandled
    }
//    printf(" %d %d %d\n", posi.x, posi.y, posi.z);
//    printf(" %d %d %d\n", menu_pos.x, menu_pos.y, menu_pos.z);
        

}
