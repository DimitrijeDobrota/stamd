#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "line.h"
#include "queue.h"
#include "lineQueue.h"
#include "text.h"

extern queue_t lineQueue;
enum tag_t;

extern FILE* md;

extern char* text;
extern char* lineEnd;

extern char line[];
extern char linebuffer[];
extern char* bufferEnd;
extern int indent;

void parseArticleLine(void);

int startList(tag_t list_tag, char currentSign, int currentIndent);
int startBlockquote(int currentIndent);
int getBlockquoteLen(void);

void startCodeIndent(int currentIndent);
void startCodeBlock(void);
void startParagraph(void);

void parseArticle(void)
{
  queue_ctor(&lineQueue);
  parseDocument(md, parseArticleLine);
  queue_dtor(&lineQueue);
}

void parseArticleLine()
{

  if (*linebuffer == '\0') return;

  static const char* headingb[] = {"#", "##", "###", "####", "#####", "######"};
  static const char* headingt[] = {"h1", "h2", "h3", "h4", "h5", "h6"};

  for (int i = 0; i < 6; i++)
  {
    if (strcmp(linebuffer, headingb[i]) == 0)
    {
      printf("</section>\n<section>\n<%s>", headingt[i]);
      parseText(text, lineEnd);
      printf("</%s>\n", headingt[i]);
      return;
    }
  }

  if (strcmp("---", linebuffer) == 0 || strcmp("___", linebuffer) == 0 || strcmp("***", linebuffer) == 0) printf("<hr>\n");
  else if (strcmp(linebuffer, "```") == 0) startCodeBlock();
  else if (*bufferEnd == '-' || *bufferEnd == '+' || *bufferEnd == '*') startList(ul, *bufferEnd, indent);
  else if (*bufferEnd == '.' && isdigit(*(bufferEnd - 1))) startList(ol, *bufferEnd, indent);
  else if (*linebuffer == '>') startBlockquote(getBlockquoteLen());
  else if (indent >= 4) startCodeIndent(indent);
  else startParagraph();

  while (!line_empty())
    line_pop();

}

void startParagraph()
{
  text = line;
  line_push(p, true);
  while (hasMoreLines(md))
  {
    readLine(md);
    splitLine();
    if (*linebuffer == '\0') return;
    printf("\n");
    parseText(line, lineEnd);
  }

}

void startCodeIndent(int currentIndent)
{
  line_push(pre, false);
  line_push(code, false);
  while (hasMoreLines(md))
  {
    printf("%s\n", line + currentIndent); // Nullify the indend when code block is started
    readLine(md);
    splitLine();
    if (*linebuffer == '\0') return;
  }
}

void startCodeBlock()
{
  line_push(pre, false);
  line_push(code, false);
  while (hasMoreLines(md))
  {
    readLine(md);
    splitLine();
    if (strcmp(linebuffer, "```") == 0) return;
    printf("%s\n", line);   // print the whole line
  }
}

int startBlockquote(int currentIndent)
{
  line_push(blockquote, false);
  line_push(p, true);
  while (hasMoreLines(md))
  {
    readLine(md);
    splitLine();
    indent = getBlockquoteLen();
    if (*linebuffer == '>')
    {
      line_pop();

      if (currentIndent < indent)
      {
        int blankLine = startBlockquote(indent);
        if (blankLine ) return 1;
      }

      if (currentIndent > indent)
      {
        line_pop();
        return 0;
      }

      line_push(p, true);
    }
    else if (line[0] == '\0') return 1;
    else parseText(line, lineEnd);
  }
  return 0;
}

int getBlockquoteLen()
{
  char* p = linebuffer;
  while (true)
  {
    if (*p == ' ') continue;
    if (*p != '>') break;
    p++;
  }

  return p - linebuffer;
}

// Function to start new level of indend with UL
// If blank line is reached 1 is returned, to end the recursion
tag_t list_tag;
char list_sign;
int startList(tag_t currentTag, char currentSign, int currentIndent)
{
  line_push(currentTag, false);
  line_push(li, true);
  while (hasMoreLines(md))
  {
    readLine(md);
    splitLine();
    list_tag = ul;
    list_sign = *bufferEnd;
    if (list_sign == '-' || list_sign == '+' || list_sign == '*' || list_sign == '.' && isdigit(*(bufferEnd - 1)))
    {
      if (list_sign == '.') list_tag = ol;

      if (currentIndent < indent)
      {
        // We are too low. Start new list with a bigger indent
        // This new list is part of the previous list item, that will get closed after
        printf("\n");
        int blankLine = startList(list_tag, list_sign, indent);
        line_pop();
        if (blankLine ) return 1; // blank line must end all Uls, propagate throught recursion
      }
      else
      {
        // Since there is no new level of indentation close the LI
        line_pop();
      }

      if (currentIndent > indent)
      {
        // We need to return one step downwards
        // Close the current LI
        line_pop();

        // Curent line will be handled by the lower levels
        return 0;
      }

      // Allow the same inded level to have a different type of a list
      if (currentTag != list_tag)
      {
        line_pop();
        line_push(list_tag, false);
        currentTag = list_tag;
      }
      // Switch in the sign means new list
      else if (currentSign != list_sign)
      {
        line_pop();
        line_push(list_tag, false);
        currentSign = list_sign;
      }

      // We are at the good indent level
      // Start a new list item
      line_push(li, true);
    }
    else if (line[0] == '\0') return 1;              // blank line must end all Uls
    else parseText(line + currentIndent + 1, lineEnd);   // Current LI spans multiple lines. Just print the text of subsiquent lines
  }
  return 0;
}
