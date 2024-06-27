#pragma once

#include <memory>
#include <string>

#include "article.hpp"

namespace stamd {

class indexer
{
public:
  using article_s = std::shared_ptr<article>;

  using article_list = std::vector<article_s>;
  using categories_t = article::categories_t;

  struct options_t
  {
    std::string base_url;
    std::string author;
    std::string email;
    std::string description;
    std::string summary;
  };

  explicit indexer(options_t options)
      : m_options(std::move(options))
  {
    if (m_options.base_url.empty() || m_options.base_url.back() != '/')
    {
      m_options.base_url += '/';
    }
  }

  article_s& add(const article_s& article);

  void sort();

  void create_robots(std::ostream& ost) const;
  void create_sitemap(std::ostream& ost) const;

  void create_atom(std::ostream& ost, const std::string& name) const;
  void create_rss(std::ostream& ost, const std::string& name) const;
  void create_index(std::ostream& ost,
                    const std::string& name,
                    const categories_t& categories);

  void create_categories() const;

private:
  options_t m_options;

  article_list m_articles;
};

}  // namespace stamd
