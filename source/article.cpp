#include <format>
#include <iterator>
#include <numeric>
#include <optional>
#include <string>

#include "article.hpp"

#include <hemplate/attribute.hpp>
#include <hemplate/classes.hpp>

#include "utility.hpp"

namespace stamd
{

std::optional<std::string> Article::get(const std::string& key) const
{
  const auto itr = m_symbols.find(key);
  if (itr == end(m_symbols)) {
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
    if (categories.empty()) {
      return std::string();
    }

    return std::accumulate(
        std::next(std::begin(categories)),
        std::end(categories),
        *categories.begin(),
        [](const auto& acc, const auto& str)
        {
          return acc + ", " + str;
        }
    );
  };

  return get("keywords").value_or(concat(m_categories));
}

hemplate::element Article::print_nav(const std::string& base)
{
  using namespace hemplate::html;  // NOLINT

  return nav {
      a {
          {{"class", "back"}},
          "&lt;-- back",
      },
      a {
          {{"href", base}},
          "index",
      },
      a {
          {{"href", "/"}},
          "home --&gt;",
      },
  };
}

hemplate::element Article::print_categories(const categories_t& categories)
{
  using namespace hemplate::html;  // NOLINT

  return nav {
      {{"class", "categories"}},
      h3 {"Categories: "},
      p {
          transform(
              categories,
              [](const auto& category)
              {
                auto ctgry = category;
                normalize(ctgry);
                return aHref {
                    std::format("./{}.html", ctgry),
                    category,
                };
              }
          ),
      },
  };
}

hemplate::element Article::write(const content_t& content) const
{
  using namespace hemplate::html;  // NOLINT

  return element {
    doctype {},
      html {
          {{"lang", get_language()}},
          head {
              title {get_title()},

              metaUTF8 {},
              metaName {"author", get_author()},
              metaName {"description", get_desciprtion()},
              metaName {"keywords", get_keywords()},
              metaName {"viewport", "width=device-width, initial-scale=1"},

              linkStylesheet {"/css/index.css"},
              linkStylesheet {"/css/colors.css"},

              element {
                  linkRss {"RSS feed", "/blog/rss.xml"},
                  linkAtom {"Atom feed", "/blog/atom.xml"},
              },

              // Icons
              linkIcon {"32x32", "/img/favicon-32x32.png"},
              linkIcon {"16x16", "/img/favicon-16x16.png"},
          },
          body {
              input {{
                  {"type", "checkbox"},
                  {"id", "theme_switch"},
                  {"class", "theme_switch"},
              }},
              hemplate::html::div {
                  {{"id", "content"}},
                  m_nonav ? element{} : [&] {
                      return header {
                          print_nav(m_options.base_url + "blog"),
                          hr{},
                      };
                  }(),
                  main {
                      label {
                          {{"for", "theme_switch"}, {"class", "switch_label"},},
                      },
                      m_categories.empty() ? element {} : [&]() {
                          return print_categories(m_categories);
                      }(),
                      content(),
                  },
                  m_nonav ? element{} : [&] {
                      return footer {
                          hr{},
                          print_nav(m_options.base_url + "blog"),
                      };
                  }(),
              },
              script {{{"set", "/scripts/main.js"}}},
          },
      },
  };
}

}  // namespace stamd
