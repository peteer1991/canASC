/*
 * scheduler.c
 *
 * Created: 2016-12-10 00:38:00
 *  Author: peter
 */ 
#include<avr/interrupt.h>
#include<inttypes.h>
#include "scheduler.h"

#define MAX_TASKS (10)

// task states
#define RUNNABLE (0x00)
#define RUNNING  (0x01)
#define STOPPED  (0x02)
#define ERROR    (0x03)




// basic task control block (TCB)
typedef struct __tcb_t
{
	uint8_t id; // task ID
	task_t task; // pointer to the task
	// delay before execution
	uint16_t delay, period;
	uint8_t status; // status of task
} tcb_t;



// prototypes of tasks
void Task1(void);
void Task2(void);

// the task list
tcb_t task_list[MAX_TASKS];

// keeps track of number of timer interrupts
uint16_t count = 0;

// scheduler function definitions

// initialises the task list
void initScheduler(void)
{
	for(uint8_t i=0; i<MAX_TASKS; i++)
	{
		task_list[i].id = 0;
		task_list[i].task = (task_t)0x00;
		task_list[i].delay = 0;
		task_list[i].period = 0;
		task_list[i].status = STOPPED;
	}
}

// adds a new task to the task list
// scans through the list and
// places the new task data where
// it finds free space
void addTask(uint8_t id, task_t task, uint16_t period)
{
	uint8_t idx = 0, done = 0x00;
	while( idx < MAX_TASKS )
	{
		if( task_list[idx].status == STOPPED )
		{
			task_list[idx].id = id;
			task_list[idx].task = task;
			task_list[idx].delay = period;
			task_list[idx].period = period;
			task_list[idx].status = RUNNABLE;
			done = 0x01;
		}
		if( done ) break;
		idx++;
	}

}

// remove task from task list
// note STOPPED is equivalent
// to removing a task
void deleteTask(uint8_t id)
{
	for(uint8_t i=0;i<MAX_TASKS;i++)
	{
		if( task_list[i].id == id )
		{
			task_list[i].status = STOPPED;
			break;
		}
	}
}

// gets the task status
// returns ERROR if id is invalid
uint8_t getTaskStatus(uint8_t id)
{
	for(uint8_t i=0;i<MAX_TASKS;i++)
	{
		if( task_list[i].id == id )
		return task_list[i].status;
	}
	return ERROR;
}

// dispatches tasks when they are ready to run
void dispatchTasks(void)
{
	for(uint8_t i=0;i<MAX_TASKS;i++)
	{
		// check for a valid task ready to run
		if( !task_list[i].delay &&
		task_list[i].status == RUNNABLE )
		{
			// task is now running
			task_list[i].status = RUNNING;
			// call the task
			(*task_list[i].task)();

			// reset the delay
			task_list[i].delay =
			task_list[i].period;
			// task is runnable again
			task_list[i].status = RUNNABLE;
		}
	}
}

// generates a "tick"
// each tick 50ms apart
ISR(TCC1_OVF_vect)
{
	count ++;
	if( count == 392 )
	{
		count = 0;
		// cycle through available tasks
		for(uint8_t i=0;i<MAX_TASKS;i++)
		{
			if( task_list[i].status == RUNNABLE )
			task_list[i].delay--;
		}
	}
}

// Task definitions

void Task1(void)
{
	printf("task1\n");
}


void Task2(void)
{
	printf("task2\n");
}

void scedular_setup()
{
		TCC1.CNT = 0;// Zeroise count
		//TCC1.PER = 8; //Period
		TCC1.PER = 8; //Period
		TCC1.CTRLA = TC_CLKSEL_DIV1024_gc; //Divider
		TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc; //Liow level interrupt
		TCC1.INTFLAGS = 0x01; // clear any initial interrupt flags
		TCC1.CTRLB = TC_WGMODE_NORMAL_gc; // Normal operation
		// set up the task list		
		initScheduler();
		sei();
}

