#include <stdlib.h>
#include "argReader.h"
#include "pathReader.h"


int main(int argc, char **argv){
    argSet *args = ArgReader_getArgSet(argc, argv);
    PathReader_traverse(args);
    free(args->pathList);
    free(args);
}