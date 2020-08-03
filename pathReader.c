#define _DEFAULT_SOURCE
#include "pathReader.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>

static char *getGroup(gid_t grpNum)
{
    struct group *grp = getgrgid(grpNum);

    if (grp)
    {
        return grp->gr_name;
    }
    else
    {
        printf("No group name for %u found\n", grpNum);
        return NULL;
    }
}

static char *getUserName(uid_t uid)
{
    struct passwd *pw = getpwuid(uid);

    if (pw)
    {
        return pw->pw_name;
    }
    else
    {
        printf("No name found for %u\n", uid);
        return NULL;
    }
}

static void getModeStr(mode_t m, char *modeStr)
{
    switch (m & __S_IFMT)
    {
    case __S_IFBLK:
        modeStr[0] = 'b';
        break;
    case __S_IFCHR:
        modeStr[0] = 'c';
        break;
    case __S_IFIFO:
        modeStr[0] = 'p';
        break;
    case __S_IFREG:
        modeStr[0] = '-';
        break;
    case __S_IFDIR:
        modeStr[0] = 'd';
        break;
    case __S_IFLNK:
        modeStr[0] = 'l';
        break;
    }
    const char symbol[] = "rwx";
    for (size_t i = 0; i < MODE_STR_LEN - 2; ++i)
    {
        if (m & (1 << (MODE_STR_LEN - 3 - i)))
        {
            modeStr[i + 1] = symbol[i % 3];
        }
        else
        {
            modeStr[i + 1] = '-';
        }
    }
    modeStr[MODE_STR_LEN - 1] = '\0';
}

static void printFileinfo(const char *dirp, const char *name){
    struct stat st;
    struct tm lt;
    char path[PATH_MAX + 1];
    char modeStr[11];
    char monthStr[4];

    memset(path, 0, PATH_MAX + 1);
    strncpy(path, dirp, strlen(dirp));
    strcat(path, name);
    
    lstat(path, &st);

    getModeStr(st.st_mode, modeStr);

    localtime_r(&st.st_mtime, &lt);

    strftime(monthStr, 4, "%b", &lt);

    printf("%ld %s %ld %s %s %ld %s %2d %d %02d:%02d %s\n",
        st.st_ino, 
        modeStr, 
        st.st_nlink, 
        getUserName(st.st_uid), 
        getGroup(st.st_gid), 
        st.st_size,
        monthStr,
        lt.tm_mday,
        lt.tm_year + 1900,
        lt.tm_hour,
        lt.tm_min,
        name);
}

static void traverseDir(const char *path, bool opt_i, bool opt_l, bool opt_R)
{

    struct dirent **namelist;
    int n;
    n = scandir(path, &namelist, NULL, alphasort);
    if (n == -1)
    {
        fprintf(stderr, "myls: fail to open '%s'", path);
        return;
    }

    for (size_t i = 0; i < n; ++i)
    {
        printFileinfo(path, namelist[i]->d_name);
        free(namelist[i]);
    }
}

static void traverse(const char *path, bool opt_i, bool opt_l, bool opt_R)
{
    struct stat st;
    if (lstat(path, &st) == -1)
    {
        fprintf(stderr, "myls: cannot access '%s': No such file or directory", path);
        return;
    }

    if (S_ISDIR(st.st_mode))
    {
        traverseDir(path, opt_i, opt_l, opt_R);
    }
    else if (S_ISREG(st.st_mode))
    {
        printFileinfo("./", path);
    }
    else if (S_ISLNK(st.st_mode))
    {
        printFileinfo("./", path);
    }
}

void PathReader_traverse(argSet *pArgSet)
{
    assert(pArgSet);
    char *path;
    while((path = List_trim(pArgSet->pathList))){
        traverse(path, pArgSet->showIndex, pArgSet->longListing, pArgSet->recursive);
    }
}