#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include "sys/stat.h"
#include <fcntl.h>

int child_counter = 0;                                                                      //счетчик дочерних потоков  

#define ENC_TABLE_SIZE 256                                                                  //таблица перекодировки и ее размер
static char enc_table[256];

int child_body(const char* file_name);
int parent_signal_handler();

int main(int argc, char* argv[])
{
    DIR* catalog;
    struct dirent* entry;
    struct stat st;
    int count = 0;
    catalog = opendir(argv[1]);                                                             //открываем каталог
    while((entry = readdir(catalog)) != NULL)                                               //считаем кол-во файлов(предполагается, что каталог пустой, кроме файлов формата f${num})
        count++;
    count -= 2;                                                                             //уменьшаем на 2 ("." и "..")

    for(int i = 0; i < 256; i++)                                                            //заполняем таблицу случайными значениями
        enc_table[i]  = rand() % 256;

    int stat_loc;
    int pid;
    int child_max = count / 10;                                                             //считаем максимальное кол-во дочерних потоков
    int next_file_index = 1;                                                                //счетчик файлов переданных дочерним потокам
    char* file_name = (char*)malloc(sizeof(char) * 37);                                     //строка для формирования полного имени файла
    signal (SIGCHLD, parent_signal_handler);                                                //привязываем сигнал SIGCHILD к обработчику
    while(next_file_index <= count)                                                         //цикл перекодировки файлов потоками
    {
        if(child_counter <= child_max)                                                      //если число потоков меньше допущенного рождаем новый
        {
            switch (fork())
            {
            case -1:
                printf("Ошибка при создании дочернего потока \n" );
                break;
            case 0:                                                                         //вход в дочерних поток
                strcpy(file_name, argv[1]);  
                char buf[10];
                sprintf(buf, "%d", next_file_index);
                strcat(strcat(file_name, "/f"), buf);                                       //формируем имя файла
                return child_body(file_name);                                               //возвращаем результат выполнения тела дочернего потока
                break;
            default:
                signal (SIGCHLD, parent_signal_handler);
                next_file_index++;                                                          //увеличиваем счетчик файлов на 1
                child_counter++;                                                            //увеличиваем счетчик потоков на 1
                break;
            }
        }
    }

    while((pid = wait(&stat_loc)) != -1)                                                    //ожидаем завершения работы оставшихся потоков
    sleep(0);
    return 0;
}

int parent_signal_handler()                                                                 //обработчик сигналов от дочерних потоков, начинает выполняться после смерти дочернего потока
{
    child_counter--;                                                                        //уменьшаем счетчик дочерних потоков
    return 0;
}


int child_body(const char* file_name)
{
    int f;                                                                                      //переменная дескриптора файла
    struct stat st;                                                                             //stat структура для получения информации о файле
    f = open(file_name, O_RDWR);
    printf("Привет, я дочерний процесс %d и я перекодирую файл: %s \n", getpid(), file_name);
    fstat(f, &st);                                                                              //получаем информацию о файле
    char* map = mmap(map, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0);                //создаем отображение файла в память
    for(int i = 0; i < st.st_size; i++)                                                         //перекодируем файл
    {
        map[i] = enc_table[map[i]];                                                             //все изменения в отображаемой памяти будут записываться в файл
        msync(&map[i], sizeof(char), MS_SYNC);                                                  //синхронизируем память и файл
    }
    printf("Привет, я дочерний процесс %d и я закончил перекодировать файл: %s \n", getpid(), file_name);
    munmap(map, st.st_size);                                                                    //закрываем отображаемую память
    close(f);  
    free(map);
    return 0;
}