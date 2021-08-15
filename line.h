#ifndef LINE_H
#define LINE_H

#include <stdlib.h>
#include <stdbool.h>

void parseDocument(FILE* fp, void (*callback)(void));
bool hasMoreLines(FILE* fp);
void readLine(FILE* fp);
void splitLine(void);

#endif
