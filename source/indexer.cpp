#include <algorithm>
#include <ctime>
#include <format>
#include <iomanip>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>

#include "indexer.hpp"

#include <hemplate/atom.hpp>
#include <hemplate/html.hpp>
#include <hemplate/rss.hpp>
#include <hemplate/sitemap.hpp>

#include "article.hpp"

namespace
{

int64_t parse_time(const std::string& date)
{
  std::tm tms = {};
  std::stringstream stream(date);
  stream >> std::get_time(&tms, "%Y-%m-%d");
  return std::mktime(&tms);
}

}  // namespace

namespace stamd
{

void Indexer::add(const article_s& article)
{
  m_articles.emplace_back(article);
}

void Indexer::add(categories_t categories)
{
  m_categories.merge(categories);
}

void Indexer::sort()
{
  std::sort(
      begin(m_articles),
      end(m_articles),
      [](const auto& lft, const auto& rht)
      {
        return lft->get_date() > rht->get_date();
      }
  );
}

void Indexer::create_index(std::ostream& ost, const std::string& doc_title)
{
  using namespace hemplate::html;  // NOLINT

  const Article index(doc_title, m_options, m_categories);

  ost << index.write(
      [&]
      {
        return element {
            h1 {doc_title},
            ul {
                {{"class", "index"}},
                transform(
                    m_articles,
                    [](const auto& article) -> element
                    {
                      if (article->is_hidden()) {
                        return {};
                      }

                      return li {
                          span {article->get_date(), " -&nbsp"},
                          aHref {article->get_filename(), article->get_title()},
                      };
                    }
                ),
            },
        };
      }
  );
}

void Indexer::create_atom(std::ostream& ost, const std::string& doc_title) const
{
  using namespace hemplate::atom;  // NOLINT
  using hemplate::atom::link;

  const std::string& base_url = m_options.base_url;
  const std::string& author_name = m_options.author;

  ost << element {
      xml {},
      feed {},
      title {doc_title},
      id {base_url},
      updated {format_time_now()},
      author {name {author_name}},
      linkSelf {base_url + "/atom.xml"},
      linkAlternate {base_url + "blog/atom.xml"},
      feed {
          transform(
              m_articles,
              [&](const auto& article)
              {
                const auto filename = article->get_filename();
                const auto art_title = article->get_title();
                const auto date = article->get_date();
                const auto art_summary =
                    article->get("summary").value_or(m_options.summary);

                return entry {
                    title {art_title},
                    id {base_url + filename},
                    linkHref {base_url + filename},
                    updated {format_time(parse_time((date)))},
                    summary {art_summary},
                };
              }
          ),
      },
  };
}

void Indexer::create_rss(std::ostream& ost, const std::string& doc_title) const
{
  using namespace hemplate::rss;  // NOLINT
  using hemplate::rss::link;

  const std::string& base_url = m_options.base_url;
  const std::string& desc = m_options.description;

  ost << element {
      xml {},
      rss {
          channel {
              title {doc_title},
              link {base_url},
              description {desc},
              generator {"stamd"},
              language {"en-us"},
              atomLink {base_url + "blog/rss.xml"},
              transform(
                  m_articles,
                  [&](const auto& article)
                  {
                    const auto filename = article->get_filename();
                    const auto date = article->get_date();
                    const auto author_name =
                        article->get("author").value_or(m_options.author);
                    const auto email =
                        article->get("email").value_or(m_options.email);

                    return item {
                        title {filename},
                        link {base_url + filename},
                        guid {base_url + filename},
                        pubDate {format_time(parse_time(date))},
                        author {std::format("{} ({})", email, author_name)},
                    };
                  }
              ),
          },
      },
  };
}

void Indexer::create_sitemap(std::ostream& ost) const
{
  using namespace hemplate::sitemap;  // NOLINT

  static const std::string& base_url = m_options.base_url;

  ost << element {
      xml {},
      urlset {
          transform(
              m_articles,
              [&](const auto& article)
              {
                const auto& filename = article->get_filename();
                const auto& date = article->get_date();

                return url {
                    loc {base_url + filename},
                    lastmod {date},
                };
              }
          ),
      }
  };
}

void Indexer::create_robots(std::ostream& ost) const
{
  static const std::string& base_url = m_options.base_url;

  ost << "User-agent: *";
  ost << std::format("Sitemap: {}/sitemap.xml", base_url);
}

}  // namespace stamd
