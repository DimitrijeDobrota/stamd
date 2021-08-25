#include "stack.h"

void stack_ctor(stack_t* stack)
{
  stack->top = stack->storage;
}
void stack_dtor(stack_t* stack)
{
  stack->top = stack->storage;
}

int stack_top(stack_t* stack)
{
  return *stack->top;
}

void  stack_push(stack_t* stack, int item)
{
  *++stack->top = item;
}

void stack_pop(stack_t* stack)
{
  if (!stack_empty(stack))
    stack->top--;
}

bool stack_empty(stack_t* stack)
{
  return stack->top == stack->storage;
}
