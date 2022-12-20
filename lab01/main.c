#include "stdio.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include "sys/stat.h"
#include <locale.h>

bool symbolic_flag = false;                                                                 //флаги опций вывода
bool catalogs_flag = false;
bool files_flag = false;
bool sorting_flag = false;

const char* SYMBOLIC_FLAG = "-l";                                                           //маски для флагов
const char* CATALOGS_FLAG = "-d";
const char* FILES_FLAG = "-f";
const char* SORTING_FLAG = "-s";

int scan_dir(char* path);                                                                   //прототип функции сканирования

int main(int argc, char *argv[])
{

    char* launch_dir;
    int i = 1;                                                                              //приравниваем единице для итерации по аргументам командной строки(пропускаем первый)

    if(argc > 1 && argv[1][0] != '-')                                                                   //проверям является ли аргумент строкой начала поиска
    {
        launch_dir = argv[1];
        i++;                                                                                //увеличиваем на 1 т.к. первый аргумент использован
    }
    else
    {
        launch_dir = (char*)malloc(sizeof(char) * 256);                                     //если не нашли дирректорию поиска, ставим дирректорию запуска программы
        launch_dir = getcwd(launch_dir, 256);                                               //http://www.c-cpp.ru/content/getcwd
    }
    
    for(i; i < argc; i++)                                                                   //раставляем флаги опций
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

    if (!symbolic_flag && !catalogs_flag && !files_flag)                                        //если ни одного флага нет, значит активируем все флаги
    {
        symbolic_flag = true;
        catalogs_flag = true;
        files_flag = true;
    }

    return scan_dir(launch_dir);                                                                //вызываем функцию сканирования дирректорий
}   

int scan_dir(char* path)
{
    DIR* curr_dir = opendir(path);                                                              //открываем дирректорию поиска и если таковой нет, возвращаем ошибку
    if(curr_dir == NULL)
    {
        printf("\n Dirrectory not found \n");
        return -1;
    }

    struct dirent* entry;
    struct stat s;

    char** curr_dir_objects = (char**)malloc(sizeof(char*) * 100);                              //объявляем и выделяем массив путей до фалов/каталогов
    int size = 0;                                                                               //размер массива

    while ( (entry = readdir(curr_dir)) != NULL && size < 100)                                  //читаем все объекты фаловой системы в текущей дирректории и добавляем в массив https://firststeps.ru/linux/r.php?20
    {
        curr_dir_objects[size] = (char*)malloc(sizeof(char) * 256);
        strcpy(curr_dir_objects[size], entry->d_name);
        size++;
    };

    char* change;
    for(int i = 0; i < size && sorting_flag; i++)                                               //надеюсь меня простят за сортировку пузырьком, соритруем, если был поставлен соответствующий флаг
        for(int j = i + 1; j < size; j++)
        {
            if(strcoll(curr_dir_objects[i], curr_dir_objects[j]) > 0)                           //для сравнения используем strcoll, которая работает в соответствии с LC_COLLATE
            {
                change = curr_dir_objects[i];
                curr_dir_objects[i] = curr_dir_objects[j];
                curr_dir_objects[j] = change;
            }
        }

    char* curr_obj = (char*)malloc(sizeof(char) * 256);                                         //служебная строка для текущего пути до файла/каталога

    for(int i = 0; i < size; i++)                                                               //цикл вывода 
    {
        if(!strcmp(curr_dir_objects[i], "..") || !strcmp(curr_dir_objects[i], "."))             //Пропускаем "." и ".."
            continue;
        strcpy(curr_obj, path);
        curr_obj = strcat(strcat(curr_obj, "/"), curr_dir_objects[i]);                          //получаем полный путь до файла/каталога

        stat(curr_obj, &s);                                                                     //получаем состояние файла  https://ru.manpages.org/stat/2
        if(S_ISDIR(s.st_mode))                                                                  //Проверяем тип файла  https://www.gnu.org/software/libc/manual/html_node/Testing-File-Type.html
        {
            if(catalogs_flag)                                                                   //если стоит флаг каталога выводим его
                printf("%s\n", curr_obj);
            scan_dir(curr_obj);                                                                 //рекурсивно вызываем ту же функцию для полученного каталога
        }
        else if(S_ISREG(s.st_mode) && files_flag)                                               //выводим соответствующие объекты, если стоят флаги
            printf("%s\n", curr_obj);
        else if(S_ISLNK(s.st_mode) && symbolic_flag) 
            printf("%s\n", curr_obj);
    };

    for(int i = 0; i < size; i++)                                                               //освобождаем память массива строк и других массивов
        free(curr_dir_objects[i]);
    free(curr_dir_objects);
    free(curr_obj);
    closedir(curr_dir);                                                                         //закрываем дирректорию
    return 0;
}