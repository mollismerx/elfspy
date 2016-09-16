#include "elfspy/Error.h"

#include <iostream>

namespace
{
std::function<void(const char*)> reporter =
  [](const char* text)
  {
    std::cerr << text << std::endl;
  };

} // namespace

namespace spy
{

void error(const char* text)
{
  reporter(text);
}

void set_reporter(const std::function<void(const char*)>& reporter)
{
  ::reporter = reporter;
}

} // namespace spy
