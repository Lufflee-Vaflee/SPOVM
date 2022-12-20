
#include "child.h"
#include <stdio.h>
#include <memory.h>
#include <signal.h>
#include <sys/time.h>
#include "glob_set.h"
#include "unistd.h"

#define ALARM_TMS 15000
#define ITER_COUNT 101

char stat_str[100];
int stat_size;
int current_char;

bool interrapted;

struct twinki
{
    int i;
    int j;
} twin;
int stats[4];

struct itimerval _interval;

void form_out_string(){
stat_size = sprintf(stat_str, "%i %i %i %i %i %i\n", getppid(), getpid(), stats[0],stats[1],stats[2],stats[3]);
}

void child_alarm_handler(int signum)
{
    interrapted = true;
    stats[twin.i + twin.j * 2]++; //итерирует номер комбинации(00 = 0, 10 = 1, 01 = 2, 11 = 3)
    form_out_string();
}

void child_usrsig_handler(int sig, siginfo_t *info, void *context)
{
    if (sig == SIGUSR1)
    {
        if(current_char<stat_size)
        {
            putchar(stat_str[current_char]);//Вывод осуществляется посимвольно
            current_char++;
            kill(getppid(), SIGUSR1);//Запрос на вывод следующего символа
        } else{
            kill(getppid(), SIGUSR2);//Информирует что все символы были выведены
            current_char = 0;
        }
    }
    else
    {
        current_char = 0;
    }
}

void init_child()
{
    //тоже самое что и в основном файле, только еще таймеры добавились
    current_char = 0;
    interrapted = false;
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = child_usrsig_handler;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    signal(SIGALRM, child_alarm_handler);
    memset(&twin, 0, sizeof(twin));
    memset(&stats, 0, sizeof(stats));
    timerclear(&_interval.it_interval); //очищаем инфу интервала
    timerclear(&_interval.it_value); //очищаем инфу таймера
    
    _interval.it_value.tv_usec = ALARM_TMS; //сетаем ALARM_TMS микросекунд(это для будильника)
    
}

void child_process()
{
    init_child();
    while (true)
    {
        for (int i = 0; i < ITER_COUNT; i++)
        {
            setitimer(ITIMER_REAL, &_interval, NULL);//сетаем будильник(дед не указывал в задании как, так что будет так)
            while (!interrapted)
            {
                twin.i = !twin.i;
                                 //наш неатомарный доступ ебашится здесь
                twin.j = !twin.j;
            }
            interrapted = false;
        }
        memset(&stats, 0, sizeof(stats)); // очищаю просто чтобы дохуя циферок не было, это можно убрать
        if(!current_char)
            kill(getppid(), SIGUSR1);
    }
}