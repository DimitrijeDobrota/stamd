#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <queue.h>

typedef enum {ul, ol, li} tag_t;
static char* tag_n[] = {"ul", "ol", "li"};

queue_t lineQueue;
void line_push(tag_t tag, bool parse);
void line_pop();

static const char *articledir;
static const char *outdir;
static FILE* md;
static FILE* html;

char line[1000] = "\0";
char linebuffer[100] = "\0";
char* text;
char* lineEnd;
char* lineEnd;
char* bufferEnd;
int indent;

bool hasMoreLines();
void readLine();
void splitLine();
void parseLine();
void parseText(char* start, char* end);

int startList(tag_t list_tag, int currentIndent);

char* searchOne(char c, char* start, char* end);
char* searchTwo(char c1, char c2, char* start, char* end);
char* tagSearchOne(char* current, char* end, char* open, char* close);
char* tagSearchTwo(char* current, char* end, char* open, char* close);
char* tagSearchLink(char* current, char* end);
char* tagSearchImg(char* current, char* end);

void usage(char *argv0)
{
  fprintf(stderr, "%s [-o outdir] articledir\n", argv0);
  exit(1);
}

int main(int argc, char* argv[])
{
  char  articledirabs[PATH_MAX + 1], outdirabs[PATH_MAX + 1], mdfile[PATH_MAX + 1], htmlfile[PATH_MAX + 1];
  int i;

  queue_ctor(&lineQueue);


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
    parseLine();
  }

  queue_dtor(&lineQueue);
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

  while (isspace(*p) && *p != '\0')
    p++;

  indent = p - line + 1;

  /* while (*p != ' ' && *p != '\0') */
  while (!isspace(*p) && *p != '\0')
    *b++ = *p++;
  *b = '\0';
  bufferEnd = b - 1;
  text = ++p;
}

void parseLine()
{

  char* tag = "p";

  char* headingb[] = {"#", "##", "###", "####", "#####", "######"};
  char* headingt[] = {"h1", "h2", "h3", "h4", "h5", "h6"};

  for (int i = 0; i < 6; i++)
  {
    if (strcmp(linebuffer, headingb[i]) == 0)
    {
      printf("<%s>", headingt[i]);
      parseText(text, lineEnd);
      printf("</%s>\n", headingt[i]);
      return;
    }
  }

  if (strcmp("---", linebuffer) == 0 || strcmp("___", linebuffer) == 0 || strcmp("***", linebuffer) == 0)
  {
    printf("<hr>\n");
    return;
  }


  if (strcmp(linebuffer, "```") == 0)
  {
    printf("<code>\n");
    while (hasMoreLines())
    {
      readLine();
      splitLine();
      if (strcmp(linebuffer, "```") == 0)
      {
        printf("</code>\n");
        return;
      }
      printf("%s\n", line);
    }
  }
  else if (*bufferEnd == '-' || *bufferEnd == '+' || *bufferEnd == '*' || *bufferEnd == '.')
  {
    *bufferEnd == '.' ? startList(ol, indent) : startList(ul, indent);
    while (!queue_empty(&lineQueue))
      line_pop();
    return;
  }

  printf("<p>");
  parseText(line, lineEnd);
  printf("</p>\n");
}

void line_push(tag_t tag, bool parse)
{
  if (parse)
  {
    printf("<%s>", tag_n[tag]);
    parseText(text, lineEnd);
  }
  else
  {
    printf("<%s>\n", tag_n[tag]);
  }
  queue_push(&lineQueue, tag);
}

void line_pop()
{
  tag_t tag = queue_top(&lineQueue);
  queue_pop(&lineQueue);
  printf("</%s>\n", tag_n[tag]);
}

// Function to start new level of indend with UL
// If blank line is reached 1 is returned, to end the recursion
int startList(tag_t currentTag, int currentIndent)
{
  line_push(currentTag, false);
  line_push(li, true);
  while (hasMoreLines())
  {
    readLine();
    splitLine();
    tag_t listTag = ul;
    if (*bufferEnd == '-' || *bufferEnd == '+' || *bufferEnd == '*' || *bufferEnd == '.')
    {
      if (*bufferEnd == '.') listTag = ol;
      // New list item is reached, end the previous one
      line_pop();

      if (currentIndent < indent)
      {
        // We are too low. Start new Ul with a bigger indent
        int blankLine = startList(listTag, indent);
        if (blankLine ) return 1; // blank line must end all Uls, propagate throught recursion
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
      if (currentTag != listTag){
        line_pop();
        line_push(listTag, false);
        currentTag=listTag;
      }

      // We are at the good indent level
      // Start a new list item
      line_push(li, true);
    }
    else if (line[0] == '\0') return 1;              // blank line must end all Uls
    else parseText(line + currentIndent, lineEnd);   // Current LI spans multiple lines. Just print the text of subsiquent lines
  }
  return 0;
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
      else if (*current == '!')
        current = tagSearchImg(current, end);
    }
    if (control != current) continue;   // if there has been change skip already parsed text
    printf("%c", *current);
    current++;
  }
}

char* tagSearchImg(char* current, char* end)
{
  if (*(current + 1) != '[') return current;

  char* p1 = searchTwo(']', '(', current, end);
  if (p1 == end) return current;

  char* p2 = searchOne(')', p1 + 2, end);
  if (p2 == end) return current;

  *p1 = '\0';
  *p2 = '\0';

  char* blank = strchr(p1 + 2, ' ');

  if (blank == NULL)
  {
    printf("<img src=\"%s\" alt=\"%s\">", p1 + 2, current + 2);
  }
  else
  {
    *blank = '\0';
    printf("<img src=\"%s\" alt=\"%s\" title=%s>", p1 + 2, current + 2, blank + 1);
  }

  return p2 + 1;
}

char* tagSearchLink(char* current, char* end)
{
  char* p1 = searchTwo(']', '(', current, end);
  if (p1 == end) return current;

  char* p2 = searchOne(')', p1, end);
  if (p2 == end) return current;

  *p1 = '\0';
  *p2 = '\0';

  printf("<a href=\"%s\">%s</a>", p1 + 2, current + 1);
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
  char* p = strchr(start, c);
  return (p != NULL && p < end) ? p : end;
}

char* searchTwo(char c1, char c2, char* start, char* end)
{
  char* p = strchr(start, c1);
  return (p != NULL && *(p + 1) == c2 && p < end) ? p : end;
}
