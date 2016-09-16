#ifndef ELFSPY_FUNCTION_H
#define ELFSPY_FUNCTION_H

namespace spy
{

/**
 * @namespace spy
 * @union Function
 * Conversion helper from function to address
 */

template <typename R, typename... Args>
union Function
{
  R (*address_)(Args...);
  void* pointer_;
};

} // namespace spy

#endif
