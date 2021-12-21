#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
void parseConfig(void);

void writeHeader();
void writeFooter(void);
void writeCategories(char *c);

void writeArticleItem(char*);

#endif
