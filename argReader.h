#ifndef _ARGREADER_H_
#define _ARGREADER_H_

#define MAX_PATH_NUM 65536

#include <stdbool.h>

typedef struct argSet_s argSet;
struct argSet_s {
    bool showIndex;
    bool longListing;
    bool recursive;
    char **pathList;
    int pathNum;
};

argSet* ArgReader_getArgSet(int argc, char **argv);
void ArgReader_getArgSet_r(argSet *pArgset, char **pPathList, int argc, char **argv);

#endif