#include <format>
#include <numeric>

#include "article.hpp"

#include <hemplate/attribute.hpp>
#include <hemplate/classes.hpp>

#include "utility.hpp"

void article::print_nav(std::ostream& ost)
{
  using namespace hemplate;  // NOLINT

  static const char* base = "https://dimitrijedobrota.com/blog";

  ost << html::div()
             .add(html::nav()
                      .add(html::a("&lt;-- back", {{"class", "back"}}))
                      .add(html::a("index", {{"href", base}}))
                      .add(html::a("hime --&gt;", {{"href", "/"}})))
             .add(html::hr());
}

void article::print_categories(std::ostream& ost,
                               const categories_t& categories)
{
  using namespace hemplate;  // NOLINT

  ost << html::div(
      attributeList({{"class", "categories"}}),
      std::accumulate(
          begin(categories),
          end(categories),
          elementList(html::h3("Categories: "), html::p()),
          [](elementList&& list, std::string ctgry)
          {
            normalize(ctgry);
            list.add(
                html::a(ctgry, {{"href", std::format("./{}.html", ctgry)}}));
            return std::move(list);
          })
          .add(html::p()));
}

void article::write(const std::string& data, std::ostream& ost)
{
  using namespace hemplate;  // NOLINT

  static const char* description_s =
      "Dimitrije Dobrota's personal site. You can find my daily findings in a "
      "form of articles on my blog as well as various programming projects.";

  static const attributeList icon  = {{"rel", "icon"}, {"type", "image/png"}};
  static const attributeList style = {{"rel", "stylesheet"},
                                      {"type", "text/css"}};

  static const attributeList viewport = {
      {"content", "width=device-width, initial-scale=1"},
      {"name", "viewport"}};

  static const attributeList description = {{"name", "description"},
                                            {"content", description_s}};

  ost << html::html().set("lang", get_language());
  ost << html::head()
             .add(html::title(get_title()))
             .add(html::meta(viewport))
             .add(html::meta({{"charset", "UTF-8"}}))
             .add(html::meta(description))
             .add(html::link(style).set("href", "/css/index.css"))
             .add(html::link(style).set("href", "/css/colors.css"))
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
  ost << html::main();
  ost << html::div().set("class", "content");
  ost << html::label(" ")
             .set("for", "theme_switch")
             .set("class", "switch_label");

  if (!m_nonav) print_nav(ost);
  if (!m_categories.empty()) print_categories(ost, m_categories);

  ost << data;

  if (!m_nonav) print_nav(ost);

  ost << html::div();
  ost << html::main();
  ost << html::script(" ").set("source", "/scripts/main.js");
  ost << html::body();
  ost << html::html();
}
