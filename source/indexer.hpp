#pragma once

#include <memory>
#include <string>
#include <vector>

#include "article.hpp"
#include "options.hpp"

namespace stamd {

class Indexer
{
public:
  using article_s = std::shared_ptr<Article>;

  using article_list = std::vector<article_s>;
  using categories_t = Article::categories_t;

  explicit Indexer(options_t options)
      : m_options(std::move(options))
  {
  }

  void add(const article_s& article);
  void add(categories_t categories);

  void sort();

  void create_robots(std::ostream& ost) const;
  void create_sitemap(std::ostream& ost) const;

  void create_atom(std::ostream& ost, const std::string& name) const;
  void create_rss(std::ostream& ost, const std::string& name) const;
  void create_index(std::ostream& ost, const std::string& name);

private:
  options_t m_options;
  categories_t m_categories;

  article_list m_articles;
};

}  // namespace stamd
