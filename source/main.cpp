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
#include "options.hpp"
#include "utility.hpp"

void preprocess(stamd::Article& article, std::istream& ist)
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

  stamd::options_t options;
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
    case poafloc::END:
      if (args->options.base_url.empty()
          || args->options.base_url.back() != '/')
      {
        args->options.base_url += '/';
      }
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

  using category_map_t = std::unordered_map<std::string, Indexer>;

  category_map_t category_map;
  Indexer index(args.options);

  for (const auto& path : args.files)
  {
    const std::string filename = path.stem().string() + ".html";

    const auto article = make_shared<stamd::Article>(filename, args.options);
    index.add(article);

    std::ifstream ifs(path.string());
    preprocess(*article, ifs);

    std::stringstream sst;
    sst << ifs.rdbuf();

    // filename can change in preprocessing phase
    std::ofstream ofs(args.output_dir / article->get_filename());

    article->write_header(ofs);
    md_html(sst.str().c_str(),
            static_cast<MD_SIZE>(sst.str().size()),
            process_output,
            &ofs,
            MD_DIALECT_GITHUB,
            0);
    article->write_footer(ofs);

    if (article->is_hidden()) continue;

    index.add(article->get_categories());
    for (const auto& category : article->get_categories())
    {
      auto [it, _] = category_map.emplace(category, args.options);
      it->second.add(article);
    }
  }

  if (!args.index) return 0;

  index.sort();

  std::ofstream ofs_rss(args.output_dir / "rss.xml");
  index.create_rss(ofs_rss, "index");

  std::ofstream ofs_atom(args.output_dir / "atom.xml");
  index.create_atom(ofs_atom, "index");

  std::ofstream ofs_index(args.output_dir / "index.html");
  index.create_index(ofs_index, "blog");

  for (auto& [category_name, category_index] : category_map)
  {
    auto ctgry = category_name;
    std::ofstream ost(args.output_dir / (normalize(ctgry) + ".html"));

    category_index.sort();
    category_index.create_index(ost, category_name);
  }

  std::ofstream ofs_robots(args.output_dir / "robots.txt");
  index.create_robots(ofs_robots);

  std::ofstream ofs_sitemap(args.output_dir / "sitemap.xml");
  index.create_sitemap(ofs_sitemap);

  return 0;
}
