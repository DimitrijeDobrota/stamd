#include <err.h>
#include <limits.h>
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

bool hasMoreLines();
void readLine();
void splitLine();

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
    printf("%s\n", text);
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
  else if (strcmp(linebuffer, "---") == 0) return;
  else if (strcmp(linebuffer, "-") == 0 || strcmp(linebuffer, "+") == 0 || strcmp(linebuffer, "*") == 0) return;
  else if (linebuffer[-1] == '.') return;

  text = line;  // text is a whole line
}
