#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define FILE_MAX_SIZE 1024

extern char **environ;

int main(int argc, char *argv[], char *envp[])
{
    printf("я родился\n");  //не забыть удалить
    FILE* reduced_env_file = fopen(argv[1], "r");                                       //опускаем проверку т.к. в родительском процессе она уже была, и открываем файл сокращенного набора переменных среды
    char string[FILE_MAX_SIZE];                                                         //строка для чтения файла сокращенного набора переменных окружения, сделано грубо, через массив фикс. размера, но думаю сойдет
    char* temp;                                                                         //служебная переменнная
    char** reduced_env_arr = (char**)malloc(sizeof(char*) * 1);                         //массив строк предназначенных для сохранения в память содержимого фала
    reduced_env_arr[0] = NULL;                                                          //окончание массива держим NULL

    fgets(string, FILE_MAX_SIZE, reduced_env_file);                                     //подразумевается, что файл состоит из одной строки, т.е. обрабатываться будет только первая строка
    
    int counter = 1;
    if(temp = strtok(string, ", "))                                                     //подразумевается, что разделителем для названий переменных в файле служит сочетание символов ", "
    {                                                                                   //отдельный блок if для первого элемента создан из-за нюансов работы функции strtok
        reduced_env_arr[0] = temp;                                                      
        reduced_env_arr = (char**)realloc(reduced_env_arr, sizeof(char*) * 2);          
        reduced_env_arr[1] = NULL;
    }
    while(temp = strtok(NULL, ", "))                                                    //заполняем остальные элементы массива
    {
        reduced_env_arr[counter] = temp;
        counter++;
        reduced_env_arr = (char**)realloc(reduced_env_arr, sizeof(char*) * (counter + 1));
        reduced_env_arr[counter] = NULL;
    }

    printf("Вывод сокращенного набора переменных среды:\n");
    switch(atoi(argv[2]))                                                               //выводим сокращенный набор переменных, способ вывода зависит от argv[2] переданного от отца
    {
        case 0:                                                                         //вывод с помощью getenv()
            for(int i = 0; reduced_env_arr[i] != NULL; i++)
                if(getenv(reduced_env_arr[i]))
                    printf("%s=%s\n", reduced_env_arr[i], getenv(reduced_env_arr[i]));
        break;
        case 1:                                                                         //вывод с помощью envp
            temp = (char*)malloc(sizeof(char*) * 1);                                    //переменная копирующая envp[i], копирование из envp необходимо, т.к. strtok меняет содержимое обрабатываемой строки, чего для envp допускать нельзя
            for(int i = 0; envp[i] != NULL; i++)
            {
                temp = (char*)realloc(temp, strlen(envp[i]));                                  //перевыделяем temp в соответствии с необходимым размером
                for(int j = 0; reduced_env_arr[j] != NULL; j++)
                {
                    strcpy(temp, envp[i]);                                              //производим копирование
                    if(!strcmp(strtok(temp, "="), reduced_env_arr[j]))                  //
                    {
                        printf("%s=%s\n", temp, strtok(NULL, "="));
                        continue;
                    }
                }
            }
            free(temp);                                                                 //очищаем память 
        break;
        case 2:                                                                         //аналогичный предыдущему вывод, но с помощью environ
            temp = (char*)malloc(sizeof(char*) * 1);
            for(int i = 0; environ[i] != NULL; i++)
            {
                temp = (char*)realloc(temp, strlen(environ[i]));
                for(int j = 0; reduced_env_arr[j] != NULL; j++)
                {
                    strcpy(temp, environ[i]);
                    if(!strcmp(strtok(temp, "="), reduced_env_arr[j]))
                    {
                        printf("%s=%s\n", temp, strtok(NULL, "\0"));
                        continue;
                    }
                }
            }
            free(temp);
        break;
    }

    printf("PID: %d\nPPID: %d\n", getpid(), getppid());                                 //выводим PID и PPID https://www.opennet.ru/man.shtml?topic=getpid&category=2&russian=0

    free(reduced_env_arr);                                                              //освобождаем массив
    close(reduced_env_file);                                                            //закрываем файл
    return 0;
}