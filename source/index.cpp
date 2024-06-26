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

void create_index(std::ostream& ost,
                  const std::string& name,
                  const article_list& articles,
                  const categories_t& categories)
{
  using namespace hemplate;  // NOLINT

  const article index(name, categories);

  index.write_header(ost);
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
  index.write_footer(ost);
}

void create_atom(std::ostream& ost,
                 const std::string& name,
                 const article_list& articles)
{
  using namespace hemplate;  // NOLINT

  static const char* base    = "https://dimitrijedobrota.com/blog";
  static const char* loc     = "https://dimitrijedobrota.com/blog/atom.feed";
  static const char* summary = "Click on the article link to read...";

  auto const time =
      std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

  const elementList content = std::accumulate(
      begin(articles),
      end(articles),
      elementList(),
      [](elementList&& list, const auto& article)
      {
        const auto filename = article->get_filename();
        const auto date     = article->get_date();

        list.add(atom::entry()
                     .add(atom::title(filename))
                     .add(atom::link(" ").set(
                         "href", std::format("{}/{}", base, filename)))
                     .add(atom::updated(date))
                     .add(atom::summary(summary)));
        return std::move(list);
      });

  ost << xml();
  ost << atom::feed();
  ost << atom::title(name);
  ost << atom::link(" ").set("href", base);
  ost << atom::link(" ", {{"rel", "self"}, {"href", loc}});
  ost << atom::id(base);
  ost << atom::updated(std::format("{:%Y-%m-%d %X}", time));
  ost << atom::author().add(atom::name(name));
  ost << content;
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
  static const char* loc = "https://dimitrijedobrota.com/blog/index.rss";

  const elementList content = std::accumulate(
      begin(articles),
      end(articles),
      elementList(),
      [](elementList&& list, const auto& article)
      {
        const auto filename = article->get_filename();
        const auto date     = article->get_date();

        list.add(rss::item()
                     .add(rss::title(filename))
                     .add(rss::link(std::format("{}/{}", base, filename)))
                     .add(rss::guid(std::format("{}/{}", base, filename)))
                     .add(rss::pubDate(date))
                     .add(rss::author(std::format("{} ({})", email, author))));
        return std::move(list);
      });

  ost << xml();
  ost << rss::rss();
  ost << rss::channel();
  ost << rss::title(name);
  ost << rss::link(base);
  ost << rss::description(description);
  ost << rss::generator("stamd");
  ost << rss::language("en-us");
  ost << rss::atomLink().set("href", loc);
  ost << content;
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
