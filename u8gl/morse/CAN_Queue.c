/*
 * CAN_Queue.c
 *
 * Created: 2016-07-01 21:45:43
 *  Author: peter
 */ 
/* based on 

https://gist.github.com/mycodeschool/7510222
*/

/*This file is a simpel transmit que for handeling can pakages to trasmit */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "can/can.h"
#include "CAN_Queue.h"

// modified for CAN messeges 
struct Node {
	can_message_t data;
	struct Node* next;
};

// Two glboal variables to store address of front and rear nodes.
struct Node* front = NULL;
struct Node* rear = NULL;

// To Enqueue an integer
void can_queue_Enqueue(can_message_t x) {
	struct Node* temp =
	(struct Node*)malloc(sizeof(struct Node));
	temp->data =x;
	temp->next = NULL;
	if(front == NULL && rear == NULL){
		front = rear = temp;
		return;
	}
	rear->next = temp;
	rear = temp;
}

// To Dequeue an integer.
void can_queue_Dequeue() {
	struct Node* temp = front;
	if(front == NULL) {

		return;
	}
	if(front == rear) {
		front = rear = NULL;
	}
	else {
		front = front->next;
	}
	free(temp);
}

can_message_t can_queue_Front() {
	if(front == NULL) {
		
		can_message_t emp;
		emp.msg_id=0xFF;
		return emp;
	}
	return front->data;
}
int can_queue_is_empty()
{
		if(front == NULL) {
			//printf("Queue is empty\n");
			return 1;
		}
		return 0;
}

