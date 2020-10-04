/*
  implement some way to setup different kind of
  animations to use in print text with FreeType

  scope: reusing and sharing resources from the
  already available freetype-gl library

  2020, masterzorag
*/
//#include <freetype-gl.h>


// -------------------------------------------------------------- effects ---


/* each fx have those states */
enum ani_states
{
    ANI_CLOSED,
    ANI_IN,
    ANI_DEFAULT,
    ANI_OUT
};

enum ani_type_num
{
    TYPE_0,
    TYPE_1,
    TYPE_2,
    TYPE_3,
    MAX_ANI_TYPE
};

/* hold the current state values */
typedef struct
{
// GLuint program;
    int   status, // current ani_status
          fcount; // current framecount

    float life;   // total duration in frames
} fx_entry_t;


static fx_entry_t fx_entry[MAX_ANI_TYPE];
