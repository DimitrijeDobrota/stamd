#ifndef LINE_H
#define LINE_H

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

typedef enum tag_t {ul, ol, li, blockquote, p, code, pre} tag_t;
static char* tag_n[] = {"ul", "ol", "li", "blockquote", "p", "code", "pre"};

void line_push_tag(tag_t tag);
void line_push_text(tag_t tag);
void line_pop();
bool line_empty();

void parseDocument(FILE* fp, void (*callback)(void));
bool hasMoreLines(FILE* fp);
void readLine(FILE* fp);
void splitLine(void);

#endif
