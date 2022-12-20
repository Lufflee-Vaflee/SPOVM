#include "stdio.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include "sys/stat.h"
#include <locale.h>

bool symbolic_flag = true;
bool catalogs_flag = true;
bool files_flag = true;
bool sorting_flag = false;

const char* SYMBOLIC_FLAG = "-l";
const char* CATALOGS_FLAG = "-d";
const char* FILES_FLAG = "-f";
const char* SORTING_FLAG = "-s";

int scan_dir(char* path);

int main(int argc, char *argv[])
{

    char* launch_dir;
    int i = 1;

    if(argc == 1)
    {
        launch_dir = (char*)malloc(sizeof(char) * 256);
        launch_dir = getcwd(launch_dir, 256);
    }
    else if(argv[1][0] != '-')
    {
        launch_dir = argv[1];
        i++;
    }
    
    for(i; i < argc; i++)
    {
        if(!strcmp(argv[i], SYMBOLIC_FLAG))
            symbolic_flag = true;
        else if(!strcmp(argv[i], CATALOGS_FLAG))
            catalogs_flag = true;
        else if(!strcmp(argv[i], FILES_FLAG))
            files_flag = true;
        else if(!strcmp(argv[i], SORTING_FLAG))
            sorting_flag = true;
        else
        {
           printf("\n Undefined argument at %d position \n", i);
           return -1;
        }
    }

    if (!symbolic_flag && !catalogs_flag && !files_flag)
    {
        symbolic_flag = true;
        catalogs_flag - true;
        files_flag = true;
    }

    return scan_dir(launch_dir);
}   

int scan_dir(char* path)
{
    DIR* curr_dir = opendir(path);
    struct dirent* entry;
    struct stat s;
    char* curr_obj = (char*)malloc(sizeof(char) * 256);

    while ( (entry = readdir(curr_dir)) != NULL) 
    {
        strcpy(curr_obj, path);
        curr_obj = strncat(strncat(curr_obj, "/", 1), entry->d_name, strlen(entry->d_name));

        stat(curr_obj, &s);
        if(S_ISDIR(s.st_mode))
        {
            if(catalogs_flag && strcmp(entry->d_name, "..") && strcmp(entry->d_name, "."))
                printf("%s\n", curr_obj);
            if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
                scan_dir(curr_obj);
        }
        else if(S_ISREG(s.st_mode) && files_flag)
            printf("%s\n", curr_obj);
        else if(S_ISLNK(s.st_mode) && symbolic_flag) 
            printf("%s\n", curr_obj);
        else
            printf("something else: %s", curr_obj);
    };

    //int strcoll(const char *s1, const char *s2);
    free(curr_obj);
    closedir(curr_dir);
    return 0;
}