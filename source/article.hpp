#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>

namespace stamd {

class article
{
public:
  using symbols_t    = std::unordered_map<std::string, std::string>;
  using categories_t = std::set<std::string>;

  explicit article(std::string filename, categories_t categories = {})
      : m_categories(std::move(categories))
      , m_symbols({{"filename", filename}})
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

private:
  static void print_nav(std::ostream& ost);
  static void print_categories(std::ostream& ost,
                               const categories_t& categories);

  bool m_hidden = false;
  bool m_nonav  = false;

  categories_t m_categories;
  symbols_t m_symbols;
};

}  // namespace stamd
