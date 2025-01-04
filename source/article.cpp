#include <format>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <string>

#include "article.hpp"

#include <hemplate/attribute.hpp>
#include <hemplate/classes.hpp>

#include "utility.hpp"

namespace stamd {

std::optional<std::string> Article::get(const std::string& key) const
{
  const auto itr = m_symbols.find(key);
  if (itr == end(m_symbols))
  {
    // std::cerr << "Warning: getting invalid value for: " << key << std::endl;
    return {};
  }
  return itr->second;
}

std::string Article::get_filename() const
{
  return m_filename;
}

std::string Article::get_date() const
{
  return get("date").value_or("0000-00-00");
}

std::string Article::get_title() const
{
  return get("title").value_or(get_filename());
}

std::string Article::get_language() const
{
  return get("language").value_or("en");
}

std::string Article::get_desciprtion() const
{
  return get("description").value_or(m_options.description);
}

std::string Article::get_author() const
{
  return get("author").value_or(m_options.author);
}

std::string Article::get_keywords() const
{
  static const auto concat = [](const categories_t& categories)
  {
    if (categories.empty()) return std::string();
    return std::accumulate(std::next(std::begin(categories)),
                           std::end(categories),
                           *categories.begin(),
                           [](const auto& acc, const auto& str)
                           { return acc + ", " + str; });
  };

  return get("keywords").value_or(concat(m_categories));
}

void Article::print_nav(std::ostream& ost, const std::string& base)
{
  using namespace hemplate;  // NOLINT

  ost << html::nav()
             .add(html::a("&lt;-- back", {{"class", "back"}}))
             .add(html::a("index", {{"href", base}}))
             .add(html::a("home --&gt;", {{"href", "/"}}));
}

void Article::print_categories(std::ostream& ost,
                               const categories_t& categories)
{
  using namespace hemplate;  // NOLINT

  ost << html::nav().set("class", "categories");
  ost << html::h3("Categories: ");
  ost << html::p();
  for (const auto& category : categories)
  {
    auto ctgry = category;
    normalize(ctgry);
    ost << html::a(category, {{"href", std::format("./{}.html", ctgry)}});
  }
  ost << html::p();
  ost << html::nav();
}

void Article::write_header(std::ostream& ost) const
{
  using namespace hemplate;  // NOLINT

  ost << html::doctype();
  ost << html::html().set("lang", get_language());
  ost << html::head()
             .add(html::title(get_title()))
             // Meta tags
             .add(html::meta({{"charset", "UTF-8"}}))
             .add(html::meta({{"name", "author"}, {"content", get_author()}}))
             .add(html::meta(
                 {{"name", "description"}, {"content", get_desciprtion()}}))
             .add(html::meta(
                 {{"name", "keywords"}, {"content", get_keywords()}}))
             .add(html::meta(
                 {{"content", "width=device-width, initial-scale=1"},
                  {"name", "viewport"}}))
             // Stylesheets
             .add(html::link({{"rel", "stylesheet"}, {"type", "text/css"}})
                      .set("href", "/css/index.css"))
             .add(html::link({{"rel", "stylesheet"}, {"type", "text/css"}})
                      .set("href", "/css/colors.css"))
             // Rss feed
             .add(html::link({{"rel", "alternate"},
                              {"type", "application/atom+xml"},
                              {"title", "RSS feed"},
                              {"href", "/blog/rss.xml"}}))
             // Atom feed
             .add(html::link({{"rel", "alternate"},
                              {"type", "application/atom+xml"},
                              {"title", "Atom feed"},
                              {"href", "/blog/atom.xml"}}))
             // Icons
             .add(html::link({{"rel", "icon"}, {"type", "image/png"}})
                      .set("sizes", "32x32")
                      .set("href", "/img/favicon-32x32.png"))
             .add(html::link({{"rel", "icon"}, {"type", "image/png"}})
                      .set("sizes", "16x16")
                      .set("href", "/img/favicon-16x16.png"));

  ost << html::body();
  ost << html::input()
             .set("type", "checkbox")
             .set("id", "theme_switch")
             .set("class", "theme_switch");

  ost << html::div().set("id", "content");

  if (!m_nonav)
  {
    ost << html::header();
    print_nav(ost, m_options.base_url + "blog");
    ost << html::hr();
    ost << html::header();
  }

  ost << html::main();
  ost << html::label(" ")
             .set("for", "theme_switch")
             .set("class", "switch_label");

  if (!m_categories.empty()) print_categories(ost, m_categories);
}

void Article::write_footer(std::ostream& ost) const
{
  using namespace hemplate;  // NOLINT

  ost << html::main();

  if (!m_nonav)
  {
    ost << html::footer();
    ost << html::hr();
    print_nav(ost, m_options.base_url + "blog");
    ost << html::footer();
  }

  ost << html::div();
  ost << html::script(" ").set("src", "/scripts/main.js");
  ost << html::body();
  ost << html::html();
}

}  // namespace stamd
