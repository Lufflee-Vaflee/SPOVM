#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory.h>

#include "glob_set.h"
#include <signal.h>

#include "child.h"


#define MAX_PROCESS_COUNT 100

/*
Принцип очень прост.

РЕБЕНОК ВЫВОДИТ ТЕКСТ ПО ИСТЕЧЕНИЮ ВРЕМЕНИ

Ребенок ждет своего времени на отправку, отправляет сигнал 1,
если он не замьючен и никто из детей не выводит свою инфу, то папа отвечает ему 1.
Получая сигнал 1 сынок, если строка еще не выведена полностью,
выводит символ и отправляет папе разрешение на след символ.
Папа видит что это именно тот сынок, который должен сейчас выводить так что отвечает 1 сыночку.
Как только все символы будут выведены, сыночек пошлет папе сигнал 2.
Папа поменяет глобальные переменные на свободу вывода и сможет принять вывод от других сыночков.

РЕБЕНОК ПОЛУЧАЕТ ЗАПРОС НА ВЫВОД ИНФОРМАЦИИ

Папочка отправляет сыночку сигнал 1, и сына действует так же, как будто он сам захотел инфу вывести.

ВАЖНО но как бы и похуй

если мутнуть сына в момент вывода инфы, он хуй забьет и все равно до конца свой текст выведет.
это в любом случае ебаные наносекунды, так что значения не имеет

*/

typedef struct global_state
{
    bool allmuted; //эта поебота только чтобы новых детей глухимим можно было делать, когда все процы замьючены
    bool output_child; // флаг того выводит ли в данный момент какой либо ребенок свой текст
    pid_t output_child_pid; //пид этого ребеночка
} global_state;

typedef struct process
{
    pid_t pid;
    bool muted;
} process;

process proc_arr[MAX_PROCESS_COUNT];
unsigned int proc_cnt;

global_state g_state;

int read_command(char *c, unsigned int *id)
//получает пользовательский ввод
{
    char buff[256];
    if(!fgets(buff, 256, stdin))
        return 0;
    
    (*c) = buff[0];
    if (buff[1] >= '0' && buff[1] <= '9')
    {
        (*id) = atoi(buff + 1);
        return 2;
    }
    return 1;
}

bool push_child(pid_t pid)
//добавляет элемент в стек дочерних процессов
{
    if (proc_cnt > MAX_PROCESS_COUNT - 1)
        return false;
    proc_arr[proc_cnt].pid = pid;
    proc_arr[proc_cnt].muted = g_state.allmuted; //параметр вывода статистики инициализируется глобальным состоянием вывода 
    proc_cnt++;
    return true;
}

bool pop_child()
//удаляет последний элемент из стека дочерних процессов
{
    if (proc_cnt == 0)
        return false;
    proc_cnt--;
    return true;
}

void foreach_child(void (*fu)(process *))
//проходит весь стек дочерних процессов отправляя каждый элемент в callback-функцию
{
    for (int i = 0; i < proc_cnt; i++)
        fu(&proc_arr[i]);
}

void mute_child(process *proc)
//запрещает выводить статистику дочернему процессу proc
{
    proc->muted = true;
}

void unmute_child(process *proc)
//разрешает выводить статистику дочернему процессу proc
{
    proc->muted = false;
}

process *get_child(unsigned int i)
//возвращает i-тый элемент стека дочерних процессов
{
    if (i >= proc_cnt)
        return 0;
    return &proc_arr[i];
}

process *head()
//возвращает последний элемент стека дочерних процессов
{
    if (!proc_cnt)
        return 0;
    return &proc_arr[proc_cnt - 1];
}

process *find(pid_t pid)
//поиск дочернего процесса в стеке по pid
{
    for(int i=0;i<proc_cnt;i++)
        if(proc_arr[i].pid==pid) return &proc_arr[i];
    return 0;
}

void kill_all()
//убивает все дочерние процессы и очищает стек дочерних процессов
{
    while (proc_cnt)
    {
        if (kill(head()->pid, SIGKILL) == -1)
            break;
        pop_child();
    }
}

void usrsig_handler(int sig, siginfo_t *info, void *context)
//обработчик сигналов от дочерних процессов
{
    pid_t call_pid = info->si_pid;
    if (sig == SIGUSR1)
    {
        if(g_state.output_child){
            if(g_state.output_child_pid==call_pid)
                kill(call_pid, SIGUSR1);
            else
                kill(call_pid, SIGUSR2);
        } else{
            process* temp = find(call_pid);
            if(temp && !temp->muted)
            {
                g_state.output_child = true;
                g_state.output_child_pid = call_pid;
                kill(call_pid, SIGUSR1);
            }
        }
    }
    else
    {
        if(g_state.output_child){
            if(g_state.output_child_pid==call_pid)
                g_state.output_child = false;
        }
    }
}

void alarm_handler(int signum)
//обработчик сигналов будильника, используется только для p<num>
{
    foreach_child(unmute_child);
    g_state.allmuted = false;
}

void init()
//Инициализирует обработчики и глобальные переменные
{
    //Почему для sigusr не signal? потому что мне нужно было инфу передавать(pid), а signal так не умеет

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO; // флаг для нескольких параметров в обработчике
    sa.sa_sigaction = usrsig_handler;
    sigaction(SIGUSR1, &sa, NULL); //биндим собственно два наших сигнала в одну топку
    sigaction(SIGUSR2, &sa, NULL);
    signal(SIGALRM, alarm_handler);
    proc_cnt = 0;
    memset(&g_state, 0, sizeof(global_state));
}

int main(int argc, char **argv)
{
    init();
    
    while (true)
    {
        char c = '\0'; // присвоение нельзя убирать(сломает программу(я не ебу почему))
        unsigned int code;
        int res = read_command(&c, &code);
        if (res == 1)
        {
            if (c == '+')
            {
                pid_t pid = fork();
                if (pid == -1)
                    return -1;
                if (pid == 0)
                {
                    child_process();
                    return 0;
                }
                puts(push_child(pid) ? "Процесс создан" : "Процесс не может быть создан");
            }
            else if (c == '-')
            {
                if (!proc_cnt)
                    puts("Некого убивать");
                else
                {
                    if (kill(head()->pid, SIGKILL) != -1)
                    {
                        pop_child();
                        printf("Дочерний процесс был убит. Осталось процессов: %i\n", proc_cnt);
                    }
                    else
                    {
                        puts("Error: не могу убить ребенка");
                    }
                }
            }
            else if (c == 'k')
            {
                kill_all();
                puts(head() ? "Дети не убиты" : "Все дочерние процессы убиты");
            }
            else if (c == 's')
            {
                foreach_child(mute_child);
                g_state.allmuted = true;
            }
            else if (c == 'g')
            {
                alarm(0);
                alarm_handler(0);
            }
            else if (c == 'q')
            {
                kill_all();
                return 0;
            }
        }
        else
        {
            if (c == 's')
            {
                process *temp = get_child(code);
                if (temp)
                    temp->muted = true;
            }
            else if (c == 'g')
            {
                process *temp = get_child(code);
                if (temp)
                    temp->muted = false;
            }
            else if (c == 'p')
            {
                process *temp = get_child(code);
                if (temp)
                {
                    while(g_state.output_child) usleep(10000); 
                    // кастыль чтобы не сломать вывод ребеночка(шансы сломать реально один к хулиарду, но лучше обезопасить себя)
                        
                    foreach_child(mute_child);
                    temp->muted = false;
                    g_state.allmuted = true;
                    g_state.output_child = true;
                    g_state.output_child_pid = temp->pid;
                    

                    kill(temp->pid, SIGUSR1);
                    alarm(5);
                }
            }
        }
    }
}