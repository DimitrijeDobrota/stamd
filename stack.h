#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

typedef struct
{
  int storage[1000];
  int* top;
} stack_t;

void stack_ctor(stack_t* stack);
void stack_dtor(stack_t* stack);

int stack_top(stack_t* stack);
void  stack_push(stack_t* stack, int item);
void  stack_pop(stack_t* stack);
bool stack_empty(stack_t* stack);

#endif
