#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <md4c-html.h>
#include <poafloc/poafloc.hpp>

#include "article.hpp"
#include "indexer.hpp"
#include "utility.hpp"

void preprocess(stamd::article& article, std::istream& ist)
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
  std::filesystem::path output_dir = ".";
  std::vector<std::filesystem::path> files;
  bool index = false;

  stamd::indexer::options_t options;
};

int parse_opt(int key, const char* arg, poafloc::Parser* parser)
{
  auto* args = static_cast<arguments_t*>(parser->input());
  switch (key)
  {
    case 'o':
      args->output_dir = arg;
      break;
    case 'i':
      args->index = true;
      break;
    case 'b':
      args->options.base_url = arg;
      break;
    case 'a':
      args->options.author = arg;
      break;
    case 'e':
      args->options.email = arg;
      break;
    case 'd':
      args->options.description = arg;
      break;
    case 's':
      args->options.summary = arg;
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
// clang-format off
static const poafloc::option_t options[] = {
    {0, 0, 0, 0, "Output mode", 1},
    {"output", 'o', "DIR", 0, "Output directory"},
    {"index", 'i', 0, 0, "Generate all of the indices"},
    {0, 0, 0, 0, "General information", 2},
    {"base", 'b', "URL", 0, "Base URL for the content"},
    {"author", 'a', "NAME", 0, "Name of the author, if not specified in article"},
    {"email", 'e', "EMAIL", 0, "Email of the author, if not specified in article"},
    {"summary", 's', "SMRY", 0, "A summary, if not specified in article"},
    {"description", 'd', "DESC", 0, "Description of RSS feed"},
    {0, 0, 0, 0, "Informational Options", -1},
    {0},
};
// clang-format on

static const poafloc::arg_t arg {
    options,
    parse_opt,
    "config_file",
    "",
};
// NOLINTEND

void process_output(const MD_CHAR* str, MD_SIZE size, void* data)
{
  std::ofstream& ofs = *static_cast<std::ofstream*>(data);
  ofs << std::string(str, size);
}

int main(int argc, char* argv[])
{
  using namespace stamd;  // NOLINT

  arguments_t args;

  if (poafloc::parse(&arg, argc, argv, 0, &args) != 0)
  {
    std::cerr << "There was an error while parsing arguments";
    return 1;
  }

  using category_map_t =
      std::unordered_map<std::string, indexer::article_list>;

  stamd::indexer::categories_t categories;
  category_map_t category_map;
  indexer indexer(args.options);

  for (const auto& path : args.files)
  {
    const std::string filename = path.stem().string() + ".html";

    std::ifstream ifs(path.string());
    auto& article = indexer.add(make_shared<stamd::article>(filename));

    preprocess(*article, ifs);

    // filename can change in preprocessing phase
    std::ofstream ofs(args.output_dir / article->get_filename());
    std::stringstream sst;

    sst << ifs.rdbuf();

    article->write_header(ofs);
    md_html(sst.str().c_str(),
            static_cast<MD_SIZE>(sst.str().size()),
            process_output,
            &ofs,
            MD_DIALECT_GITHUB,
            0);
    article->write_footer(ofs);

    if (!article->is_hidden())
    {
      categories.merge(article->get_categories());
      for (const auto& category : article->get_categories())
        category_map[category].emplace_back(article);
    }
  }

  if (!args.index) return 0;

  indexer.sort();

  std::ofstream rss(args.output_dir / "rss.xml");
  indexer.create_rss(rss, "index");

  std::ofstream atom(args.output_dir / "atom.xml");
  indexer.create_atom(atom, "index");

  std::ofstream index(args.output_dir / "index.html");
  indexer.create_index(index, "blog", categories);

  for (const auto& [category, articles] : category_map)
  {
    auto ctgry = category;
    std::ofstream ost(args.output_dir / (normalize(ctgry) + ".html"));
    indexer.create_index(ost, category, {});
  }

  std::ofstream robots(args.output_dir / "robots.txt");
  indexer.create_robots(robots);

  std::ofstream sitemap(args.output_dir / "sitemap.xml");
  indexer.create_sitemap(sitemap);

  return 0;
}
