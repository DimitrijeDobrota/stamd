#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>
#include <numeric>
#include <sstream>

#include "index.hpp"

#include <hemplate/attribute.hpp>
#include <hemplate/classes.hpp>

namespace stamd {

std::tm get_time(const std::string& date)
{
  int year  = 0;
  int month = 0;
  int day   = 0;

  std::sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day);

  tm time = {
      .tm_sec  = 0,
      .tm_min  = 0,
      .tm_hour = 0,
      .tm_mday = day,
      .tm_mon  = month - 1,
      .tm_year = year - 1900,
  };

  return time;
}

#define rfc882_f "{:%a, %d %b %Y %H:%M:%S %z}"  // NOLINT
#define rfc3339_f "{:%FT%H:%M:%SZ}"  // NOLINT

std::string to_rfc882(const std::string& date)
{
  using namespace std::chrono;  // NOLINT

  tm time = get_time(date);

  const auto tmp = std::mktime(&time);
  const auto chrono_time =
      time_point_cast<seconds>(system_clock::from_time_t(tmp));

  return std::format(rfc882_f, chrono_time);
}

std::string to_rfc3339(const std::string& date)
{
  using namespace std::chrono;  // NOLINT

  tm time = get_time(date);

  const auto tmp = std::mktime(&time);
  const auto chrono_time =
      time_point_cast<seconds>(system_clock::from_time_t(tmp));

  return std::format(rfc3339_f, chrono_time);
}

std::shared_ptr<article> create_index(std::ostream& ost,
                                      const std::string& name,
                                      const article_list& articles,
                                      const categories_t& categories)
{
  using namespace hemplate;  // NOLINT

  auto index = std::make_shared<article>(name, categories);

  index->write_header(ost);
  ost << html::h1(name);
  ost << html::ul().set("class", "index");
  for (const auto& article : articles)
  {
    if (article->is_hidden()) continue;

    const auto& filename = article->get_filename();
    const auto& title    = article->get_title();
    const auto& date     = article->get_date();

    ost << html::li()
               .add(html::span(std::format("{} -&nbsp", date)))
               .add(html::a(title).set("href", filename));
  };
  ost << html::ul();
  index->write_footer(ost);

  return index;
}

void create_atom(std::ostream& ost,
                 const std::string& name,
                 const article_list& articles)
{
  using namespace hemplate;  // NOLINT

  static const char* base    = "https://dimitrijedobrota.com/blog";
  static const char* loc     = "https://dimitrijedobrota.com/blog/atom.xml";
  static const char* summary = "Click on the article link to read...";

  auto const time =
      std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

  ost << xml();
  ost << atom::feed();
  ost << atom::title(name);
  ost << atom::id(base);
  ost << atom::updated(std::format(rfc3339_f, time));
  ost << atom::author().add(atom::name(name));
  ost << atom::link(" ", {{"rel", "self"}, {"href", loc}});
  ost << atom::link(
      " ", {{"href", base}, {"rel", "alternate"}, {"type", "text/html"}});

  for (const auto& article : articles)
  {
    const auto filename = article->get_filename();
    const auto title    = article->get_title();
    const auto date     = article->get_date();
    const auto path     = std::format("{}/{}", base, filename);

    ost << atom::entry()
               .add(atom::title(title))
               .add(atom::id(path))
               .add(atom::link(" ").set("href", path))
               .add(atom::updated(to_rfc3339(date)))
               .add(atom::summary(summary));
  }

  ost << atom::feed();
}

void create_rss(std::ostream& ost,
                const std::string& name,
                const article_list& articles)
{
  using namespace hemplate;  // NOLINT

  static const char* author      = "Dimitrije Dobrota";
  static const char* email       = "mail@dimitrijedobrota.com";
  static const char* base        = "https://dimitrijedobrota.com/blog";
  static const char* description = "Contents of Dimitrije Dobrota's webpage";
  static const char* loc         = "https://dimitrijedobrota.com/blog/rss.xml";

  ost << xml();
  ost << rss::rss();
  ost << rss::channel();
  ost << rss::title(name);
  ost << rss::link(base);
  ost << rss::description(description);
  ost << rss::generator("stamd");
  ost << rss::language("en-us");
  ost << rss::atomLink().set("href", loc);

  for (const auto& article : articles)
  {
    const auto filename = article->get_filename();
    const auto date     = article->get_date();

    ost << rss::item()
               .add(rss::title(filename))
               .add(rss::link(std::format("{}/{}", base, filename)))
               .add(rss::guid(std::format("{}/{}", base, filename)))
               .add(rss::pubDate(to_rfc882(date)))
               .add(rss::author(std::format("{} ({})", email, author)));
  }

  ost << rss::channel();
  ost << rss::rss();
}

void create_sitemap(std::ostream& ost, const article_list& articles)
{
  using namespace hemplate;  // NOLINT

  static const char* base = "https://dimitrijedobrota.com/blog";

  ost << xml();
  ost << sitemap::urlset();
  for (const auto& article : articles)
  {
    const auto& name = article->get_filename();
    const auto& date = article->get_date();

    ost << sitemap::url()
               .add(sitemap::loc(std::format("{}/{}.html", base, name)))
               .add(sitemap::lastmod(date));
  }
  ost << sitemap::urlset();
}

void create_robots(std::ostream& ost)
{
  static const char* base = "https://dimitrijedobrota.com/blog";

  ost << "User-agent: *";
  ost << std::format("Sitemap: {}/sitemap.xml", base);
}

}  // namespace stamd
