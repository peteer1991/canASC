/*
 * scheduler.h
 *
 * Created: 2016-12-10 01:06:07
 *  Author: peter
 */ 

// a task "type"
// pointer to a void function with no arguments
typedef void (*task_t)(void);

// initialises the task list
void initScheduler(void);

// adds a new task to the task list
// scans through the list and
// places the new task data where
// it finds free space
void addTask(uint8_t id, task_t task, uint16_t period);


// remove task from task list
// note STOPPED is equivalent
// to removing a task
void deleteTask(uint8_t id);

// gets the task status
// returns ERROR if id is invalid
uint8_t getTaskStatus(uint8_t id);

// dispatches tasks when they are ready to run
void dispatchTasks(void);



void scedular_setup();

int main_test(void);