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
  char  articledirabs[PATH_MAX + 1], outdirabs[PATH_MAX + 1];
  char  mdfile[PATH_MAX + 1], htmlfile[PATH_MAX + 1], configfile[PATH_MAX + 1];
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

  if (!articledir) usage(argv[0]);

  if (!realpath(articledir, articledirabs))
    err(EXIT_FAILURE, articledir);

  if (!outdir)
    strcpy(outdirabs, articledirabs);
  else if (!realpath(outdir, outdirabs))
    err(EXIT_FAILURE, outdir);

  sprintf(mdfile, "%s/article.md", articledirabs);
  sprintf(htmlfile, "%s%s.html", outdirabs, strrchr(articledirabs, '/'));
  sprintf(configfile, "%s/config.txt", articledirabs);

  if (!(md = fopen(mdfile, "r"))) err(EXIT_FAILURE, mdfile);
  if (!(config = fopen(configfile, "r"))) err(EXIT_FAILURE, configfile);
  if (!(html = fopen(htmlfile, "w"))) err(EXIT_FAILURE, htmlfile);

  parseConfig();

  writeHeader();
  parseArticle();
  writeFooter();

  return 0;
}

void usage(char *argv0)
{
  fprintf(stderr, "%s [-o outdir] articledir\n", argv0);
  exit(1);
}
