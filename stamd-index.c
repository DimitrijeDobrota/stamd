#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "article.h"
#include "config.h"

static const char *articledir;
static const char *outdir;

FILE* md;
FILE* html;
FILE* config;

void usage(char *argv0);

int main(int argc, char* argv[])
{
  char  articledirabs[PATH_MAX + 1];
  char  configfile[PATH_MAX + 1];
  char  *categories;

  if (argc < 2) usage(*argv);

  html = stdout;
  categories = "Articles";

  int i = 1;
  if (argv[1][0] == '-' && argv[1][1] == 'c')
  {
    categories = argv[2];
    i=3;
  }

  writeHeader();
  if (strchr(categories, ' '))
  {
    writeCategories(categories);
    fprintf(html, "<h1>Articles</h1><ul>\n");
  }
  else
    fprintf(html, "<h1>%s</h1><ul>\n", categories);


  for (; i < argc; i++)
  {
    if (!realpath(argv[i], articledirabs))
      err(1, argv[i]);

    sprintf(configfile, "%s/config.txt", articledirabs);

    if (!(config = fopen(configfile, "r")))
      err(1, configfile);

    parseConfig();
    writeArticleItem(strrchr(articledirabs, '/') + 1);
  }
  fprintf(html, "</ul>\n");
  writeFooter();

  return 0;
}

void usage(char *argv0)
{
  fprintf(stderr, "%s articledir articledir ...\n", argv0);
  exit(1);
}
