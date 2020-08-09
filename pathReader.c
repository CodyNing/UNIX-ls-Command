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
#include <libgen.h>
#include <unistd.h>
#include <errno.h>

const char *FORMAT_DEFAULT = "%s\n";
const char *FORMAT_INODE = "%%%dld ";
const char *FORMAT_LONGLISTING = "%%s %%%dld %%-%ds %%-%ds %%%dld %%s %%2d %%d %%02d:%%02d ";

const char SPECIAL_CHAR[] = {' ', '!', '$', '\'', '^', '&', '(', ')'};

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

static size_t getIntLen(size_t i)
{
    char str[21];
    snprintf(str, 21, "%ld", i);
    return strlen(str);
}

static bool isSpecialPath(const char *path)
{
    size_t pathLen = strlen(path) + 1;
    char pathcopy[pathLen];
    strncpy(pathcopy, path, pathLen);
    char *tmp = basename(pathcopy);
    for (char *p = tmp; *p != '\0'; ++p)
    {
        for (size_t i = 0; i < 7; ++i)
        {
            if (*p == SPECIAL_CHAR[i])
            {
                return true;
            }
        }
    }
    return false;
}

static void quoteStr(const char *str, char *quoteStr)
{
    size_t len = strlen(str);
    memset(quoteStr, 0, len + 3);
    quoteStr[0] = '\'';
    strncpy(quoteStr + 1, str, len);
    quoteStr[len + 1] = '\'';
}

static void getFilename(char *path, char *name, bool isLnk, size_t lnkSize)
{
    size_t pathLen = strlen(path) + 1;
    char pathcopy[pathLen];
    strncpy(pathcopy, path, pathLen);
    char *tmp = basename(pathcopy);
    size_t namelen = strlen(tmp);

    if (isSpecialPath(path))
    {
        quoteStr(tmp, name);
        namelen += 2;
    }
    else
    {
        strcpy(name, tmp);
    }
    if (isLnk)
    {
        const char *arrow = " -> ";
        char lnkPath[lnkSize + 1];

        if (readlink(path, lnkPath, lnkSize + 1) == -1)
        {
            fprintf(stderr, "myls: unable to read symbolic link '%s': %s\n", path, strerror(errno));
            return;
        }
        lnkPath[lnkSize] = '\0';

        strcat(name, arrow);
        namelen += 4;

        if (isSpecialPath(name))
        {
            quoteStr(lnkPath, name + namelen);
        }
        else
        {
            strcat(name, lnkPath);
        }
    }
}

static void formatPrintPaths(int pathNum, char **pathList, bool opt_i, bool opt_l)
{

    char format[FORMAT_LEN] = "";
    bool hasSpecial = false;

    size_t lenColNode = 1,
           lenCollnk = 1,
           lenColUsr = 1,
           lenColGrp = 1,
           lenColSize = 1,
           i = 0;

    struct stat st_arr[pathNum];

    for (i = 0; i < pathNum; ++i)
    {
        char *path = pathList[i];

        size_t len;
        struct stat st;

        //all path has been checked, this will never happen
        if (lstat(path, st_arr + i) == -1)
        {
            fprintf(stderr, "myls: cannot access '%s': %s\n", path, strerror(errno));
            --i;
            --pathNum;
            continue;
        }

        st = st_arr[i];

        if (isSpecialPath(path))
        {
            hasSpecial = true;
        }

        if (opt_i)
        {
            len = getIntLen(st.st_ino);
            if (len > lenColNode)
            {
                lenColNode = len;
            }
        }
        if (opt_l)
        {
            len = getIntLen(st.st_nlink);
            if (len > lenCollnk)
            {
                lenCollnk = len;
            }
            len = strlen(getUserName(st.st_uid));
            if (len > lenColUsr)
            {
                lenColUsr = len;
            }
            len = strlen(getGroup(st.st_gid));
            if (len > lenColGrp)
            {
                lenColGrp = len;
            }
            len = getIntLen(st.st_size);
            if (len > lenColSize)
            {
                lenColSize = len;
            }
        }
    }

    if (opt_i)
    {
        char formati[FORMAT_INODE_LEN];
        snprintf(formati, FORMAT_INODE_LEN, FORMAT_INODE, lenColNode);
        strncpy(format, formati, FORMAT_INODE_LEN);
    }
    if (opt_l)
    {
        char formatl[FORMAT_LONGLISTING_LEN];
        snprintf(formatl, FORMAT_LONGLISTING_LEN, FORMAT_LONGLISTING, lenCollnk, lenColUsr, lenColGrp, lenColSize);
        strcat(format, formatl);
    }

    strcat(format, FORMAT_DEFAULT);

    for (i = 0; i < pathNum; ++i)
    {
        struct stat st = st_arr[i];
        char *path = pathList[i];
        char filename[NAME_MAX * 2 + PATH_MAX];
        if (hasSpecial && !isSpecialPath(path))
        {
            filename[0] = ' ';
            getFilename(path, filename + 1, S_ISLNK(st.st_mode), st.st_size);
        }
        else
        {
            getFilename(path, filename, S_ISLNK(st.st_mode), st.st_size);
        }

        if (opt_l)
        {
            struct tm lt;
            char modeStr[MODE_STR_LEN];
            char monthStr[MONTH_STR_LEN];

            getModeStr(st.st_mode, modeStr);

            localtime_r(&st.st_mtime, &lt);

            strftime(monthStr, MONTH_STR_LEN, "%b", &lt);
            if (opt_i)
            {
                printf(format,
                       st.st_ino,
                       modeStr,
                       st.st_nlink,
                       getUserName(st.st_uid),
                       getGroup(st.st_gid),
                       st.st_size,
                       monthStr,
                       lt.tm_mday,
                       lt.tm_year + BASE_YEAR,
                       lt.tm_hour,
                       lt.tm_min,
                       filename);
            }
            else
            {
                printf(format,
                       modeStr,
                       st.st_nlink,
                       getUserName(st.st_uid),
                       getGroup(st.st_gid),
                       st.st_size,
                       monthStr,
                       lt.tm_mday,
                       lt.tm_year + BASE_YEAR,
                       lt.tm_hour,
                       lt.tm_min,
                       filename);
            }
        }
        else if (opt_i)
        {
            printf(format, st.st_ino, filename);
        }
        else
        {
            printf(format, filename);
        }
    }
}

static void splitPathList(size_t pathNum, char **pathList, char **dirList, size_t *dirCount, char **nonDirList, size_t *nonDirCount)
{
    size_t i = 0;
    *dirCount = 0;
    *nonDirCount = 0;
    for (; i < pathNum; ++i)
    {
        char *path = pathList[i];

        struct stat st;

        if (lstat(path, &st) == -1)
        {
            fprintf(stderr, "myls: cannot access '%s': %s\n", path, strerror(errno));
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            dirList[(*dirCount)++] = path;
        }
        else
        {
            nonDirList[(*nonDirCount)++] = path;
        }
    }
}

static int ascii_sort(const struct dirent **p1, const struct dirent **p2)
{
    char const *pc1 = (*p1)->d_name, *pc2 = (*p2)->d_name;
    for (; *pc1 == *pc2 && *pc1 != '\0' && *pc2 != '\0'; ++pc1, ++pc2)
        ;
    return (*pc1 - *pc2);
}

static int hiddenFilter(const struct dirent *p)
{
    return (p->d_name[0] != '.');
}

static void traverseDir(const char *path, bool opt_i, bool opt_l, bool opt_R)
{

    struct dirent **namelist;
    const char *slash = "/";
    int fileCount;
    size_t i = 0;
    fileCount = scandir(path, &namelist, hiddenFilter, ascii_sort);
    if (fileCount == -1)
    {
        fprintf(stderr, "myls: cannot open directory '%s': %s\n", path, strerror(errno));
        return;
    }

    char *pathList[fileCount];

    for (i = 0; i < fileCount; ++i)
    {
        size_t pathLen = strlen(path);
        pathList[i] = malloc(pathLen + NAME_MAX + 1);
        strncpy(pathList[i], path, NAME_MAX);
        if (path[pathLen - 1] != '/')
        {
            strcat(pathList[i], slash);
        }
        strcat(pathList[i], namelist[i]->d_name);
        pathList[i][pathLen + NAME_MAX] = '\0';
    }

    printf("%s:\n", path);
    formatPrintPaths(fileCount, pathList, opt_i, opt_l);

    if (opt_R)
    {
        char *path;
        for (size_t i = 0; i < fileCount; ++i)
        {
            path = pathList[i];
            struct stat st;

            if (lstat(path, &st) == -1)
            {
                fprintf(stderr, "myls: cannot access '%s': %s\n", path, strerror(errno));
                continue;
            }

            if (S_ISDIR(st.st_mode))
            {
                printf("\n");
                traverseDir(path, opt_i, opt_l, opt_R);
            }
        }
    }
    for (i = 0; i < fileCount; ++i)
    {
        free(namelist[i]);
        free(pathList[i]);
    }
    free(namelist);
}

void PathReader_traverse(argSet *pArgSet)
{
    assert(pArgSet);
    char *path;
    char *dirList[MAX_PATH_NUM];
    char *nonDirList[MAX_PATH_NUM];
    size_t dirCount = 0, nonDirCount = 0;
    splitPathList(pArgSet->pathNum, pArgSet->pathList, dirList, &dirCount, nonDirList, &nonDirCount);
    formatPrintPaths(nonDirCount, nonDirList, pArgSet->showIndex, pArgSet->longListing);
    for (size_t i = 0; i < dirCount; ++i)
    {
        path = dirList[i];
        printf("\n");
        traverseDir(path, pArgSet->showIndex, pArgSet->longListing, pArgSet->recursive);
    }
}