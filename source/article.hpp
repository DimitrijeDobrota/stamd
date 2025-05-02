#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>

#include <hemplate/html.hpp>

#include "options.hpp"

namespace stamd
{

class Article
{
public:
  using symbols_t = std::unordered_map<std::string, std::string>;
  using categories_t = std::set<std::string>;

  explicit Article(
      std::string filename, options_t options, categories_t categories = {}
  )
      : m_filename(std::move(filename))
      , m_categories(std::move(categories))
      , m_options(std::move(options))
  {
  }

  using content_t = std::function<hemplate::element()>;
  hemplate::element write(const content_t& content) const;

  void insert(const std::string& category) { m_categories.emplace(category); }
  void insert(const std::string& key, const std::string& value)
  {
    m_symbols.insert_or_assign(key, value);
  }

  auto get_categories() const { return m_categories; }

  void set_hidden() { m_hidden = true; }
  void set_nonav() { m_nonav = true; }

  bool is_hidden() const { return m_hidden; }

  std::optional<std::string> get(const std::string& key) const;

  std::string get_filename() const;
  std::string get_date() const;
  std::string get_title() const;
  std::string get_language() const;
  std::string get_desciprtion() const;
  std::string get_author() const;
  std::string get_keywords() const;

private:
  static hemplate::element print_nav(const std::string& base);
  static hemplate::element print_categories(const categories_t& categories);

  bool m_hidden = false;
  bool m_nonav = false;

  std::string m_filename;
  categories_t m_categories;
  options_t m_options;
  symbols_t m_symbols;
};

}  // namespace stamd
