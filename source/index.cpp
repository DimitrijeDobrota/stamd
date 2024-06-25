#include <algorithm>
#include <format>
#include <fstream>
#include <numeric>
#include <sstream>

#include "index.hpp"

#include <hemplate/attribute.hpp>
#include <hemplate/classes.hpp>

namespace stamd {

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

void create_atom(std::ostream& ost,
                 const std::string& name,
                 const article_list& articles)
{
  using namespace hemplate;  // NOLINT

  static const char* base    = "https://dimitrijedobrota.com/blog";
  static const char* loc     = "https://dimitrijedobrota.com/blog/atom.feed";
  static const char* updated = "2003-12-13T18:30:02Z";
  static const char* summary = "Click on the article link to read...";

  const elementList content = std::accumulate(
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

  ost << xml();
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

void create_rss(std::ostream& ost,
                const std::string& name,
                const article_list& articles)
{
  using namespace hemplate;  // NOLINT
  static const char* author      = "Dimitrije Dobrota";
  static const char* email       = "mail@dimitrijedobrota.com";
  static const char* base        = "https://dimitrijedobrota.com/blog";
  static const char* description = "Contents of Dimitrije Dobrota's webpage";
  static const char* loc     = "https://dimitrijedobrota.com/blog/index.rss";
  static const char* updated = "2003-12-13T18:30:02Z";

  const elementList content = std::accumulate(
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
    const auto& title = article->get_title();
    const auto& date  = article->get_date();

    ost << sitemap::url()
               .add(sitemap::loc(std::format("{}/{}", base, title)))
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
