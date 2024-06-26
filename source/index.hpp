#pragma once

#include <memory>
#include <string>

#include "article.hpp"

namespace stamd {

using article_list = std::vector<std::shared_ptr<article>>;
using categories_t = article::categories_t;

void create_robots(std::ostream& ost);

void create_sitemap(std::ostream& ost, const article_list& articles);

void create_atom(std::ostream& ost,
                 const std::string& name,
                 const article_list& articles);

void create_rss(std::ostream& ost,
                const std::string& name,
                const article_list& articles);

void create_index(std::ostream& ost,
                  const std::string& name,
                  const article_list& articles,
                  const categories_t& categories);

}  // namespace stamd
