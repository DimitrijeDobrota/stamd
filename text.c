#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "text.h"

char* searchOne(char c, char* start, char* end);
char* searchTwo(char c1, char c2, char* start, char* end);
char* tagSearchOne(char* current, char* end, char* open, char* close);
char* tagSearchTwo(char* current, char* end, char* open, char* close);
char* tagSearch(bool image, char* current, char* end);

void parseText(char* current, char* end)
{
  while  (current < end)
  {
    char* control = current;   // if current == control after the search nothing has been found
    if (*current == *(current + 1))    // handle tags that require double prefix
    {
      if (*current == '*' || *current == '_')
        current = tagSearchTwo(current, end, "<strong>", "</strong>");
      else if (*current == '~')
        current = tagSearchTwo(current, end, "<s>", "</s>");
    }
    else     // handle tags that require a single prefix
    {
      if (*current == '*' || *current == '_')
        current = tagSearchOne(current, end, "<em>", "</em>");
      else if (*current == '\"')
        current = tagSearchOne(current, end, "&quot;", "&quot;");
      else if (*current == '`')
        current = tagSearchOne(current, end, "<code>", "</code>");
      else if (*current == '[')
        current = tagSearch(false, current, end);
      else if (*current == '!')
        current = tagSearch(true, current, end);
    }
    if (control != current) continue;   // if there has been change skip already parsed text
    printf("%c", *current);
    current++;
  }
}

char* tagSearch(bool image, char* current, char* end)
{
  char* start = current;
  if (image) start++;
  if (*start != '[') return current;

  char* p1 = searchTwo(']', '(', start, end);
  if (p1 == end) return current;

  char* p2 = searchOne(')', p1 + 2, end);
  if (p2 == end) return current;

  *p1 = '\0';   // start->p1 = text
  *p2 = '\0';   // p1->p2 = link

  p1 += 2; // step over "]("
  start += 1; // step over "["


  char* blank = strchr(p1 + 2, ' '); // Detect title

  if (blank == NULL || blank > p2)
  {
    // there is no title attribute
    if (image)
      printf("<img src=\"%s\" alt=\"%s\">", p1, start);
    else
      printf("<a href=\"%s\">%s</a>", p1, start);
  }
  else
  {
    // there is a title attribute
    *blank = '\0';  // p2->blank = title
    blank += 1; // get to the quoted part
    if (image)
      printf("<img src=\"%s\" alt=\"%s\" title=%s>", p1, start, blank);
    else
      printf("<a href=\"%s\" title=%s>%s</a>", p1, blank, start);
  }

  return p2 + 1;  // continue to parse text just after ")"
}

char* tagSearchOne(char* current, char* end, char* open, char* close)
{
  char* start = current + 1;
  char* p = searchOne(* current, start, end);
  if (p == end) return current;
  printf("%s", open);
  parseText(current + 1, p);
  printf("%s", close);
  return p + 1;
}

char* tagSearchTwo(char* current, char* end, char* open, char* close)
{
  char* start = current + 2;
  char* p = searchTwo(*current, *current, start, end);
  if (p == end) return current;
  printf("%s", open);
  parseText(start, p);
  printf("%s", close);
  return p + 2;
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
