#ifndef LINE_QUEUE_H
#define LINE_QUEUE_H

typedef enum tag_t {ul, ol, li, blockquote, p, code, pre} tag_t;
static char* tag_n[] = {"ul", "ol", "li", "blockquote", "p", "code", "pre"};

void line_push(tag_t tag, bool parse);
void line_pop();
bool line_empty();

#endif
