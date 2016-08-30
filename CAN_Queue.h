/*
 * CAN_Queue.h
 *
 * Created: 2016-07-01 22:15:16
 *  Author: peter
 */ 

// To Enqueue an integer
void can_queue_Enqueue(can_message_t x);
// To Dequeue an integer.
void can_queue_Dequeue();
can_message_t can_queue_Front();
int can_queue_is_empty();

