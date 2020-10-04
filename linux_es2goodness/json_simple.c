#include "jsmn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"

// the png we apply glowing effect by switching shader
extern int    selected_icon;
extern GLuint shader;
extern mat4   model, view, projection;

extern texture_atlas_t *atlas;
//extern vertex_buffer_t *buffer;
/*
 * A small example of jsmn parsing when JSON structure is known and number of
 * tokens is predictable.
 */

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}


/* reuse this type to index texts around!
typedef struct
{
    char *off;
    int   len;
} item_idx_t; */
#include "json.h"


/*
typedef struct
{
    item_idx_t token  [NUM_OF_USER_TOKENS]; // indexed tokens from json_data
    ivec2      token_i[NUM_OF_USER_TOKENS]; // all idx in ft-gl VBO
//  ivec2      token_t[NUM_OF_USER_TOKENS]; // max num of text we will index and print
    int        num_of_texts;
    GLuint     texture;

    int icon;
    //ivec2 *token[sizeof(used_token) / sizeof(used_token[0])];
} page_item_t;


// the single page infos
typedef struct
{
    // each page will hold its json_data
    char *json_data;
    // and its ft-gl vertex buffer
    vertex_buffer_t *vbo;
    // max NUM_OF_TEXTURES per page, max 8 infos
    int  num_of_items;
    // all indexed tokens, per item...
    page_item_t  item[NUM_OF_TEXTURES];
    
    // unused yet
    ivec2 rela_menu_pos;// = (0);
} page_info_t;
*/


unsigned char *http_fetch_from(const char *url);

// index all used_tokens
static int json_index_used_tokens(page_info_t *page)
{
    int r, i, c = 0, idx = 0;
    jsmn_parser p;
    jsmntok_t t[512]; /* We expect no more than this tokens */

    char *json_data = page->json_data;

    jsmn_init(&p);
    r = jsmn_parse(&p, json_data, strlen(json_data), t,
                             sizeof(t) / sizeof(t[0]));
    printf("%d\n", r);
    if (r < 0) { printf("Failed to parse JSON: %d\n", r); return -1; }

    item_idx_t *token = NULL;
    for(i = 1; i < r; i++)
    {
        token = &page->item[idx].token[0];
        for(int j = 0; j < NUM_OF_USER_TOKENS; j++)
        {
         //   printf("! %d [%s]: %p, %d\n", j, used_token[j], page->tokens[j].off, page->tokens[j].len);
            if (jsoneq(json_data, &t[i], used_token[j]) == 0)
            {
                /* We may use strndup() to fetch string value */
#if 0            
                printf("- %d) %d %d [%s]: %.*s\n", c, i, j,
                    used_token[j],
                              t[i + 1].end - t[i + 1].start,
                                 json_data + t[i + 1].start);
#endif
//                json_info[c].id = strndup(json_data + t[i + 1].start,
//                                       t[i + 1].end - t[i + 1].start);
                token[j].off =    json_data + t[i + 1].start;
                token[j].len = t[i + 1].end - t[i + 1].start;
                i++; c++;
//                getchar();
                //break;
                if(j==16) idx++; // ugly but increase
            }
        }
      //printf("%d - %d [%s]: %s, %d\n", i, j, used_token[j], page->tokens[j].off, page->tokens[j].len);
    }
//    if(c == NUM_OF_USER_TOKENS * 8) printf("ok");
    return c;
}


// freetype-gl pass last composed Text_Length in pixel, we use to align text!
extern float tl;

static page_info_t *page = NULL;

void destroy_page(page_info_t *page)
{
    for(int i = 0; i < NUM_OF_TEXTURES; i++)
    {
        if(page->item[i].texture) glDeleteTextures(1, &page->item[i].texture);
    }
    // each page will hold its json_data
    free(page->json_data);
    // and its ft-gl vertex buffer
    vertex_buffer_delete(page->vbo), page->vbo = NULL;

    if(page) free(page), page = NULL;
}

// this draws the page layout:
// texts from indexed json token name, value
page_info_t *compose_page(int page_num)
{
    // do it just once for now, one page
    //if(page) return page;
    page_info_t *page = calloc(1, sizeof(page_info_t));

    char json_file[128];
    sprintf(&json_file[0], "homebrew-page%d.json", page_num);

    page->json_data = (void*)orbisFileGetFileContent(&json_file[0]);

    if(!page->json_data) { free(page); return NULL; }

    page->vbo   = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );

    int valid_t = json_index_used_tokens(page);
    // json describe max 8 items
    int ret     = valid_t /NUM_OF_USER_TOKENS;

    // if < 8 realloc page
    printf("%d\n", ret);

    texture_font_t
    *stock_font = texture_font_new_from_memory(atlas, 17, _hostapp_fonts_zrnic_rg_ttf, 
                                                          _hostapp_fonts_zrnic_rg_ttf_len),
    *title_font = texture_font_new_from_memory(atlas, 30, _hostapp_fonts_zrnic_rg_ttf, 
                                                          _hostapp_fonts_zrnic_rg_ttf_len),
    *curr_font  = NULL;
    // now prepare all the texts textures, use indexed json_data
    int count;
    for(int i = 0; i < ret; i++)
    {//        printf("== entry %d ==\n", i);
        item_idx_t *token = &page->item[i].token  [0];
        ivec2      *t_idx = &page->item[i].token_i[0];

        count = 0;
        for(int j = 0; j < NUM_OF_USER_TOKENS; j++)
        {
            switch(j) // print just the following:
            {
                case NAME:        curr_font = title_font;
                    break;
                case DESC: 
                case VERSION: 
                case REVIEWSTARS:
                case SIZE:
                case AUTHOR:
                case APPTYPE:
                case PV:
                case RELEASEDATE: curr_font = stock_font;
                    break;
                default: continue;
            }
//            printf("%d, %d: [%p]: %d\n", i, j, token[j].off, token[j].len);
            static char tmp[256];
            // get the indexed token value
            snprintf(&tmp[0], token[j].len + 1, "%s", token[j].off);
            //printf("%s\n", tmp);
            texture_font_load_glyphs( curr_font, &tmp[0] );        // set textures
            /* append to VBO */
            vec4 white = (vec4) ( 1.f ),
                 color = (vec4) { .7, .7, .8, 1. };

            /*  start indexing */
            t_idx[count].x = vector_size( page->vbo->items );
            // same position as our rects
            vec2 pen = { 100. + i * (100. + 2. /*border*/),
                         200. - count * 18. },
            // save as origin: pen will move!
            orig   = pen;
            // add a little offset from origin
            pen.x += 8.;

            add_text( page->vbo, curr_font, &tmp[0], &white, &pen);

            // position and index the token

            // switch back font
            curr_font = stock_font;
            // print token name
            sprintf(&tmp[0], "%s", used_token[j]);
            // set textures
            texture_font_load_glyphs( curr_font, &tmp[0] );
            // seek pen back to origin
            pen    = orig;
            // right align, related to origin
            pen.x -= tl;

            add_text( page->vbo, curr_font, &tmp[0], &color, &pen );

            /* end indexing */
            t_idx[count].y = vector_size( page->vbo->items )
                           - t_idx[count].x;
            // advance
            count++;
        }
        page->item[i].num_of_texts = count;
    }
    texture_font_delete( stock_font );
    texture_font_delete( title_font );

    /* discard old texture, we eventually added glyphs! */
    if(atlas->id) glDeleteTextures(1, &atlas->id), atlas->id = 0;

    /* re-create texture and upload atlas into gpu memory */
    glGenTextures  ( 1, &atlas->id );
    glBindTexture  ( GL_TEXTURE_2D, atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D   ( GL_TEXTURE_2D, 0, GL_ALPHA, atlas->width, atlas->height,
                                    0, GL_ALPHA, GL_UNSIGNED_BYTE, atlas->data );
    // don't leak buffer!
    //if(page->json_data) free (page->json_data);
    return page;
}

// review this
char *get_json_token_value(item_idx_t *item, int name)
{
    //page_info_t *p = page;
    static char tmp[256];
    // get the indexed token value
    item_idx_t *token = &item[name];

    snprintf(&tmp[0], token->len + 1, "%s", token->off);
    //printf("%s\n", tmp);
    return (char *)&tmp[0];
}

// wrapper from outside
void json_get_token_test(item_idx_t *item)
{
    printf("%s\n", get_json_token_value(item, IMAGE));

    printf("package: %s\n", get_json_token_value(item, PACKAGE));
}

// ---------------------------------------------------------------- display ---
void GLES2_render_page( page_info_t *page )
{
    // do nothing if a page not exists yet...
    if(!page) return;

    // we already clean in main renderloop()!

    // we have indexed texts: address the index pointer
    ivec2 *t_idx = &page->item[selected_icon].token_i[0];

    glUseProgram   ( shader );
    // setup state
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture  ( GL_TEXTURE_2D, atlas->id ); // rebind glyph atlas
    glDisable      ( GL_CULL_FACE );
    glEnable       ( GL_BLEND );
    glBlendFunc    ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    {
        glUniform1i       ( glGetUniformLocation( shader, "texture" ),    0 );
        glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),      1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),       1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ), 1, 0, projection.data);

        if(0) /* draw whole VBO (storing all added texts) */
        {
            vertex_buffer_render( page->vbo, GL_TRIANGLES ); // all vbo
        }
        else /* draw a range/selection of indexes (= glyph) */
        {
            vertex_buffer_render_setup( page->vbo, GL_TRIANGLES ); // start draw
            //int num_of_texts = NUM_OF_USER_TOKENS;
            for(int j=0; j < page->item[selected_icon].num_of_texts; j++)  // draw all texts
            {
              //printf("%d, %d %d (%d %d)\n", page_num, selected_icon, num_of_texts, t_idx[j].x, t_idx[j].y);
                // draw just text[j]
                // iterate each char in text[j]
                for(int i = t_idx[j].x;
                        i < t_idx[j].x + t_idx[j].y;
                        i++ )
                { // glyph by one (2 triangles: 6 indices, 4 vertices)
                    vertex_buffer_render_item ( page->vbo, i );
                }
            }
            vertex_buffer_render_finish( page->vbo ); // end draw
        }
    }
    glDisable( GL_BLEND );  // Reset state back

    // we already swapframe in main renderloop()!
}


/* useless follows below: */

// deprecated stuff follows:
typedef struct
{
    char *id,
       *name,
       *desc;
} json_info_t;
json_info_t *json_info = NULL;

/// basic, useless
int get_json_tokens(const char *json_file)
{
    char *json_data = (void*)orbisFileGetFileContent(json_file);

    if(!json_data) { printf("Can't reserve memory!"); }

    int r, i, c = 0;
    jsmn_parser p;
    jsmntok_t t[512]; /* We expect no more than this tokens */

    jsmn_init(&p);
    r = jsmn_parse(&p, json_data, strlen(json_data), t,
                             sizeof(t) / sizeof(t[0]));
    printf("%d\n", r);
    if (r < 0) { printf("Failed to parse JSON: %d\n", r); return -1; }

    for (i = 1; i < r; i++)
    {
        for (int j = 0; j < r; j++)
        {

        }

        if (jsoneq(json_data, &t[i], "id") == 0)
        {
            /* We may use strndup() to fetch string value */
//            printf("- %d id: %.*s\n", i, t[i + 1].end - t[i + 1].start,
//                                            json_data + t[i + 1].start);
            json_info[c].id = strndup(json_data + t[i + 1].start,
                                   t[i + 1].end - t[i + 1].start);
            i++;
        }
        else
        if (jsoneq(json_data, &t[i], "name") == 0)
        {
            /* We may use strndup() to fetch string value */
//            printf("- %d name: %.*s\n", i, t[i + 1].end - t[i + 1].start,
//                                              json_data + t[i + 1].start);
            json_info[c].name = strndup(json_data + t[i + 1].start,
                                     t[i + 1].end - t[i + 1].start);
            i++;
        }
        else
        if (jsoneq(json_data, &t[i], "desc") == 0)
        {
            /* We may use strndup() to fetch string value */
//            printf("- %d desc: %.*s\n", i, t[i + 1].end - t[i + 1].start,
//                                              json_data + t[i + 1].start);
            json_info[c++].desc = strndup(json_data + t[i + 1].start,
                                       t[i + 1].end - t[i + 1].start);
            i++;
        }
        else
        {
            //printf("Unexpected key: %.*s\n", t[i].end - t[i].start, json_data + t[i].start);
        }
    }

    if(json_data) free (json_data);

    return c;
}


void free_json_tokens(void)
{
    if( ! json_info ) return;

    for(int i=0; i<NUM_OF_TEXTURES; i++)
    {
        if(json_info[i].id  ) free(json_info[i].id  );
        if(json_info[i].name) free(json_info[i].name);
        if(json_info[i].desc) free(json_info[i].desc);
    }
    if(json_info) free(json_info), json_info = NULL;
}

void json_test(void)
{
    json_info = calloc(NUM_OF_TEXTURES, sizeof(json_info_t));

    // parse the entire page once and malloc all the info
    get_json_tokens("homebrew-page1.json");

    for(int i=0; i<NUM_OF_TEXTURES; i++)
    {
        printf("%d:\n%s\n%s\n%s\n", i,
           json_info[i].id,
           json_info[i].name,
           json_info[i].desc);      
    }
    // safe cleanup
    free_json_tokens();
}

//char *get_json_token(const char *token)
int _main(int argc, char **argv)
{
  //replace json_data
  char *json_data = (void*)orbisFileGetFileContent(argv[1]);
  if (!json_data) { printf("Can't reserve memory!"); }
  printf("%zub, %lu\n", _orbisFile_lastopenFile_size, strlen(json_data));

  int i;
  int r;
  jsmn_parser p;
  jsmntok_t t[512]; /* We expect no more than this tokens */

  jsmn_init(&p);
  r = jsmn_parse(&p, json_data, strlen(json_data), t,
                 sizeof(t) / sizeof(t[0]));
  printf("%d\n", r);
  if (r < 0) {
    printf("Failed to parse JSON: %d\n", r);
    return 1;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || t[0].type != JSMN_OBJECT) {
    printf("Object expected\n"); return 1;
  }

  /* Loop over all keys of the root object */
  for (i = 1; i < r; i++)
  {
      if (jsoneq(json_data, &t[i], "id") == 0)
      {
        /* We may use strndup() to fetch string value */
        printf("- id: %.*s\n", t[i + 1].end - t[i + 1].start,
               json_data + t[i + 1].start);
        i++;
      } else
      if (jsoneq(json_data, &t[i], "name") == 0)
      {
        /* We may additionally check if the value is either "true" or "false" */
        printf("- name: %.*s\n", t[i + 1].end - t[i + 1].start,
               json_data + t[i + 1].start);
        i++;
      } else
      if (jsoneq(json_data, &t[i], "desc") == 0)
      {
        /* We may additionally check if the value is either "true" or "false" */
        printf("- desc: %.*s\n", t[i + 1].end - t[i + 1].start,
               json_data + t[i + 1].start);
        i++;
      } else
      if (jsoneq(json_data, &t[i], "image") == 0)
      {
        /* http_fetch(url)
        /user/app/NPXS39041/storedata/bender-icon0.png */
        printf("- image: %.*s\n", 
              t[i + 1].end - t[i + 1].start,
                 json_data + t[i + 1].start);
        i++;
      } else
      if (jsoneq(json_data, &t[i], "picpath") == 0)
      {
        /* load  textures from 
        /user/app/NPXS39041/storedata/bender-icon0.png */
        printf("- picpath: %.*s\n", t[i + 1].end - t[i + 1].start,
               json_data + t[i + 1].start);
        i++;
      } else
      if (jsoneq(json_data, &t[i], "groups") == 0)
      {
        int j;
        printf("- Groups:\n");
        if (t[i + 1].type != JSMN_ARRAY) {
          continue; /* We expect groups to be an array of strings */
        }
        for (j = 0; j < t[i + 1].size; j++) {
          jsmntok_t *g = &t[i + j + 2];
          printf("  * %.*s\n", g->end - g->start, json_data + g->start);
        }
        i += t[i + 1].size + 1;
      } else {
        //printf("Unexpected key: %.*s\n", t[i].end - t[i].start, json_data + t[i].start);
      }
  }
  if(json_data) free (json_data);

  return EXIT_SUCCESS;
}
