#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <poafloc/poafloc.hpp>

#include "article.hpp"
#include "index.hpp"
#include "maddy/parser.h"
#include "utility.hpp"

void preprocess(article& article, std::istream& ist)
{
  std::string line;
  std::string key;
  std::string value;

  while (std::getline(ist, line))
  {
    if (line.empty()) break;
    if (line[0] != '@') break;

    {
      std::istringstream iss(line.substr(1));
      std::getline(iss, key, ':');
      std::getline(iss, value);

      trim(key);
      trim(value);
    }

    if (key == "hidden") article.set_hidden(true);
    else if (key == "nonav") article.set_nonav(true);
    else if (key != "categories") article.insert(key, value);
    else
    {
      std::istringstream iss(value);
      while (std::getline(iss, value, ',')) article.insert(trim(value));
    }
  }
}

struct arguments_t
{
  std::string output_dir = ".";
  std::vector<std::filesystem::path> files;
  bool index = false;

  std::string base = "https://dimitrijedobrota.com/blog";
};

int parse_opt(int key, const char* arg, poafloc::Parser* parser)
{
  auto* args = static_cast<arguments_t*>(parser->input());
  switch (key)
  {
    case 'o':
      args->output_dir = arg;
      break;
    case 'b':
      args->base = arg;
      break;
    case 'i':
      args->index = true;
      break;
    case poafloc::ARG:
      args->files.emplace_back(arg);
      break;
    default:
      break;
  }
  return 0;
}

// NOLINTBEGIN
static const poafloc::option_t options[] = {
    {"output", 'o', "DIR", 0, "Output directory"},
    {"index", 'i', 0, 0, "Generate all of the indices"},
    {"base", 'b', "URL", 0, "Base URL for the content"},
    {0},
};

static const poafloc::arg_t arg {
    options,
    parse_opt,
    "config_file",
    "",
};
// NOLINTEND

int main(int argc, char* argv[])
{
  using namespace stamd;  // NOLINT

  using category_map_t = std::unordered_map<std::string, article_list>;

  arguments_t args;

  if (poafloc::parse(&arg, argc, argv, 0, &args) != 0)
  {
    std::cerr << "There was an error while parsing arguments";
    return 1;
  }

  category_map_t category_map;
  categories_t all_categories;
  article_list all_articles;
  maddy::Parser parser;

  for (const auto& path : args.files)
  {
    const std::string filename = path.stem().string() + ".html";

    std::ifstream ifs(path.string());
    all_articles.push_back(make_shared<article>(filename));

    auto& article = all_articles.back();
    preprocess(*article, ifs);

    // filename can change in preprocessing phase
    std::ofstream ofs(article->get_filename());
    article->write(parser.Parse(ifs), ofs);

    if (!article->is_hidden())
    {
      all_categories.merge(article->get_categories());
      for (const auto& ctgry : article->get_categories())
        category_map[ctgry].push_back(article);
    }
  }

  if (!args.index) return 0;

  sort(begin(all_articles),
       end(all_articles),
       [](const auto& lft, const auto& rht)
       { return lft->get_date() > rht->get_date(); });

  std::ofstream atom("atom.xml");
  std::ofstream rss("rss.xml");
  std::ofstream sitemap("sitemap.xml");
  std::ofstream robots("robots.txt");

  create_sitemap(sitemap, all_articles);
  create_robots(robots);

  create_rss(rss, "index", all_articles);
  create_atom(atom, "index", all_articles);

  create_index("index", all_articles, all_categories);
  for (const auto& [category, articles] : category_map)
  {
    auto ctgry = category;
    create_index(normalize(ctgry), articles, {});
  }

  return 0;
}
