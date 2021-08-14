#include <stdio.h>

#include "queue.h"
#include "text.h"
#include "lineQueue.h"

queue_t lineQueue;

extern char* text;
extern char* lineEnd;

void line_push(tag_t tag, bool parse)
{
  if (parse)
  {
    printf("<%s>", tag_n[tag]);
    parseText(text, lineEnd);
  }
  else
  {
    printf("<%s>\n", tag_n[tag]);
  }
  queue_push(&lineQueue, tag);
}

void line_pop()
{
  tag_t tag = queue_top(&lineQueue);
  queue_pop(&lineQueue);
  printf("</%s>\n", tag_n[tag]);
}

bool line_empty(){
  return queue_empty(&lineQueue);
}
