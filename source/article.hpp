#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>

#include "options.hpp"

namespace stamd {

class Article
{
public:
  using symbols_t    = std::unordered_map<std::string, std::string>;
  using categories_t = std::set<std::string>;

  explicit Article(std::string filename,
                   options_t options,
                   categories_t categories = {})
      : m_filename(std::move(filename))
      , m_categories(std::move(categories))
      , m_options(std::move(options))
  {
  }

  void write_header(std::ostream& ost) const;
  void write_footer(std::ostream& ost) const;

  void insert(const std::string& category) { m_categories.emplace(category); }
  void insert(const std::string& key, const std::string& value)
  {
    m_symbols.insert_or_assign(key, value);
  }

  auto get_categories() const { return m_categories; }

  void set_hidden(bool state) { m_hidden = state; }
  void set_nonav(bool state) { m_nonav = state; }

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
  static void print_nav(std::ostream& ost, const std::string& base);
  static void print_categories(std::ostream& ost,
                               const categories_t& categories);

  bool m_hidden = false;
  bool m_nonav  = false;

  std::string m_filename;
  categories_t m_categories;
  options_t m_options;
  symbols_t m_symbols;
};

}  // namespace stamd
