#include "argReader.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

static bool isSwitch(const char *arg)
{
    return arg && *arg == '-' && arg[1] != '\0';
}

static void getSwitch(const char *arg, argSet *pArgSet)
{
    for (size_t i = 1; arg[i] != '\0'; ++i)
    {
        switch (arg[i])
        {
        case 'i':
            pArgSet->showIndex = true;
            break;
        case 'l':
            pArgSet->longListing = true;
            break;
        case 'R':
            pArgSet->recursive = true;
            break;
        default:
            printf("myls: invalid option -- '%c'\n", arg[i]);
            exit(EXIT_FAILURE);
        }
    }
}

static int ascii_sort(const void *p1, const void *p2)
{
    char *pc1 = *(char * const *)p1, *pc2 = *(char * const *)p2;
    for (; *pc1 == *pc2 && *pc1 != '\0' && *pc2 != '\0'; ++pc1, ++pc2);
    return (*pc1 - *pc2);
}

argSet *ArgReader_getArgSet(int argc, char **argv)
{
    argSet *pArgset = malloc(sizeof(argSet));
    char **pPathList = malloc(sizeof(char *) * argc - 1);
    ArgReader_getArgSet_r(pArgset, pPathList, argc, argv);
    return pArgset;
}

void ArgReader_getArgSet_r(argSet *pArgset, char **pPathList, int argc, char **argv)
{
    size_t pathCount = 0, i = 0;
    assert(pArgset && pPathList);
    pArgset->pathList = pPathList;
    for (i = 1; i < argc; ++i)
    {
        char *arg = argv[i];
        if (isSwitch(arg))
        {
            getSwitch(arg, pArgset);
        }
        else
        {
            break;
        }
    }
    for(; i < argc; ++i){
        char *arg = argv[i];
        if (pathCount + 1 > MAX_PATH_NUM)
        {
            printf("myls: Cannot process more then %d paths", MAX_PATH_NUM);
            exit(EXIT_FAILURE);
        }
        pPathList[pathCount++] = arg;
    }
    qsort(pPathList, pathCount, sizeof(char *), ascii_sort);
    pArgset->pathNum = pathCount;
}