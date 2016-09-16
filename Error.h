#ifndef ELFSPY_ERROR_H
#define ELFSPY_ERROR_H

#include <functional>

namespace spy
{

void error(const char* text);
void set_reporter(const std::function<void(const char*)>& reporter);

} // namespace spy

#endif
