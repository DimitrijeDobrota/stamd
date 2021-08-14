#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <queue.h>

typedef enum {ul, ol, li, blockquote, p} tag_t;
static char* tag_n[] = {"ul", "ol", "li", "blockquote", "p"};

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
void startParagraph();

int startList(tag_t list_tag, char currentSign, int currentIndent);
int startBlockquote(int currentIndent);
int getBlockquoteLen();
void startCodeBlock();
void startCodeIndent(int currentIndent);

char* searchOne(char c, char* start, char* end);
char* searchTwo(char c1, char c2, char* start, char* end);
char* tagSearchOne(char* current, char* end, char* open, char* close);
char* tagSearchTwo(char* current, char* end, char* open, char* close);
char* tagSearch(bool image, char* current, char* end);

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

  indent = p - line;

  /* while (*p != ' ' && *p != '\0') */
  while (!isspace(*p) && *p != '\0')
    *b++ = *p++;
  *b = '\0';
  bufferEnd = b - 1;
  text = ++p;
}

void parseLine()
{

  if (*linebuffer == '\0') return;

  static const char* headingb[] = {"#", "##", "###", "####", "#####", "######"};
  static const char* headingt[] = {"h1", "h2", "h3", "h4", "h5", "h6"};

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

  if (strcmp("---", linebuffer) == 0 || strcmp("___", linebuffer) == 0 || strcmp("***", linebuffer) == 0) printf("<hr>\n");
  else if (strcmp(linebuffer, "```") == 0) startCodeBlock();
  else if (*bufferEnd == '-' || *bufferEnd == '+' || *bufferEnd == '*') startList(ul, *bufferEnd, indent);
  else if (*bufferEnd == '.' && isdigit(*(bufferEnd - 1))) startList(ol, *bufferEnd, indent);
  else if (*linebuffer == '>') startBlockquote(getBlockquoteLen());
  else if (indent >= 4) startCodeIndent(indent);
  else startParagraph();

  while (!queue_empty(&lineQueue))
    line_pop();

}

void startParagraph()
{
  printf("<p>");
  parseText(line, lineEnd);
  while (hasMoreLines())
  {
    readLine();
    splitLine();
    if (*linebuffer == '\0')
    {
      printf("</p>\n");
      return;
    }
    printf("\n");
    parseText(line, lineEnd);
  }

}

void startCodeIndent(int currentIndent)
{
  printf("<pre><code>\n%s\n", line + currentIndent);
  while (hasMoreLines())
  {
    readLine();
    splitLine();
    if (*linebuffer == '\0')
    {
      printf("</code></pre>\n");
      return;
    }
    printf("%s\n", line + currentIndent);
  }
}

void startCodeBlock()
{
  if (lineEnd - line < 4)
    printf("<pre><code>\n");
  else
    printf("<pre class=\"hl%s\"><code>\n", text);
  while (hasMoreLines())
  {
    readLine();
    splitLine();
    if (strcmp(linebuffer, "```") == 0)
    {
      printf("</code></pre>\n");
      return;
    }
    printf("%s\n", line);
  }
}

int startBlockquote(int currentIndent)
{
  line_push(blockquote, false);
  line_push(p, true);
  while (hasMoreLines())
  {
    readLine();
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
tag_t list_tag;
char list_sign;
int startList(tag_t currentTag, char currentSign, int currentIndent)
{
  line_push(currentTag, false);
  line_push(li, true);
  while (hasMoreLines())
  {
    readLine();
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
