#include "queue.h"

void queue_ctor(queue_t* queue)
{
  queue->top = queue->storage;
}
void queue_dtor(queue_t* queue)
{
  queue->top = queue->storage;
}

int queue_top(queue_t* queue)
{
  return *queue->top;
}

int queue_push(queue_t* queue, int item)
{
  *++queue->top = item;
}

int queue_pop(queue_t* queue)
{
  if (!queue_empty(queue))
    queue->top--;
}

bool queue_empty(queue_t* queue)
{
  return queue->top == queue->storage;
}
