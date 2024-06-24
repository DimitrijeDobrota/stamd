#pragma once

#include <algorithm>
#include <string>

inline std::string& ltrim(std::string& str)
{
  str.erase(
      str.begin(),
      std::find_if(str.begin(),
                   str.end(),
                   [](unsigned char chr) { return std::isspace(chr) == 0; }));
  return str;
}

inline std::string& rtrim(std::string& str)
{
  str.erase(
      std::find_if(str.rbegin(),
                   str.rend(),
                   [](unsigned char chr) { return std::isspace(chr) == 0; })
          .base(),
      str.end());
  return str;
}

inline std::string& trim(std::string& str)
{
  return rtrim(ltrim(str));
}

inline std::string& normalize(std::string& str)
{
  std::replace(str.begin(), str.end(), ' ', '_');
  return str;
}
