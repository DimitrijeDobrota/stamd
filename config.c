#include "config.h"
#include "stdio.h"
#include "string.h"

#include "line.h"

extern FILE* config;
extern FILE* html;

extern char* text;
extern char line[1000];
extern char linebuffer[100];

char lang[100];
char title[100];
char date[100];

void parseConfigLine(void)
{
  if (*linebuffer == '\0') return;
  else if (strcmp(linebuffer, "Title:") == 0) strcpy(title, text);
  else if (strcmp(linebuffer, "Language:") == 0) strcpy(lang, text);
  else if (strcmp(linebuffer, "Date:") == 0) strcpy(date, text);
}

void writeArticleItem(char* name)
{
  fprintf(html, "<li><a href=\"./%s.html\">%s - %s</a></li>\n", name, date, title);
}

void  writeHeader(void)
{
  fprintf(html, "<!DOCTYPE html>\n\n<html lang=\"%s\">\n", lang);
  fprintf(html, "<head>\n<meta charset=\"UTF-8\" />\n<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" />\n");
  fprintf(html, "<meta name=”robots” content=\"index, follow\">\n\n<link rel=\"stylesheet\" type=\"text/css\" href=\"/styles/colors.css\" />\n");
  fprintf(html, "<link rel=\"stylesheet\" type=\"text/css\" href=\"/styles/main.css\" />\n\n<link rel=\"apple-touch-icon\" sizes=\"180x180\" href=\"/pix/apple-touch-icon.png\">\n");
  fprintf(html, "<link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"/pix/favicon-32x32.png\">\n<link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/pix/favicon-16x16.png\">\n");
  fprintf(html, "<link rel=\"manifest\" href=\"/site.webmanifest\">\n\n");
  fprintf(html, "<meta name=\"description\" content=\"%s - %s\" />\n<title>%s</title>\n", date, title, title);
  fprintf(html, "</head>\n<body>\n<input type=\"checkbox\" id=\"theme_switch\" class=\"theme_switch\">\n<div id=\"page\">\n<label for=\"theme_switch\" class=\"switch_label\"></label>\n<main id=\"blog\">\n<section>\n");
}

void writeFooter(void)
{
  fprintf(html, "</section>\n</main>\n</div>\n<script src=\"/main.js\"></script>\n</body>\n</html>\n");
}

void parseConfig(void)
{
  parseDocument(config, parseConfigLine);
}
