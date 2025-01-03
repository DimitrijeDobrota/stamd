#pragma once

#include <string>

namespace stamd {

struct options_t
{
  std::string base_url;  // url with trailing '/'
  std::string author;
  std::string email;
  std::string description;
  std::string summary;
};

}  // namespace stamd
