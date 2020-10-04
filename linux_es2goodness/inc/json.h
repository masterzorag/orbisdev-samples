
// describe json tokens for each items in a page 

typedef enum names
{
    ID,
    NAME,
    DESC,
    IMAGE,
    PACKAGE,
    VERSION,
    PICPATH,
    DESC_1,
    DESC_2,
    REVIEWSTARS,
    SIZE,
    AUTHOR,
    APPTYPE,
    PV,
    MAIN_ICON_PATH,
    MAIN_MENU_PIC,
    RELEASEDATE
    // should match NUM_OF_USER_TOKENS
} names;

static const char *used_token[] =
{
    "id",
    "name",
    "desc",
    "image",
    "package",
    "version",
    "picpath",
    "desc_1",
    "desc_2",
    "ReviewStars",
    "Size",
    "Author",
    "apptype",
    "pv",
    "main_icon_path",
    "main_menu_pic",
    "releaseddate"
};

#define NUM_OF_USER_TOKENS  (sizeof(used_token) / sizeof(used_token[0]))

// single page item infos
typedef struct
{
    item_idx_t token  [NUM_OF_USER_TOKENS]; // indexed tokens from json_data
    ivec2      token_i[NUM_OF_USER_TOKENS]; // enough for all idx in ft-gl VBO
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

page_info_t *compose_page(int page_num);
void destroy_page(page_info_t *page);
void GLES2_render_page( page_info_t *page );