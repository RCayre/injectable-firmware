#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

typedef struct MessageQueueElement MessageQueueElement;
typedef struct MessageQueueElement {
	Message *message;
	MessageQueueElement *nextElement;
} MessageQueueElement;

typedef struct MessageQueue {
	size_t size;
	MessageQueueElement *firstElement;
	MessageQueueElement *lastElement;
} MessageQueue;

#endif
