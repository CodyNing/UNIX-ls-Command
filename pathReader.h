#ifndef _PATHREADER_H_
#define _PATHREADER_H_

#define FORMAT_LEN 128
#define FORMAT_INODE_LEN 8
#define FORMAT_LONGLISTING_LEN 64
#define MODE_STR_LEN 11
#define MONTH_STR_LEN 4
#define BASE_YEAR 1900

#include "argReader.h"

void PathReader_traverse(argSet *pArgSet);

#endif