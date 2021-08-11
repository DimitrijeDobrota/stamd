#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static const char *articledir;
static const char *outdir;

static char *name = "";


void usage(char *argv0)
{
  fprintf(stderr, "%s [-o outdir] articledir\n", argv0);
  exit(1);
}

int main(int argc, char* argv[]){
  char  articledirabs[PATH_MAX + 1], outdirabs[PATH_MAX + 1];
  int i;

  for (i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      if (articledir)
        usage(argv[0]);
      articledir= argv[i];
    } else if (argv[i][1] == 'o') {
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
  else
    if (!realpath(outdir, outdirabs))
      err(1, outdir);

  printf("Article dir: %s\n", articledir);
  printf("Article dir abs: %s\n", articledirabs);
  printf("Out dir: %s\n", outdir);
  printf("Out dir abs: %s\n", outdirabs);

  return 0;
}
