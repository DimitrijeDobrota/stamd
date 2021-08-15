#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "line.h"

char* text;
char* lineEnd;

char line[1000] = "\0";
char linebuffer[100] = "\0";
char* bufferEnd;
int indent;

void parseDocument(FILE* fp, void (*callback)(void))
{
  while (hasMoreLines(fp))
  {
    readLine(fp);
    if (line[0] == '\0') continue;
    splitLine();
    callback();
  }

  return;
}

bool hasMoreLines(FILE* fp)
{
  int a;
  if ((a = fgetc(fp)) != EOF)
  {
    ungetc(a, fp);
    return true;
  }
  return false;
}

void readLine(FILE* fp)
{
  char* p = line;
  int c;

  while ((c = fgetc(fp)) != '\n')
    * p++ = c;

  *p = '\0';
  lineEnd = p;
}

void splitLine()
{
  char *p = line;
  char *b = linebuffer;   // save potential line tag

  while (isspace(*p) && *p != '\0')
    p++;

  indent = p - line;

  /* while (*p != ' ' && *p != '\0') */
  while (!isspace(*p) && *p != '\0')
    *b++ = *p++;
  *b = '\0';
  bufferEnd = b - 1;
  text = ++p;
}
