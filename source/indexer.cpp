#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <iomanip>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>

#include "indexer.hpp"

#include <hemplate/attribute.hpp>
#include <hemplate/classes.hpp>

#include "article.hpp"

namespace stamd {

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
  std::sort(begin(m_articles),
            end(m_articles),
            [](const auto& lft, const auto& rht)
            { return lft->get_date() > rht->get_date(); });
}

int64_t parse_time(const std::string& date)
{
  std::tm tms = {};
  std::stringstream stream(date);
  stream >> std::get_time(&tms, "%Y-%m-%d");
  return std::mktime(&tms);
}

void Indexer::create_index(std::ostream& ost, const std::string& name)
{
  using namespace hemplate;  // NOLINT

  const Article index(name, m_options, m_categories);

  index.write_header(ost);
  ost << html::h1(name);
  ost << html::ul().set("class", "index");
  for (const auto& article : m_articles)
  {
    if (article->is_hidden()) continue;

    const auto& filename = article->get_filename();
    const auto& title    = article->get_title();
    const auto& date     = article->get_date();

    ost << html::li()
               .add(html::span(date + " -&nbsp"))
               .add(html::a(title).set("href", filename));
  };
  ost << html::ul();
  index.write_footer(ost);
}

void Indexer::create_atom(std::ostream& ost, const std::string& name) const
{
  using namespace hemplate;  // NOLINT

  const std::string& base_url = m_options.base_url;

  auto const time = std::chrono::current_zone()
                        ->to_local(std::chrono::system_clock::now())
                        .time_since_epoch()
      / std::chrono::seconds(1);

  ost << xml();
  ost << atom::feed();
  ost << atom::title(name);
  ost << atom::id(base_url);
  ost << atom::updated(atom::format_time(time));
  ost << atom::author().add(atom::name(name));
  ost << atom::link(" ",
                    {{"rel", "self"}, {"href", base_url + "blog/atom.xml"}});
  ost << atom::link(
      " ", {{"href", base_url}, {"rel", "alternate"}, {"type", "text/html"}});

  for (const auto& article : m_articles)
  {
    const auto filename = article->get_filename();
    const auto title    = article->get_title();
    const auto date     = article->get_date();
    const auto summary  = article->get("summary").value_or(m_options.summary);

    ost << atom::entry()
               .add(atom::title(title))
               .add(atom::id(base_url + filename))
               .add(atom::link(" ").set("href", base_url + filename))
               .add(atom::updated(atom::format_time(parse_time((date)))))
               .add(atom::summary(summary));
  }

  ost << atom::feed();
}

void Indexer::create_rss(std::ostream& ost, const std::string& name) const
{
  using namespace hemplate;  // NOLINT

  const std::string& base_url    = m_options.base_url;
  const std::string& description = m_options.description;

  ost << xml();
  ost << rss::rss();
  ost << rss::channel();

  ost << rss::title(name);
  ost << rss::link(base_url);
  ost << rss::description(description);
  ost << rss::generator("stamd");
  ost << rss::language("en-us");
  ost << rss::atomLink().set("href", base_url + "blog/rss.xml");

  for (const auto& article : m_articles)
  {
    const auto filename = article->get_filename();
    const auto date     = article->get_date();
    const auto author   = article->get("author").value_or(m_options.author);
    const auto email    = article->get("email").value_or(m_options.email);

    ost << rss::item()
               .add(rss::title(filename))
               .add(rss::link(base_url + filename))
               .add(rss::guid(base_url + filename))
               .add(rss::pubDate(rss::format_time(parse_time(date))))
               .add(rss::author(std::format("{} ({})", email, author)));
  }

  ost << rss::channel();
  ost << rss::rss();
}

void Indexer::create_sitemap(std::ostream& ost) const
{
  using namespace hemplate;  // NOLINT

  static const std::string& base_url = m_options.base_url;

  ost << xml();
  ost << sitemap::urlset();
  for (const auto& article : m_articles)
  {
    const auto& filename = article->get_filename();
    const auto& date     = article->get_date();

    ost << sitemap::url()
               .add(sitemap::loc(base_url + filename))
               .add(sitemap::lastmod(date));
  }
  ost << sitemap::urlset();
}

void Indexer::create_robots(std::ostream& ost) const
{
  static const std::string& base_url = m_options.base_url;

  ost << "User-agent: *";
  ost << std::format("Sitemap: {}/sitemap.xml", base_url);
}

}  // namespace stamd
