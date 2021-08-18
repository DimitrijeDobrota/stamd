#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "text.h"

extern FILE* html;

char* searchOne(char c, char* start, char* end);
char* searchTwo(char c1, char c2, char* start, char* end);

void tagSearchOne(char* current, char* end, char* open, char* close);
void tagSearchTwo(char* current, char* end, char* open, char* close);
void tagSearch(bool image, char* current, char* end);
void tagSearchCode (char* current, char* end);


char* control; // before returning from any tagSearch control needs to be set if the search was successfull
void parseText(char* current, char* end)
{
  while  (current < end)
  {
    control = current;
    if (*current == *(current + 1))    // handle tags that require double prefix
    {
      if (*current == '*' || *current == '_') tagSearchTwo(current, end, "<strong>", "</strong>");
      else if (*current == '~') tagSearchTwo(current, end, "<s>", "</s>");
    }
    else     // handle tags that require a single prefix
    {
      if (*current == '*' || *current == '_') tagSearchOne(current, end, "<em>", "</em>");
      else if (*current == '\"') tagSearchOne(current, end, "&quot;", "&quot;");
      else if (*current == '`') tagSearchCode(current, end);
      else if (*current == '[') tagSearch(false, current, end);
      else if (*current == '!') tagSearch(true, current, end);
    }
    if (control != current)    // if there has been change skip already parsed text
    {
      current = control;
      continue;
    }
    fprintf(html, "%c", *current);
    current++;
  }
}

void tagSearch(bool image, char* current, char* end)
{
  char* start = current;
  if (image) start++;
  if (*start != '[') return;

  char* p1 = searchTwo(']', '(', start, end);
  if (p1 == end) return;

  char* p2 = searchOne(')', p1 + 2, end);
  if (p2 == end) return;

  *p1 = '\0';   // start->p1 = text
  *p2 = '\0';   // p1->p2 = link

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

  control = p2 + 1;  // continue to parse text just after ")"
}
void tagSearchCode (char* current, char* end)
{
  char* start = current + 1;
  char* p = searchOne(* current, start, end);
  if (p == end) return;
  fprintf(html, "<code>");
  printEscaped(start, p);
  fprintf(html, "</code>");
  control = p + 1;
}


void tagSearchOne(char* current, char* end, char* open, char* close)
{
  char* start = current + 1;
  char* p = searchOne(* current, start, end);
  if (p == end) return;
  fprintf(html, "%s", open);
  parseText(current + 1, p);
  fprintf(html, "%s", close);
  control = p + 1;
}

void tagSearchTwo(char* current, char* end, char* open, char* close)
{
  char* start = current + 2;
  char* p = searchTwo(*current, *current, start, end);
  if (p == end) return;
  fprintf(html, "%s", open);
  parseText(current + 2, p);
  fprintf(html, "%s", close);
  control = p + 2;
}


char* searchOne(char c, char* start, char* end)
{
  char* p = strchr(start, c);
  return (p != NULL && p < end) ? p : end;
}

char* searchTwo(char c1, char c2, char* start, char* end)
{
  char* p = strchr(start, c1);
  return (p != NULL && *(p + 1) == c2 && p < end) ? p : end;
}

void printEscaped(char* start, char* end){
  while (start < end)
  {
    if (*start == '<')
      fprintf(html, "&lt");
    else if (*start == '>')
      fprintf(html, "&gt");
    else
      fputc(*start, html);
    start++;
  }
}
