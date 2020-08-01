/*
    listing local folder using getdents()
*/

#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>  // malloc, qsort, free

/*
  Glibc does not provide a wrapper for
  getdents, so we call it by syscall()
*/
#include <sys/syscall.h> // SYS_getdents

//#include "defines.h"

char *entryName(int entryType) {
    switch(entryType)
    {
        case 4:  return "DIR";
        case 8:  return "FILE";
        default: return "OTHER";
    }
}


/*
    minimal folder listing:
    ps4 uses getdents(),
    glibc doesn't have so use syscall
    test implementation, now unused
*/
int ls_dir(char *dirpath)
{
    int dfd = open(dirpath, O_RDONLY, 0); // try to open dir
    if(dfd < 0)
        { fprintf(stdout, "Invalid directory. (%s)\n", dirpath); return -1; }
    else 
        { fprintf(stdout, "open(%s)\n", dirpath); }

    struct dirent *dent;
    char           buffer[512]; // fixed buffer

    memset(buffer, 0, sizeof(buffer));

#if 1
    while(syscall(SYS_getdents, dfd, buffer, sizeof(buffer)) != 0)
#else
    while(getdents(dfd, buffer, sizeof(buffer)) != 0) // get directory entries
#endif
    {
        dent = (struct dirent *)buffer;

        while(dent->d_fileno)
        {
            fprintf(stdout, "[%p]: %8x %2d, type: %4d, '%s'\n", // report entry
                dent, 
                dent->d_fileno, dent->d_reclen, 
                dent->d_type,   dent->d_name /* on pc we step back by 1*/ -1);

            dent = (struct dirent *)((void *)dent + dent->d_reclen);

            if(dent == (void*) &buffer[512]) break; // refill buffer
        }
        memset(buffer, 0, sizeof(buffer));
    }
    close(dfd);

    return 0;
}





/* two pass folder parser */
typedef struct
{
    char  *name;
    //size_t size;
} entry_t;

/* qsort struct comparison function (C-string field) */
static int struct_cmp_by_name(const void *a, const void *b)
{
    entry_t *ia = (entry_t *)a;
    entry_t *ib = (entry_t *)b;
    return strcmp(ia->name, ib->name);
/* strcmp functions works exactly as expected from comparison function */
}

int num = 0; // total item count

int get_item_count(void)
{
    return num;
}

void *free_item_entries(entry_t *e)
{
    for (int i = 0; i < num; ++i)
    {
        if(e->name) free(e->name), e->name = NULL;
    }
    free(e), e = NULL;
}

entry_t *get_item_entries(char *dirpath)
{   
    struct dirent *dent;
    char           buffer[512]; // fixed buffer

    int dfd    = 0,
        n, r   = 1;    // item counter, rounds to loop
    entry_t *p = NULL; // we fill this struct with items

loop:
    n = 0;
    fprintf(stderr, "loop: %d, num:%d\n", r, num);

    // try to open dir
    dfd = open(dirpath, O_RDONLY, 0);
    if(dfd < 0)
        { fprintf(stdout, "Invalid directory. (%s)\n", dirpath); return NULL; }
    else
        { fprintf(stdout, "open(%s)\n", dirpath); }

    memset(buffer, 0, sizeof(buffer));

#if 1
    while(syscall(SYS_getdents, dfd, buffer, sizeof(buffer)) != 0)
#else
    while(getdents(dfd, buffer, sizeof(buffer)) != 0) // get directory entries
#endif
    {
        dent = (struct dirent *)buffer;

        while(dent->d_fileno)
        {
            switch(r)
            {   // first round: just count items
                case 1: 
                {
                #if 0
                    fprintf(stdout, "[%p]: %8x %2d, type: %4d, '%s'\n", // report entry
                        dent,
                        dent->d_fileno, dent->d_reclen,
                        dent->d_type,   dent->d_name /* on pc we step back by 1*/ -1);
                #endif
                    break;
                }
                // second round: store filenames
                case 0: p[n].name = strdup(dent->d_name -1); break;
            }
            n++;

            dent = (struct dirent *)((void *)dent + dent->d_reclen);

            if(dent == (void*) &buffer[512]) break; // refill buffer
        }
        memset(buffer, 0, sizeof(buffer));
    }
    close(dfd);

    // on first round, calloc for our list
    if(!p)
    {   // now n holds total item count, note it
        p = calloc(n, sizeof(entry_t)); num = n;
    }

    // first round passed, loop
    r--; if(!r) goto loop;

    // report count
    fprintf(stderr, "%d items at %p\n", num, p);

    /* resort using custom comparision function */
    qsort(p, num, sizeof(entry_t), struct_cmp_by_name);

    // report items
    //for (int i = 0; i < num; ++i) fprintf(stderr, "%s\n", p[i].name);

    return p;
}

/*
int main(int argc, char const *argv[])
{
    ls_dir(argv[1]);
    return 0;
}
*/

/*
Valid directory.
[7efbcf920]:       60 12, type:  4, .
[7efbcf92c]:        6 12, type:  4, ..
[7efbcf938]:       61 32, type:  4, remote-web-inspector
[7efbcf958]:      1f8 16, type:  4, theme
[7efbcf968]: ffffffff 24, type:  8, I18N.CJK.sdll
[7efbcf980]: ffffffff 20, type:  8, I18N.sdll
[7efbcf994]: ffffffff 32, type:  8, LoginMgrUIProcess.self
[7efbcf9b4]: f000004a 28, type:  8, MonoCompiler.elf
[7efbcf9d0]: f0000058 36, type:  8, Sce.PlayStation.Core.sdll
*/