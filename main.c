#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

char* getAndPrintGroup(gid_t grpNum)
{
    struct group *grp = getgrgid(grpNum);

    if (grp) {
        //printf("The group ID %u -> %s\n", grpNum, grp->gr_name);
        return grp->gr_name;
    } else {
        printf("No group name for %u found\n", grpNum);
        return NULL;
    }
}

char* getAndPrintUserName(uid_t uid)
{
    struct passwd *pw = getpwuid(uid);

    if (pw) {
        //printf("The user ID %u -> %s\n", uid, pw->pw_name);
        return pw->pw_name;
    } else {
        perror("Hmm not found???");
        printf("No name found for %u\n", uid);
        return NULL;
    }
}

void getModeStr(mode_t m, char *modeStr){
    switch(m & __S_IFMT){
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
    const char symbol[] = "rwxrwxrwx";
    for(size_t i = 0; i < 9; ++i){
        if(m & (1 << (8-i))){
            modeStr[i+1] = symbol[i];
        }
        else{
            modeStr[i+1] = '-';
        }
    }
    modeStr[10] = '\0';
}

int main(int argc, char **argv){
    const char timeFormat1[] = "%b %d %Y ";
    const char timeFormat2[] = "%H:%M";
    struct tm lt;
    struct dirent *ent = NULL;
    struct stat st;
    memset(&st, 0, sizeof(st));
    char *path = "/home/crystalloid/cmpt300/";
    char realpath[PATH_MAX + 1];
    DIR *dir = opendir(path);
    while(NULL != (ent = readdir(dir))){
        memset(realpath, 0, PATH_MAX + 1);
        strncpy(realpath, path, strlen(path));
        strcat(realpath, ent->d_name);
        lstat(realpath, &st);
        char modeStr[11];
        getModeStr(st.st_mode, modeStr);

        char datetimeStr[18];
        char timeStr[6];

        localtime_r(&st.st_mtime, &lt);
        strftime(datetimeStr, 18, timeFormat1, &lt);
        strftime(timeStr, 6, timeFormat2, &lt);

        strcat(datetimeStr, timeStr);

        printf("%ld %s %ld %s %s %ld %s %s\n",
            ent->d_ino, 
            modeStr, 
            st.st_nlink, 
            getAndPrintUserName(st.st_uid), 
            getAndPrintGroup(st.st_gid), 
            st.st_size,
            datetimeStr, 
            ent->d_name);
        // if(st){
        // } else {
        //     printf("%ld %s\n", ent->d_ino, ent->d_name);
        // }
    }

    return 0;
}