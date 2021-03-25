/*@Student name: Phi Long Nguyen
 Student ID: 19247171*/
typedef struct{
    int task;
    int cpu_burst;
    time_t arrive_time;
    time_t service_time;
    time_t completion_time;
}Task;
typedef struct{
    int id;
    int num_task_served;
    char timeString[9];
}CPU;
void* (*MyTask)(void*);

void* task(void*);
void* cpu(void*);