#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "text.h"

extern FILE* html;

typedef char*(*search)(char, char, char*, char*);
typedef void(*print)(char*, char*);

char* searchOne(char c, char foo, char* start, char* end);
char* searchTwo(char c1, char c2, char* start, char* end);

void tagSearchLink(char* current, char* end, bool image);
void tagSearchCode(char* current, char* end);
void tagSearchMatch(char* start, char* end, char* tag, search searchCall, print printCall);


char* control; // before returning from any tagSearch control needs to be set if the search was successful
void parseText(char* current, char* end)
{
  for (; current < end; current++)
  {
    control = current;
    if (*current == *(current + 1))    // handle tags that require double prefix
    {
      if (*current == '*' || *current == '_') tagSearchMatch(current + 2, end, "strong", &searchTwo, &parseText);
      else if (*current == '~') tagSearchMatch(current + 2, end, "s", &searchTwo, &parseText);
    }
    else     // handle tags that require a single prefix
    {
      if (*current == '*' || *current == '_') tagSearchMatch(current + 1, end, "em", &searchOne, &parseText);
      else if (*current == '`') tagSearchMatch(current + 1, end, "code", &searchOne, &printEscaped);
      else if (*current == '[') tagSearchLink(current, end, false);
      else if (*current == '!') tagSearchLink(current, end, true);
    }

    if (control != current) current = control;
    else fprintf(html, "%c", *current);
  }
}

void tagSearchLink(char* current, char* end, bool image)
{
  char* start = current;
  if (image) start++;
  if (*start != '[') return;

  char* p1 = searchTwo(']', '(', start, end);
  if (p1 == end) return;

  char* p2 = searchOne(')', ' ', p1 + 2, end);
  if (p2 == end) return;

  *p1 = '\0';   // start->p1 = text
  *p2 = '\0';   // p1->p2 = link if there is no blank

  p1 += 2; // step over "]("
  start += 1; // step over "["


  char* blank = strchr(p1 + 2, ' '); // Detect title

  if (blank == NULL || blank > p2)
  {
    // there is no title attribute
    if (image)
      fprintf(html, "<img src=\"%s\" alt=\"%s\">", p1, start);
    else
      fprintf(html, "<a href=\"%s\">%s</a>", p1, start);
  }
  else
  {
    // there is a title attribute
    *blank = '\0';  // p2->blank = title
    blank += 1; // get to the quoted part
    if (image)
      fprintf(html, "<img src=\"%s\" alt=\"%s\" title=%s>", p1, start, blank);
    else
      fprintf(html, "<a href=\"%s\" title=%s>%s</a>", p1, blank, start);
  }

  control = p2;
}

void tagSearchMatch(char* start, char* end, char* tag, search searchCall, print printCall)
{
  char c = *(start - 1);
  char* p = searchCall(c, c, start, end);
  if (p == end) return;
  fprintf(html, "<%s>", tag);
  printCall(start, p);
  fprintf(html, "</%s>", tag);
  control = p + (searchCall == searchTwo);
}

char* searchOne(char c, char foo, char* start, char* end)
{
  char* p = strchr(start, c);
  return (p != NULL && p < end) ? p : end;
}

char* searchTwo(char c1, char c2, char* start, char* end)
{
  char* p = strchr(start, c1);
  return (p != NULL && *(p + 1) == c2 && p < end) ? p : end;
}

void printEscaped(char* start, char* end)
{
  for (; start < end; start++)
    if (*start == '<')
      fprintf(html, "&lt");
    else if (*start == '>')
      fprintf(html, "&gt");
    else
      fputc(*start, html);
}
