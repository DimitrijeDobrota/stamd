#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct
{
  int storage[1000];
  int* top;
} queue_t;

void queue_ctor(queue_t* queue);
void queue_dtor(queue_t* queue);

int queue_top(queue_t* queue);
int queue_push(queue_t* queue, int item);
int queue_pop(queue_t* queue);
bool queue_empty(queue_t* queue);

#endif
