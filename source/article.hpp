#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class article
{
public:
  using symbols_t    = std::unordered_map<std::string, std::string>;
  using categories_t = std::set<std::string>;

  explicit article(std::string name, categories_t categories = {})
      : m_name(std::move(name))
      , m_categories(std::move(categories))
  {
  }

  void write(const std::string& data, std::ostream& ost);
  void emplace(const std::string& category) { m_categories.emplace(category); }
  void emplace(const std::string& key, const std::string& value)
  {
    m_symbols.emplace(key, value);
  }

  auto get_categories() const { return m_categories; }

  void set_hidden(bool state) { m_hidden = state; }
  void set_nonav(bool state) { m_nonav = state; }

  bool is_hidden() const { return m_hidden; }

  const std::string& get_language() { return m_symbols.find("lang")->second; }
  const std::string& get_title() { return m_symbols.find("title")->second; }
  const std::string& get_date() { return m_symbols.find("date")->second; }

private:
  static void print_nav(std::ostream& ost);
  static void print_categories(std::ostream& ost,
                               const categories_t& categories);

  std::string m_name;

  bool m_hidden = false;
  bool m_nonav  = false;

  categories_t m_categories;
  symbols_t m_symbols = {
      {"title", "test"}, {"lang", "en"}, {"date", "1970-01-01"}};
};
