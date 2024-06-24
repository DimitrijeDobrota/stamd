#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>

#include <hemplate/attribute.hpp>
#include <hemplate/classes.hpp>
#include <poafloc/poafloc.hpp>

#include "article.hpp"
#include "maddy/parser.h"
#include "utility.hpp"

using article_list = std::vector<std::shared_ptr<article>>;
using categories_t = article::categories_t;

void preprocess(article& article, std::istream& ist)
{
  std::string line;
  std::string key;
  std::string value;

  while (std::getline(ist, line))
  {
    if (line.empty()) continue;
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
    else if (key != "categories") article.emplace(key, value);
    else
    {
      std::istringstream iss(value);
      while (std::getline(iss, value, ',')) article.emplace(trim(value));
    }
  }
}

void create_index(const std::string& name,
                  const article_list& articles,
                  const categories_t& categories)
{
  using namespace hemplate;  // NOLINT

  std::ofstream ost(name + ".html");
  std::stringstream strs;

  strs << html::h1(name);
  strs << html::ul().set("class", "index");
  for (const auto& article : articles)
  {
    if (article->is_hidden()) continue;
    const auto& title = article->get_title();
    const auto& date  = article->get_date();

    strs << html::li()
                .add(html::div(std::format("{} - ", date)))
                .add(html::div().add(html::a(title).set("href", title)));
  };
  strs << html::ul();

  article index(name, categories);
  index.write(strs.str(), ost);
}

void create_atom(const std::string& name, const article_list& articles)
{
  using namespace hemplate;  // NOLINT

  static const char* base    = "https://dimitrijedobrota.com/blog";
  static const char* loc     = "https://dimitrijedobrota.com/blog/atom.feed";
  static const char* updated = "2003-12-13T18:30:02Z";
  static const char* summary = "Click on the article link to read...";

  std::ofstream ost(name + ".atom");

  elementList content = std::accumulate(
      begin(articles),
      end(articles),
      elementList(),
      [](elementList&& list, const auto& article)
      {
        const auto title = article->get_title();
        list.add(atom::entry()
                     .add(atom::title(title))
                     .add(atom::link().set(
                         "href", std::format("{}/{}.html", base, title)))
                     .add(atom::updated(updated))
                     .add(atom::summary(summary)));
        return std::move(list);
      });

  ost << atom::xml({{"version", "1.0"}, {"encoding", "utf-8"}});
  ost << atom::feed();
  ost << atom::title(name);
  ost << atom::link().set("href", base);
  ost << atom::link({{"rel", "self"}, {"href", loc}});
  ost << atom::id(base);
  ost << atom::updated(updated);
  ost << atom::author().add(atom::name(name));
  ost << atom::feed();
  ost << content;
  ost << atom::feed(content);
}

void create_rss(const std::string& name, const article_list& articles)
{
  using namespace hemplate;  // NOLINT

  std::ofstream ost(name + ".rss");

  static const char* author      = "Dimitrije Dobrota";
  static const char* email       = "mail@dimitrijedobrota.com";
  static const char* base        = "https://dimitrijedobrota.com/blog";
  static const char* description = "Contents of Dimitrije Dobrota's webpage";
  static const char* loc     = "https://dimitrijedobrota.com/blog/index.rss";
  static const char* updated = "2003-12-13T18:30:02Z";

  elementList content = std::accumulate(
      begin(articles),
      end(articles),
      elementList(),
      [](elementList&& list, const auto& article)
      {
        const auto title = article->get_title();
        list.add(rss::item()
                     .add(rss::title(title))
                     .add(rss::link(std::format("{}/{}.html", base, title)))
                     .add(rss::guid(std::format("{}/{}.html", base, title)))
                     .add(rss::pubDate(updated))
                     .add(rss::author(std::format("{} ({})", email, author))));
        return std::move(list);
      });

  ost << rss::xml({{"version", "1.0"}, {"encoding", "utf-8"}});
  ost << rss::rss(
      {{"version", "2.0"}, {"xmlns:atom", "http://www.w3.org/2005/Atom"}});
  ost << rss::channel();
  ost << rss::title(name);
  ost << rss::link(base);
  ost << rss::description(description);
  ost << rss::generator("stamd");
  ost << rss::language("en-us");
  ost << rss::atomLink(
      {{"href", loc}, {"rel", "self"}, {"type", "application/rss+xml"}});
  ost << content;
  ost << rss::channel();
  ost << rss::rss();
}

struct arguments_t
{
  std::string output_dir = ".";
  std::vector<std::string> articles;
};

int parse_opt(int key, const char* arg, poafloc::Parser* parser)
{
  auto* args = static_cast<arguments_t*>(parser->input());
  switch (key)
  {
    case 'o':
      args->output_dir = arg;
      break;
    case poafloc::ARG:
      args->articles.emplace_back(arg);
      break;
    defaut:
      poafloc::help(parser, stderr, poafloc::STD_USAGE);
      break;
  }
  return 0;
}

// NOLINTBEGIN
static const poafloc::option_t options[] = {
    {"output", 'o', "DIR", 0, "Output directory"},
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

  for (const auto& name : args.articles)
  {
    std::ofstream ofs(name + ".out");
    std::ifstream ifs(name);

    all_articles.push_back(make_shared<article>(name));

    auto& article = all_articles.back();
    preprocess(*article, ifs);
    article->write(parser.Parse(ifs), ofs);

    if (!article->is_hidden())
    {
      all_categories.merge(article->get_categories());
      for (const auto& ctgry : article->get_categories())
        category_map[ctgry].push_back(article);
    }
  }

  create_rss("index", all_articles);
  create_atom("index", all_articles);
  create_index("index", all_articles, all_categories);
  for (const auto& [category, articles] : category_map)
  {
    auto ctgry = category;
    create_index(normalize(ctgry), articles, {});
  }

  return 0;
}
