#include <err.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *articledir;
static const char *outdir;
static FILE* md;
static FILE* html;

char line[1000] = "\0";
char linebuffer[100] = "\0";
char* text;
char* lineEnd;

bool hasMoreLines();
void readLine();
void splitLine();
void parseText(char* start, char* end);

char* searchOne(char c, char* start, char* end);
char* searchTwo(char c1, char c2, char* start, char* end);
char* tagSearchOne(char* current, char* end, char* open, char* close);
char* tagSearchTwo(char* current, char* end, char* open, char* close);
char* tagSearchLink(char* current, char* end);

void usage(char *argv0)
{
  fprintf(stderr, "%s [-o outdir] articledir\n", argv0);
  exit(1);
}

int main(int argc, char* argv[])
{
  char  articledirabs[PATH_MAX + 1], outdirabs[PATH_MAX + 1], mdfile[PATH_MAX + 1], htmlfile[PATH_MAX + 1];
  int i;


  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] != '-')
    {
      if (articledir)
        usage(argv[0]);
      articledir = argv[i];
    }
    else if (argv[i][1] == 'o')
    {
      if (i + 1 >= argc)
        usage(argv[0]);
      outdir = argv[++i];
    }
  }

  if (!articledir)
    usage(argv[0]);

  if (!realpath(articledir, articledirabs))
    err(1, articledir);

  if (!outdir)
    strcpy(outdirabs, articledirabs);
  else if (!realpath(outdir, outdirabs))
    err(1, outdir);


  sprintf(mdfile, "%s/article.md", articledirabs);
  sprintf(htmlfile, "%s%s.html", outdirabs, strrchr(articledirabs, '/'));

  if (!(md = fopen(mdfile, "r")))
    err(1, mdfile);
  if (!(html = fopen(htmlfile, "w")))
    err(1, htmlfile);

  while (hasMoreLines())
  {
    readLine();
    if (line[0] == '\0') continue;
    splitLine();
    parseText(text, lineEnd);
    printf("\n");
  }

  return 0;
}

bool hasMoreLines()
{
  int a;
  if ((a = fgetc(md)) != EOF)
  {
    ungetc(a, md);
    return true;
  }
  return false;
}

void readLine()
{
  char* p = line;
  int c;

  while ((c = fgetc(md)) != '\n')
    * p++ = c;

  *p = '\0';
  lineEnd = p;
}

void splitLine()
{
  char *p = line;
  char *b = linebuffer;   // save potential line tag

  while (*p != ' ' && *p != '\0')
    *b++ = *p++;
  *b = '\0';
  text = ++p;

  if (strcmp(linebuffer, "#") == 0) return;
  else if (strcmp(linebuffer, "##") == 0) return;
  else if (strcmp(linebuffer, "###") == 0) return;
  else if (strcmp(linebuffer, "####") == 0) return;
  else if (strcmp(linebuffer, "#####") == 0) return;
  else if (strcmp(linebuffer, "######") == 0) return;
  else if (strcmp(linebuffer, "---") == 0 || strcmp(linebuffer, "___") == 0 || strcmp(linebuffer, "***") == 0) return;
  else if (strcmp(linebuffer, "-") == 0 || strcmp(linebuffer, "+") == 0 || strcmp(linebuffer, "*") == 0) return;
  else if (linebuffer[-1] == '.') return;

  text = line;  // text is a whole line
}

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
        current = tagSearchOne(current, end, "<code>", "<code>");
      else if (*current == '[')
        current = tagSearchLink(current, end);
    }
    if (control != current) continue;   // if there has been change skip already parsed text
    printf("%c", *current);
    current++;
  }
}

char* tagSearchLink(char* current, char* end)
{
  char* next = current + 1;
  char* p1 = searchTwo(']', '(', next, end);
  if (p1 == end) return current;

  char* p2 = searchOne(')', p1 + 2, end);
  if (p2 == end) return current;

  current = p1 + 2;
  printf("<a href=\"%.*s\">%.*s</a>", p2 - current, current, p1 - next, next);
  return p2 + 1;
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
  while (start < end)
  {
    if (*start == c)
      return start;
    start++;
  }
  return end;
}

char* searchTwo(char c1, char c2, char* start, char* end)
{
  while (start < end)
  {
    if (*start == c1 && *(start + 1) == c2)
      return start;
    start++;
  }
  return end;
}
