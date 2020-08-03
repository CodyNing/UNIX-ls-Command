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

argSet *ArgReader_getArgSet(int argc, char **argv)
{
    argSet *pArgset = malloc(sizeof(argSet));
    List *pPathList = List_create();
    ArgReader_getArgSet_r(pArgset, pPathList, argc, argv);
    return pArgset;
}

void ArgReader_getArgSet_r(argSet *pArgset, List *pPathList, int argc, char **argv)
{
    assert(pArgset && pPathList);
    pArgset->pathList = pPathList;
    for (size_t i = 1; i < argc; ++i)
    {
        char *arg = argv[i];
        if (isSwitch(arg))
        {
            getSwitch(arg, pArgset);
        }
        else
        {
            if(List_append(pPathList, arg) == -1){
                printf("myls: Cannot process more then %d paths", LIST_MAX_NUM_NODES);
                exit(EXIT_FAILURE);
            }
        }
    }
}