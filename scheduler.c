/*@Student name: Phi Long Nguyen
 Student ID: 19247171*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include "queue.h"
#include "scheduler.h"
#include "generator.h"

/*FIELDS*/
/*FIFO queue*/
Queue* readyQueue;
/*2 mutexes lock: 1 for task and cpu, 1 for simulation log
  2 condition variables: 1 to control the empty slots, 1 to control the full slot*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t con_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t con_full = PTHREAD_COND_INITIALIZER;
/*cpus' shared data */
int num_task = 0;
double total_waiting_time = 0;
double total_turnaround_time = 0;
/*shared logfile to record processes*/
FILE* log_file;
/*condition if more tasks in RQ and task_file: false - more/ true - no more*/
int task_terminated = false;
/*end of fields*/

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Command-line parameter invalid\n" 
                "Please follow format: ./(executable file) (task_file) (num_task)\n");
    }
    else
    {
        /*number of tasks*/
        int m = atoi(argv[2]); 
        /*initialize READY queue (capacity 10)*/
        readyQueue = initQueue(10);
        /*create waiting list*/
        generateTaskFile(m,argv[1]);

        /*initialize cpu1 cpu2 cpu3*/
        CPU *cpu1,*cpu2,*cpu3; 
        cpu1 = (CPU*) calloc(1,sizeof(CPU));   
        cpu1->id = 1;    
        cpu2 = (CPU*) calloc(1,sizeof(CPU));  
        cpu2->id = 2;   
        cpu3 = (CPU*) calloc(1,sizeof(CPU));   
        cpu3->id = 3; 
        /*1 task thread and 3 cpu threads run concurrently*/
        pthread_t task_thread;
        pthread_create(&task_thread,NULL,task,argv[1]);/*pass the file name to task()*/
        pthread_t cpu_1;
        pthread_create(&cpu_1,NULL,cpu,cpu1);
        pthread_t cpu_2;
        pthread_create(&cpu_2,NULL,cpu,cpu2);
        pthread_t cpu_3;
        pthread_create(&cpu_3,NULL,cpu,cpu3);

        /*terminate the threads*/
        pthread_join(cpu_1,NULL);
        pthread_join(cpu_2,NULL);
        pthread_join(cpu_3,NULL);
        pthread_join(task_thread,NULL);

        /*Summarize performance of task() and cpu()*/
        pthread_mutex_lock(&mutex_log);
        log_file = fopen("Simulation_log","a");
        fprintf(log_file,"Number of tasks: %d\n",num_task);
        fprintf(log_file,"Average waiting time: %.f seconds\n", (total_waiting_time/num_task));
        fprintf(log_file,"Average turnaround time: %.f seconds\n",(total_turnaround_time/num_task));
        fclose(log_file);
        pthread_mutex_unlock(&mutex_log);

        /*free section*/
        free(cpu1);
        free(cpu2);
        free(cpu3);
        freeQueue(readyQueue);
        pthread_mutex_destroy(&mutex);
        pthread_mutex_destroy(&mutex_log);
        pthread_cond_destroy(&con_empty);
        pthread_cond_destroy(&con_full);
    }
    return 0;//hasnt free newtask read in
}

/*taking tasks from task_file (WAITING queue) put into READY Queue*/
void* task(void* arg)
{
    //record field
    char arriveString[9]; //(format 18:17:54 + \0)
    char terminateString[9]; //(format 18:17:54 + \0)
    int task_count = 0;
    time_t terminateTime;

    //task field
    FILE* file = fopen(arg,"r");
    int condition = 0;
    Task* task1, *task2;

    if(file == NULL)
    {
        perror("Error Opening!");
    }else   
    {
    /*start getting task from task_file*/
        while(condition != EOF)
        {
            /*read 2 lines at the time
            PRODUCE 2 ITEMS*/
            task1 = (Task*)calloc(1, sizeof(Task));
            condition = fscanf(file,"%d %d\n",&task1->task, &task1->cpu_burst);
            task2 = (Task*)calloc(1, sizeof(Task));
            condition = fscanf(file,"%d %d\n",&task2->task, &task2->cpu_burst); 
            //error format check
            if(condition != 2 && condition != EOF)
            {
                printf("FILE has INCORRECT PARAMETER text\n");
                /*free incorrect data*/
                free(task1);
                free(task2);
            }
            else
            {
                /*add to BUFFER Section:*/
                    /*PUT 2 Items into READY QUEUE
                    block other processes while first process is interacting with READY QUEUE*/
                    pthread_mutex_lock(&mutex);
                    /*free empty task*/
                    if(task1->task <= 0 || task1->cpu_burst < 0){
                        free(task1);
                    }else{
                            /*block task() when READY QUEUE is full*/
                            while(isFull(readyQueue)){
                                pthread_cond_wait(&con_empty,&mutex);
                            }  
                            /*add task to READY QUEUE -> set arrival time for task*/
                                enqueue(readyQueue, task1);
                                task1->arrive_time = time(NULL);//second (in decimal)
                            /*tell cpu() that RQ has item to be executed*/
                            pthread_cond_signal(&con_full);
                            task_count++;

                            /* record arrivial time for task1*/
                            pthread_mutex_lock(&mutex_log);
                                log_file = fopen("Simulation_log","a");
                                strftime(arriveString,9,"%H:%M:%S",localtime(&task1->arrive_time));
                                fprintf(log_file,"task %d: %d second\n",task1->task, task1->cpu_burst);
                                fprintf(log_file,"Arrival Time: %s\n",arriveString);
                                fprintf(log_file,"---------->READY QUEUE (%d/%d)---------\n",readyQueue->numItem,readyQueue->capacity);
                                fclose(log_file);
                            pthread_mutex_unlock(&mutex_log);
                    }
                    /*similar to adding task1 to QUEUE*/
                    /*free empty task*/
                    if(task2->task <= 0 || task2->cpu_burst < 0){
                        free(task2);
                    }else{
                            /*block task() when READY QUEUE is full*/
                            while(isFull(readyQueue)){
                                pthread_cond_wait(&con_empty,&mutex);
                            }
                            /*add task to READY QUEUE -> set arrival time for task*/
                            enqueue(readyQueue, task2);
                            task2->arrive_time = time(NULL);
                            /*tell cpu() that RQ has item to be executed*/
                            pthread_cond_signal(&con_full);
                            task_count++;
                            /* record arrivial time for task2*/
                            pthread_mutex_lock(&mutex_log);
                                log_file = fopen("Simulation_log","a");
                                strftime(arriveString,9,"%H:%M:%S",localtime(&task2->arrive_time));
                                fprintf(log_file,"task %d: %d second\n",task2->task, task2->cpu_burst);
                                fprintf(log_file,"Arrival Time: %s\n",arriveString);
                                fprintf(log_file,"----------->READY QUEUE (%d/%d)----------\n",readyQueue->numItem,readyQueue->capacity);
                                fclose(log_file);
                            pthread_mutex_unlock(&mutex_log);
                    }   
                /*tell other processes that this process is finished interacting with RQ*/
                pthread_mutex_unlock(&mutex);
            }
        }/*exit while loop - reach EOF*/

        /*file is empty error catch*/
        if(ftell(file) == 0)
        {
            printf("file is empty\n");
        }  
        /*when reach EOF task_file then terminate task thread
        set task_terminated <- true, let cpu() know that no more task waiting from file*/
        task_terminated = true;  
        terminateTime = time(NULL);
        /*release all threads that are blocked because waiting for new arrival task  RQ
        because after task thread is terminated
        cpu() shouldn't wait for RQ's full slot but terminate*/
        pthread_cond_broadcast(&con_full); 

        /*record task thread termination*/
        pthread_mutex_lock(&mutex_log);
            log_file = fopen("Simulation_log","a");
            strftime(terminateString,9,"%H:%M:%S",localtime(&terminateTime));
            fprintf(log_file,"\n%s\n","-----------------------------------------");
            fprintf(log_file,"|Number of tasks put into ReadyQueue: %d|\n",task_count);
            fprintf(log_file,"|Task Terminate at Time: %s       |\n",terminateString);
            fprintf(log_file,"%s\n","----------TASK THREAD TERMINATED---------");
            fclose(log_file);
        pthread_mutex_unlock(&mutex_log);
    }
    fclose(file);
    pthread_exit(0);
    return NULL;
}

/*taking task from READY queue and execute it 1 by 1*/
void* cpu(void* arg)
{
    char arriving [9];//(format 18:17:54 + \0)
    
    //cpu field
    Task* servicing_task;
    CPU* cpu = arg;
    int cpu_terminate = false;//terminate condition

    while(cpu_terminate == false)
    {
        /*block other processes while this cpu() is taking task from RQ */
        pthread_mutex_lock(&mutex);
           
            /*block cpu() while RQ is empty*/
            while(isEmpty(readyQueue)){
                /*wait for new tasks arrive to the empty RQ*/
                pthread_cond_wait(&con_full,&mutex);
                /*terminate cpu when no more tasks 
                this will terminate any unecessary cpu*/
                if(task_terminated == true && isEmpty(readyQueue))
                {   
                    pthread_mutex_lock(&mutex_log);
                        log_file = fopen("Simulation_log","a");
                        fprintf(log_file,"\nCPU-%d terminates after servicing %d tasks\n",cpu->id,cpu->num_task_served);
                        fprintf(log_file,"%s\n","----------------------------------------");
                        fclose(log_file);
                    pthread_mutex_unlock(&mutex_log);
                    /*release the mutex before terminate a thread*/ 
                    pthread_mutex_unlock(&mutex);
                    pthread_exit(0);
                }
            }
            /*take task from RQ and set response time for that task*/
            servicing_task = dequeue(readyQueue);
            servicing_task->service_time = time(NULL);//first response
            strftime(arriving,9,"%H:%M:%S",localtime(&servicing_task->arrive_time));
            /*record service time*/
            pthread_mutex_lock(&mutex_log);
                log_file = fopen("Simulation_log","a");
                fprintf(log_file,"\n----------Statistics for CPU-%d----------\n",cpu->id);
                fprintf(log_file,"task %d\n",servicing_task->task);
                fprintf(log_file,"Arrival Time: %s\n",arriving);
                
                strftime(cpu->timeString,9,"%H:%M:%S",localtime(&servicing_task->service_time));
                fprintf(log_file,"ServiceTime Time: %s\n",cpu->timeString);
                fprintf(log_file,"-----------READY QUEUE (%d/%d)---------->\n",readyQueue->numItem,readyQueue->capacity);
                fclose(log_file);
            pthread_mutex_unlock(&mutex_log);
            /*tell task() that cpu() get 1 task from RQ
            so RQ has a empty slot for task() to add more tasks*/
            pthread_cond_signal(&con_empty);
        /*tell other processes that this cpu() done interacting, others can continue*/
        pthread_mutex_unlock(&mutex);

        /*CONSUME section: */
        sleep(servicing_task->cpu_burst);
        /*set finish time when cpu executed the task*/
        servicing_task->completion_time = time(NULL);
        strftime(cpu->timeString,9,"%H:%M:%S",localtime(&servicing_task->completion_time));

        /*record Completion time*/
        pthread_mutex_lock(&mutex_log);
            log_file = fopen("Simulation_log","a");
            fprintf(log_file,"\n----------Statistics for CPU-%d----------\n",cpu->id);
            fprintf(log_file,"task %d\n",servicing_task->task);
            fprintf(log_file,"Arrival Time: %s\n",arriving);
            fprintf(log_file,"Completion Time: %s\n",cpu->timeString);
            fclose(log_file);
        pthread_mutex_unlock(&mutex_log);

        /*calculate waiting time and turnaround time each task -> add to total*/
        double turnaround_time = difftime(servicing_task->completion_time,servicing_task->arrive_time);
        double wait_time = difftime(servicing_task->service_time,servicing_task->arrive_time);

        /*add to shared total time */
        total_turnaround_time += turnaround_time;
        total_waiting_time += wait_time;

        /*finish CONSUME */
        cpu->num_task_served += 1;
        num_task += 1;//total task

        /*terminate condition: when no more task in RQ and task_file
        this will terminate cpu that finish its job */
        if(isEmpty(readyQueue) && task_terminated == true)
        {
            cpu_terminate = true;
            pthread_cond_broadcast(&con_full);
            pthread_mutex_lock(&mutex_log);
                log_file = fopen("Simulation_log","a");
                fprintf(log_file,"\nCPU-%d terminates after servicing %d tasks\n",cpu->id,cpu->num_task_served);
                fprintf(log_file,"%s\n","----------------------------------------");
                fclose(log_file);
            pthread_mutex_unlock(&mutex_log);
        }
        /*free the task after serviced*/
        free(servicing_task);
    }/*end of while loop*/
    pthread_exit(0);
    return NULL;
}