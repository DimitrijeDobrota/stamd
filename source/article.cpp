#include <format>
#include <iostream>
#include <numeric>

#include "article.hpp"

#include <hemplate/attribute.hpp>
#include <hemplate/classes.hpp>

#include "utility.hpp"

std::optional<std::string> article::get(const std::string& key) const
{
  const auto itr = m_symbols.find(key);
  if (itr == end(m_symbols))
  {
    std::cerr << "Warning: getting invalid value for: " << key << std::endl;
    return {};
  }
  return itr->second;
}

std::string article::get_filename() const
{
  return m_symbols.find("filename")->second;
}

std::string article::get_date() const
{
  return get("date").value_or("0000-00-00");
}

std::string article::get_title() const
{
  return get("title").value_or(get_filename());
}

std::string article::get_language() const
{
  return get("language").value_or("en");
}

void article::print_nav(std::ostream& ost)
{
  using namespace hemplate;  // NOLINT

  static const char* base = "https://dimitrijedobrota.com/blog";

  ost << html::nav()
             .add(html::a("&lt;-- back", {{"class", "back"}}))
             .add(html::a("index", {{"href", base}}))
             .add(html::a("home --&gt;", {{"href", "/"}}));
}

void article::print_categories(std::ostream& ost,
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

void article::write_header(std::ostream& ost) const
{
  using namespace hemplate;  // NOLINT

  static const char* description_s =
      "Dimitrije Dobrota's personal site. You can find my daily "
      "findings in a "
      "form of articles on my blog as well as various programming "
      "projects.";

  static const attributeList icon  = {{"rel", "icon"}, {"type", "image/png"}};
  static const attributeList style = {{"rel", "stylesheet"},
                                      {"type", "text/css"}};

  static const attributeList viewport = {
      {"content", "width=device-width, initial-scale=1"},
      {"name", "viewport"}};

  static const attributeList description = {{"name", "description"},
                                            {"content", description_s}};

  static const attributeList rss = {{"rel", "alternate"},
                                    {"type", "application/atom+xml"},
                                    {"title", "RSS feed"},
                                    {"href", "/blog/rss.xml"}};

  static const attributeList atom = {{"rel", "alternate"},
                                     {"type", "application/atom+xml"},
                                     {"title", "Atom feed"},
                                     {"href", "/blog/atom.xml"}};

  ost << html::doctype();
  ost << html::html().set("lang", get_language());
  ost << html::head()
             .add(html::title(get_title()))
             .add(html::meta(viewport))
             .add(html::meta({{"charset", "UTF-8"}}))
             .add(html::meta(description))
             .add(html::link(style).set("href", "/css/index.css"))
             .add(html::link(style).set("href", "/css/colors.css"))
             .add(html::link(rss))
             .add(html::link(atom))
             .add(html::link(icon)
                      .set("sizes", "32x32")
                      .set("href", "/img/favicon-32x32.png"))
             .add(html::link(icon)
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
    print_nav(ost);
    ost << html::hr();
    ost << html::header();
  }

  ost << html::main();
  ost << html::label(" ")
             .set("for", "theme_switch")
             .set("class", "switch_label");

  if (!m_categories.empty()) print_categories(ost, m_categories);
}

void article::write_footer(std::ostream& ost) const
{
  using namespace hemplate;  // NOLINT

  ost << html::main();

  if (!m_nonav)
  {
    ost << html::footer();
    ost << html::hr();
    print_nav(ost);
    ost << html::footer();
  }

  ost << html::div();
  ost << html::script(" ")
             .set("type", "text/javascript")
             .set("src", "/scripts/main.js");
  ost << html::body();
  ost << html::html();
}
