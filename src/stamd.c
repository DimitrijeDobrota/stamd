#define _XOPEN_SOURCE 700

#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "md4c-html.h"

#include <cii/atom.h>
#include <cii/list.h>
#include <cii/mem.h>
#include <cii/str.h>
#include <cii/table.h>

#define MAX_SIZE 100

#define BASE_URL     "https://dimitrijedobrota.com/blog"
#define TITLE        "Dimitrije Dobrota's blog"
#define AUTHOR       "Dimitrije Dobrota"
#define AUTHOR_EMAIL "mail@dimitrijedobrota.com"

#define SETTINGS_TIME_FORMAT "%Y-%m-%d"

#define ATOM_TIME_FORMAT "%Y-%m-%dT%H:%M:%SZ"
#define ATOM_FILE        "index.atom"
#define ATOM_LOCATION    BASE_URL "/" ATOM_FILE

#define RSS_TIME_FORMAT "%a, %d %b %Y %H:%M:%S +0200"
#define RSS_FILE        "rss.xml"
#define RSS_LOCATION    BASE_URL "/" RSS_FILE

#define SITEMAP      "%a, %d %b %Y %H:%M:%S +0200"
#define SITEMAP_FILE "sitemap.xml"

List_T  articles;        /* List of all articles */
List_T  articlesVisible; /* List of all articles that are not hidden*/
Table_T category_table;  /* Table of all non hidden articles for each category*/

void usage(char *argv0) {
  fprintf(stderr, "Usage: %s [-o output_dir] article\n", argv0);
  exit(EXIT_FAILURE);
}

void process(const MD_CHAR *text, MD_SIZE size, void *userdata) {
  fprintf((FILE *)userdata, "%.*s", size, text);
}

char *memory_open(char *infile, int *size) {
  char       *addr;
  int         fd, ret;
  struct stat st;

  if ((fd = open(infile, O_RDWR)) < 0)
    error(EXIT_FAILURE, errno, "%s", infile);

  if ((ret = fstat(fd, &st)) < 0)
    error(EXIT_FAILURE, errno, "line %d, fstat", __LINE__);

  if ((addr = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd,
                   0)) == MAP_FAILED)
    error(EXIT_FAILURE, errno, "line %d, addr", __LINE__);

  if (size)
    *size = st.st_size;

  return addr;
}

void memory_close(char *content, int size) {
  if (munmap(content, size) == -1)
    error(EXIT_FAILURE, errno, "line %d, munmap", __LINE__);
}

char *normalize(char *name) {
  char *s = name, *c = name;

  while (isspace(*c))
    c++;

  for (; *c; c++)
    if (isspace(*c) && *(name - 1) != '_')
      *name++ = '_';
    else if (isalnum(*c))
      *name++ = *c;
  *name = '\0';
  return s;
}

void strip_whitspace(char *str) {
  char *p;

  if (!str)
    return;

  p = str + strlen(str);
  do {
    p--;
  } while (isspace(*p));

  *(p + 1) = '\0';

  p = str;
  while (isspace(*p))
    p++;

  while (*p)
    *str++ = *p++;

  *str = '\0';
}

int strscmp(const void *a, const void *b) {
  return -strcmp((char *)b, (char *)a);
}

void applyListFree(void **ptr, void *cl) { FREE(*ptr); }

#define T Article_T
typedef struct T *T;
struct T {
  char *content;
  char *output_dir;
  int   content_size;

  int hidden;
  int nonav;

  FILE *outfile;

  Table_T symbols;
  List_T  categories;
};

#define AP(...) fprintf(article->outfile, __VA_ARGS__);

T Article_new(char *output_dir, char *title) {
  T p;

  NEW0(p);

  if (!title)
    title = "article";

  p->output_dir = output_dir;

  p->symbols = Table_new(0, NULL, NULL);
  p->categories = List_list(NULL);

  Table_put(p->symbols, Atom_string("title"), title);
  Table_put(p->symbols, Atom_string("date"), "1970-01-01");
  Table_put(p->symbols, Atom_string("lang"), "en");

  return p;
}

void Article_setContent(T self, char *content, int content_size) {
  self->content = content;
  self->content_size = content_size;
}

int Article_cmp(const void *a, const void *b) {
  Article_T a1 = (Article_T)a;
  Article_T a2 = (Article_T)b;

  int res = strcmp(Table_get(a1->symbols, Atom_string("date")),
                   Table_get(a2->symbols, Atom_string("date")));
  if (res)
    return -res;

  return strcmp(Table_get(a1->symbols, Atom_string("title")),
                Table_get(a2->symbols, Atom_string("title")));
}

void Article_openWrite(T self) {
  char outfile[2 * PATH_MAX];

  if (!Table_get(self->symbols, Atom_string("filename"))) {
    Table_put(self->symbols, Atom_string("filename"),
              Str_dup(Table_get(self->symbols, Atom_string("title")), 1, 0, 1));
  }

  char *filename = Table_get(self->symbols, Atom_string("filename"));
  normalize(filename);
  sprintf(outfile, "%s/%s.html", self->output_dir, filename);

  if ((self->outfile = fopen(outfile, "w")) == NULL)
    error(EXIT_FAILURE, errno, "line %d, fopen(%s)", __LINE__, outfile);
}

void Article_closeWrite(T self) {
  if (self->outfile)
    fclose(self->outfile);
}

void Article_free(T self) {
  Table_free(&self->symbols);

  /* List_map(self->categories, applyListFree, NULL); */
  List_free(&self->categories);

  FREE(self);
}

void print_category_item(void **item, void *article_pointer) {
  char *category = *(char **)item, *norm;

  Article_T article = (Article_T)article_pointer;
  norm = normalize(Str_dup(category, 1, 0, 1));
  AP("<a href=\"./%s.html\">%s</a>\n", norm, category);
  FREE(norm);
}

void print_header(T article) {
  AP("<!DOCTYPE html>\n"
     "<html lang=\"%s\">\n"
     "<head>\n"
     "<title>%s</title>\n",
     (char *)Table_get(article->symbols, Atom_string("lang")),
     (char *)Table_get(article->symbols, Atom_string("title")));

  AP("<meta charset=\"UTF-8\" />\n"
     "<meta name=\"viewport\" "
     "content=\"width=device-width,initial-scale=1\"/>\n"
     "<meta name=\"description\" content=\"Dimitrije Dobrota's personal site. "
     "You can find my daily findings in a form of articles on my blog as well "
     "as various programming projects.\" />\n"
     "<link rel=\"stylesheet\" type=\"text/css\" href=\"/css/index.css\" />\n"
     "<link rel=\"stylesheet\" type=\"text/css\" href=\"/css/colors.css\" />\n"

     "<link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" "
     "href=\"/img/favicon-32x32.png\">\n"
     "<link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" "
     "href=\"/img/favicon-16x16.png\">\n"
     "</head>\n"
     "<body>\n"
     "<input type=\"checkbox\" id=\"theme_switch\" class=\"theme_switch\">\n"
     "<main>\n"
     "<div class=\"content\">\n"
     "<label for=\"theme_switch\" class=\"switch_label\"></label>\n");

  if (article->nonav)
    return;

  AP("<div>\n<nav><a class=\"back\">&lt;-- back</a><a "
     "href=\"" BASE_URL "\">index</a><a href=\"/\">home "
     "--&gt;</a></nav><hr>\n</div>\n");

  if (List_length(article->categories) > 0) {
    List_sort(article->categories, strscmp);

    AP("<div class=\"categories\"><h3>Categories:</h3><p>\n");
    List_map(article->categories, print_category_item, article);
    AP("</p></div>\n");
  }
}

void print_footer(T article) {
  if (!article->nonav) {
    AP("<div class=\"bottom\">\n<hr>\n<nav>\n<a class=\"back\">&lt;-- "
       "back</a><a href=\"" BASE_URL "\">index</a><a href=\"/\">home "
       "--&gt;</a></nav></div>\n");
  }

  AP("</div>\n</main>\n"
     "<script src=\"/scripts/main.js\"></script>\n"
     "</body>\n</html>\n");
}

void print_index_item(void **article_item_pointer, void *article_pointer) {
  Article_T article_item = *(Article_T *)article_item_pointer;
  Article_T article = (Article_T)article_pointer;

  AP("<li><div>%s - </div><div><a href=\"%s.html\">%s</a></div></li>\n",
     (char *)Table_get(article_item->symbols, Atom_string("date")),
     (char *)Table_get(article_item->symbols, Atom_string("filename")),
     (char *)Table_get(article_item->symbols, Atom_string("title")))
}

void print_index(Article_T article, List_T articles, List_T categories) {

  article->categories = categories;
  Article_openWrite(article);

  print_header(article);
  {
    List_sort(articles, Article_cmp);

    AP("<h1>%s</h1>\n",
       (char *)Table_get(article->symbols, Atom_string("title")));

    AP("<ul class=\"index\">\n");
    List_map(articles, print_index_item, article);
    AP("</ul>\n");
  }
  print_footer(article);

  Article_closeWrite(article);
  Article_free(article);
}

void Article_preprocess(T self) {
  char *text = self->content;

  char *line;
  for (line = strtok(text, "\n"); line; line = strtok(NULL, "\n")) {
    strip_whitspace(line);
    if (!*line)
      continue;
    else if (*line == '@') {
      char *keys, *values;

      keys = CALLOC(1000, sizeof(char));
      values = CALLOC(1000, sizeof(char));

      sscanf(line, " @%[^:]: %[^\n] ", keys, values);

      if (!strcmp(keys, "hidden"))
        self->hidden = 1;
      else if (!strcmp(keys, "nonav"))
        self->nonav = 1;
      else
        Table_put(self->symbols, Atom_string(keys), Str_dup(values, 1, 0, 1));

      FREE(values);
      FREE(keys);
    } else {
      *(line + strlen(line)) = '\n';
      text = line;
      break;
    }
  }

  self->content_size = self->content_size - (text - self->content);
  self->content = text;

  char *cat;
  if ((cat = (char *)Table_get(self->symbols, Atom_string("categories")))) {
    char delim[] = ",", *category;
    for (category = strtok(cat, delim); category;
         category = strtok(NULL, delim)) {
      if (strlen(category) > 1) {
        strip_whitspace(category);

        const char *atom = Atom_string(category);
        self->categories = List_push(self->categories, category);

        /* append the article to the list of articles for a current category*/
        if (!self->hidden)
          Table_put(category_table, atom,
                    List_push(Table_get(category_table, atom), self));
      }
    }
  }

  if (!self->hidden)
    articlesVisible = List_push(articlesVisible, self);
}

void Article_translate(T self) {
  Article_preprocess(self);
  Article_openWrite(self);
  print_header(self);
  md_html(self->content, self->content_size, process, self->outfile,
          MD_DIALECT_GITHUB, 0);
  print_footer(self);
  Article_closeWrite(self);
}

char *get_date(char *date_str, char **date_buf, char *conversion) {
  struct tm date;
  memset(&date, 0, sizeof(date));

  strptime(date_str, SETTINGS_TIME_FORMAT, &date);
  strftime(*date_buf, MAX_SIZE, conversion, &date);

  return *date_buf;
}

void print_atom_item(void **article_item_pointer, void *file_pointer) {
  Article_T article_item = *(Article_T *)article_item_pointer;
  char     *date_buffer = ALLOC(MAX_SIZE);
  FILE     *f = (FILE *)file_pointer;

  get_date((char *)Table_get(article_item->symbols, Atom_string("date")),
           &date_buffer, ATOM_TIME_FORMAT);
  fprintf(f,
          "<entry>\n"
          "  <title>%s</title>\n"
          "  <link href=\"" BASE_URL "/%s.html\"/>\n"
          "  <id>" BASE_URL "/%s.html</id>\n"
          "  <updated>%s</updated>\n"
          "  <summary>Click on the article link to read...</summary>\n"
          "</entry>\n",
          (char *)Table_get(article_item->symbols, Atom_string("title")),
          (char *)Table_get(article_item->symbols, Atom_string("filename")),
          (char *)Table_get(article_item->symbols, Atom_string("filename")),
          date_buffer);
  FREE(date_buffer);
}
void print_atom(List_T articles, FILE *f) {
  fprintf(f, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
             "<feed xmlns=\"http://www.w3.org/2005/Atom\">\n"
             "<title>" TITLE "</title>\n"
             "<link href=\"" BASE_URL "/\"/>\n"
             "<link rel=\"self\" "
             "href=\"" ATOM_LOCATION "\" />\n"
             "<id>" BASE_URL "</id>\n"
             "<updated>2003-12-13T18:30:02Z</updated>\n"
             "<author>\n"
             "<name>" AUTHOR "</name>\n"
             "</author>\n");
  List_map(articles, print_atom_item, f);
  fprintf(f, "</feed>\n");
}

void print_rss_item(void **article_item_pointer, void *file_pointer) {
  Article_T article_item = *(Article_T *)article_item_pointer;
  char     *date_buffer = ALLOC(MAX_SIZE);
  FILE     *f = (FILE *)file_pointer;

  get_date((char *)Table_get(article_item->symbols, Atom_string("date")),
           &date_buffer, RSS_TIME_FORMAT);
  fprintf(f,
          "<item>\n"
          "  <title>%s</title>\n"
          "  <link>" BASE_URL "/%s.html</link>\n"
          "  <guid>" BASE_URL "/%s.html</guid>\n"
          "  <pubDate>%s</pubDate>\n"
          "  <author>" AUTHOR_EMAIL " (" AUTHOR ")</author>\n"
          "</item>\n",
          (char *)Table_get(article_item->symbols, Atom_string("title")),
          (char *)Table_get(article_item->symbols, Atom_string("filename")),
          (char *)Table_get(article_item->symbols, Atom_string("filename")),
          date_buffer);
  FREE(date_buffer);
}

void print_rss(List_T articles, FILE *f) {
  fprintf(f,
          "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
          "<rss version=\"2.0\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n"
          "<channel>\n"
          "<title>" TITLE "</title>\n"
          "<link>" BASE_URL "</link>\n"
          "<description>Contents of Dimitrije Dobrota's webpage</description>"
          "<generator>stamd</generator>"
          "<language>en-us</language>\n"
          "<atom:link href=\"" RSS_LOCATION "\" rel=\"self\" "
          "type=\"application/rss+xml\" />");
  List_map(articles, print_rss_item, f);
  fprintf(f, "</channel>\n"
             "</rss>\n");
}

void print_sitemap_item(void **article_item_pointer, void *file_pointer) {
  Article_T article_item = *(Article_T *)article_item_pointer;
  char     *date_buffer = ALLOC(MAX_SIZE);
  FILE     *f = (FILE *)file_pointer;

  get_date((char *)Table_get(article_item->symbols, Atom_string("date")),
           &date_buffer, RSS_TIME_FORMAT);
  fprintf(f,
          "<url>\n"
          "  <loc>" BASE_URL "/%s.html</loc>\n"
          "  <changefreq>weekly</changefreq>\n"
          "</url>\n",
          (char *)Table_get(article_item->symbols, Atom_string("filename")));
  FREE(date_buffer);
}

void print_sitemap(List_T articles, FILE *f) {
  fprintf(f,
          "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
          "<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n");
  List_map(articles, print_sitemap_item, f);
  fprintf(f, "</ulrset>\n");
}

/* void print_xml(char *file_name, void(*)) */

int main(int argc, char *argv[]) {
  char output_dir[PATH_MAX];
  int  opt;

  while ((opt = getopt(argc, argv, "o:")) != -1) {
    switch (opt) {
    case 'o':
      if (!realpath(optarg, output_dir))
        error(EXIT_FAILURE, errno, "-o %s", optarg);
      break;
    default:
      usage(argv[0]);
    }
  }

  if (optind >= argc)
    usage(argv[0]);

  if (!*output_dir)
    realpath(".", output_dir);

  category_table = Table_new(0, NULL, NULL);

  articles = List_list(NULL);
  articlesVisible = List_list(NULL);

  for (; optind < argc; optind++) {
    T     article;
    char *content;
    int   content_size;

    content = memory_open(argv[optind], &content_size);
    article = Article_new(output_dir, NULL);
    Article_setContent(article, content, content_size);
    Article_translate(article);
    memory_close(content, content_size);

    articles = List_push(articles, article);
  }

  /* Print main index and index for each encountered category*/
  {
    List_T categories = List_list(NULL);
    void **array = Table_toArray(category_table, NULL);

    for (int i = 0; array[i]; i += 2) {
      categories = List_push(categories, array[i]);
      print_index(Article_new(output_dir, array[i]), array[i + 1], NULL);
    }

    if (List_length(articlesVisible) > 1) {
      print_index(Article_new(output_dir, "index"), articlesVisible,
                  categories);

      char  outfile[2 * PATH_MAX];
      FILE *f;

      sprintf(outfile, "%s/%s", output_dir, ATOM_FILE);
      f = fopen(outfile, "w");
      print_atom(articlesVisible, f);
      fclose(f);

      sprintf(outfile, "%s/%s", output_dir, RSS_FILE);
      f = fopen(outfile, "w");
      print_rss(articlesVisible, f);
      fclose(f);

      sprintf(outfile, "%s/%s", output_dir, SITEMAP_FILE);
      f = fopen(outfile, "w");
      print_sitemap(articlesVisible, f);
      fclose(f);
    }

    FREE(array);
  }

  /* Free  category table*/
  {
    List_T *symbols = (List_T *)Table_toArray(category_table, NULL);
    for (int i = 0; symbols[i]; i += 2)
      List_free(&symbols[i + 1]);
    FREE(symbols);
  }

  /* Free articles */
  {
    Article_T *article_list = (Article_T *)List_toArray(articles, NULL);
    for (int i = 0; article_list[i]; i++)
      Article_free(article_list[i]);
    FREE(article_list);
  }

  Table_free(&category_table);

  List_free(&articles);
  List_free(&articlesVisible);

  return 0;
}
