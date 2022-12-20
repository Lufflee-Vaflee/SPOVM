#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char **environ;                                                                                  //внешний массив переменных среды https://ru.manpages.org/environ/7

char* to_XX(unsigned int num, char buffer[]);                                                           //небольшая функция преобразования числа в строку формата XX (9 - "09")

int main(int argc, char *argv[], char *envp[])                                                          //обычный main, требует 1 аргумент - ссылка на файл сокращенных переменных среды, который передается child
{
    //argv[0]
    //argv[1]
    //argv[2]
    FILE* reduced_env_file;                                                                            //проверяем корректность ввода параметров
    if(argc != 2)
    {
        printf("Некорректный ввод параметров: parent [reduced_environment_file]\n");
        return -1;
    }
    else if(reduced_env_file = fopen(argv[1], "r"))
                fclose(reduced_env_file);
    else
    {
        printf("Некорекктное имя файла сокращенного окружения\n");
        return -1;
    }

    char** sorted_envp = (char**)malloc(sizeof(char*) * 1);                                             //создаем массив для сортировки переменных среды в соотвтствии с LC_COLLATE
    sorted_envp[0] = NULL;
    for(int i = 0; envp[i] != NULL; i++)                                                                //и заполняем его данными
    {
        sorted_envp[i] = envp[i];
        sorted_envp = (char**)realloc(sorted_envp, sizeof(char*) * (i + 2));
        sorted_envp[i + 1] = NULL;
    }

    char* temp;
    for(int i = 0; sorted_envp[i] != NULL; i++)                                                         //выполняем сортировку над массивом как обычно пузырьком)))
    {
        for(int j = i + 1; sorted_envp[j] != NULL; j++)
        {
            if(strcoll(sorted_envp[i], sorted_envp[j]) > 0)
            {
                temp = sorted_envp[i];
                sorted_envp[i] = sorted_envp[j];
                sorted_envp[j] = temp;
            }
        }
        printf("%s\n", sorted_envp[i]);
    }


    char num_xx[3];                                                                                     //строка для работы функции to_XX
    char inp_symb = '\0';                                                                               //переменная для считывания символа
    int count  = 0;
    printf("Для создания дочернего процесса введите один из символов: +, *, &\n");
    printf("Для выхода из программы введите: q\n");
    char* new_argv[4];                                                                                  //массив строк аргументов для дочернего процесса
    new_argv[0] = (char*)malloc(sizeof(char) * 10);                                                     //название процесса (child_XX)
    new_argv[1] = (char*)malloc(sizeof(char) * 512);                                                    //ссылка на файл сокращенного набора переменных среды
    new_argv[2] = (char*)malloc(sizeof(char) * 3);                                                      //код способа вывода(чтобы child выводил переменные среды также как родитель)
    new_argv[3] = NULL;                                                                                 //нулевой указатель для корректной работы
    strcpy(new_argv[1], argv[1]);                                                                       //второй аргумент(ссылка на файл сокращенного набора переменных среды) берем из аргументов родителя
    temp = (char*)malloc(sizeof(char) * 1024);                                                          //служебная переменная
    while(inp_symb != 'q')                                                                              //цикл считывания символа
    {
        scanf(" %c", &inp_symb);
        count++;                                                                                        //переменная пдсчета кол-ва процессов
        if(inp_symb == '+')                                                                             //получаем дочерний файл с помощью команды getenv()
        {
            switch(fork())                                                                              //создаем дочерний процесс https://www.opennet.ru/docs/RUS/linux_parallel/node7.html
            {
                case -1:                                                                                //обработка ошибки его создания
                    printf("Ошибка при создании потомка\n");
                    break;
                case 0:                                                                                 //тело дочернего процесса
                    printf("Привет\n");
                    strcpy(new_argv[0],"child_");                                                       //создаем имя дочернего процесса
                    strcat(new_argv[0], to_XX(count, num_xx));                                          //присоединяем к нему порядковый номер
                    strcpy(new_argv[2], "0");                                                           //ставим код типа вывода
                    if(execve(strcat(getenv("CHILD_PATH"), "/child"), new_argv, envp) == - 1)                             //перезаписываем child программу в процесс https://www.opennet.ru/man.shtml?topic=execve&category=2&russian=0
                    {
                        printf("Не удалось вызвать execve, Проверьте переменную среды CHILD_PATH\n %s\n", getenv("CHILD_PATH"));                    //обрабатываем ошибку
                        return -1;
                    }
                    break;
                default:                                                                                //тело родительского процесса
                    printf("Дочерний процесс успешно создан\n");
                    break;
            }
        }
        else if(inp_symb == '*')                                                                        //аналогичный первому блоку, блок кода, но дочерний файл получаем чтением envp
        {
            int index = 1;
            while(envp[index] != NULL)
            {
                strcpy(temp, envp[index]);
                if(!strcmp(strtok(temp, "="), "CHILD_PATH"))                                            //вот здесь получам название переменной
                {
                    switch (fork())
                    {
                    case -1:
                        printf("Ошибка при создании потомка\n");
                        break;
                    case 0:
                        strcpy(new_argv[0],"child_");
                        strcat(new_argv[0], to_XX(count, num_xx));
                        strcpy(new_argv[2], "1");
                        if(execve(strcat(strtok(NULL, "="), "/child"), new_argv, envp))                                   //а здесь ее значение
                        {
                            printf("Не удалось вызвать execve, Проверьте переменную среды CHILD_PATH\n %s\n", getenv("CHILD_PATH"));
                            return -1;
                        }
                        break;
                    default:
                        printf("Дочерний процесс успешно создан\n");
                        index = 0;
                        break;
                    }
                }
                if(!index)
                    break;
                index++;
            }
        }
        else if(inp_symb == '&')                                                                        //аналогичный второму блоку, блок кода, но вместо envp - environ
        {
            int index = 1;
            while(environ[index] != NULL)
            {
                strcpy(temp, environ[index]);
                if(!strcmp(strtok(temp, "="), "CHILD_PATH"))
                {
                    switch (fork())
                    {
                    case -1:
                        printf("Ошибка при создании потомка\n");
                        break;
                    case 0:
                        strcpy(new_argv[0],"child_");
                        strcat(new_argv[0], to_XX(count, num_xx));
                        strcpy(new_argv[2], "2");
                        if(execve(strcat(strtok(NULL, "="), "/child"), new_argv, envp))     //environ - заменить на сокращенный набор переменных, lj, Добавить проверку на execve и getenv
                        {
                            printf("Не удалось вызвать execve, Проверьте переменную среды CHILD_PATH\n %s\n", getenv("CHILD_PATH"));
                            return -1;
                        }
                        break;
                    default:
                        printf("Дочерний процесс успешно создан\n");
                        index = 0;
                        break;
                    }
                }
                if(!index)
                    break;
                index++;
            }
        }
        else if(inp_symb == 'q')                                                                        //обрабатываем выход из цикла ввода
            break;
        else                                                                                            //обрабатываем некорректный ввод
        {
            count--;
            printf("Неверный ввод\n");
        }
        //if(count > 10)   //!!!WARNING!!! при отладке, изменении кода очень советую оставлять данный блок кода, т.к. fork() находится в цикле и некорректная обработка возвращаемого значения
                           //и функции execve может привести к росту кол-ва процессов по экспоненте и очень замедлить систему.
                           //я отлаживал на виртуальной машине и не мог получить доступ к отдельным процессам, а сам образ отказывался отвечать на любые запросы извне
                           //причем даже после перезапуска хост-системы ситуация оставалась неизменной - процесс vmem забирал 90% можности ЦП и где-то столько же памяти
                           //пришлось в безопасном режиме сносить виртуалку, все образы, и заново устанавливать
                           //итог - потраченные 2 часа жизни, куча нервов, и сломанная от злости мышка
        //    return -1;
    }


    return 0;

}

char* to_XX(unsigned int num, char* buffer)                       //небольшая функция преобразования числа в строку формата XX (9 - "09")
{
    buffer[2] = '\0';
    buffer[1] = (num % 10) + '0';
    num /= 10;
    buffer[0] = (num % 10) + '0';
    return buffer;
}