/*
    $ clang double_pointer_test.c -o double_pointer_test -Wextra

    $ ./double_pointer_test a b c

    number of passed argv: 4
    argv[0] = './double_pointer_test'
    argv[1] = 'a'
    argv[2] = 'b'
    argv[3] = 'c'
    ...
    Segmentation fault 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if defined (__ORBIS__)

#include <ps4sdk.h>
#include <debugnet.h>
#define  fprintf  debugNetPrintf
#define  ERROR    DEBUGNET_ERROR
#define  DEBUG    DEBUGNET_DEBUG
#define  INFO     DEBUGNET_INFO

#else // on linux

#include <stdio.h>
#define  debugNetPrintf  fprintf
#define  ERROR           stderr
#define  DEBUG           stdout
#define  INFO            stdout

#endif // (__ORBIS__)


/* report passed argv, it's working, don't touch it */
int report_print(int my_argc, char **my_argv)
{
    fprintf(DEBUG, "number of passed argv: %d\n", my_argc);

    int    i = 0;
    char **p = my_argv;
    while(*p != NULL)
    {
        fprintf(DEBUG, "argv[%d] = '%s'\t%p\n", i++, *p, p);
        p++;
    }

    return 0;
}

static int count = 0;

char **build_cmd(char *cmd_line)
{
    fprintf(DEBUG, "%s start\n", __FUNCTION__);
/* this worked too
    char cmd[256];
    sprintf(&cmd[0], "blah -ar 48000 -c 2");
    printf("%s\n", cmd);
*/
    const char s[2] = " "; // split at spaces

    char **my_b = calloc(10, sizeof(char *));
//  fprintf(DEBUG, "%s before strtok\n", __FUNCTION__);
    char *token = strtok(cmd_line, s); /* get the first token */
//  fprintf(DEBUG, "%s after strtok\n", __FUNCTION__);

    //printf("%p %p\n", cmd, my_b);
    int  i = 0; // to count

    while(token) /* walk through other tokens */
    {
        my_b[i] = strdup(token);
        fprintf(DEBUG, "%d: '%s'\t\t%p '%s' %p\n", i, token, token,
                                                 my_b[i], my_b[i]);
        token = strtok(NULL, s);
        i++;
    }
    count = i; // save args_count
    //test(i, my_b);
    fprintf(DEBUG, "%s return: %p\n", __FUNCTION__, my_b);
    return my_b;
}

/* demo app */
int demo(int argc, char **argv)
{
    // this works because the intrinsic way of how main grab arguments when executing app
    //report_print(argc, argv);

    /* build a valid commandline and pass to ffplay */
    char  cmd[256] = "./vt_demo_ffplay -v debug -ac 2 -ar 48000 /hostapp/main.mp3";
    char **out = build_cmd(&cmd[0]);

// works!
    //char cmd[12] = "abc def ghi";
    //char **out   = build_cmd(&cmd[0]);


                //char **out   = build_cmd(*argv);

    fprintf(DEBUG, "%s after build_cmd\n", __FUNCTION__);

    report_print(count, out);

    // cleanup
    if (out)
    {
        // free the strdupes
        for (int i = 0; i < count; ++i) free(out[i]), out[i] = NULL;
        // free double pointer
        free(out), out = NULL;
    }

/* 
    goal is create a custom set of (int my_argc, char **my_argv) to be passed to test(),
    we need to construct a VALID doublepointer array
*/
    return 0;
}
