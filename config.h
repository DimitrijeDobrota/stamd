#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

void parseConfig(void);

void writeHeader(bool index);
void writeFooter(void);

void writeArticleItem(char*);

#endif
