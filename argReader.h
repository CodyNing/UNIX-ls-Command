#ifndef _ARGREADER_H_
#define _ARGREADER_H_

#include "list.h"
#include <stdbool.h>

typedef struct argSet_s argSet;
struct argSet_s {
    bool showIndex;
    bool longListing;
    bool recursive;
    List *pathList;
};

argSet* ArgReader_getArgSet(int argc, char **argv);
void ArgReader_getArgSet_r(argSet *pArgset, List *pPathList, int argc, char **argv);

#endif