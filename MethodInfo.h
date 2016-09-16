#ifndef ELFSPY_METHODINFO_H
#define ELFSPY_METHODINFO_H

namespace spy
{

/**
 * @class MethodInfo
 * contains the resolved function address of a method and the address of it's
 * vtable entry if the method is virtual
 */

template <typename ReturnType, typename... ArgTypes>
struct MethodInfo
{
  union
  {
    ReturnType (*function_)(ArgTypes...);
    void* address_;
  };
  void** vtable_entry_;
};

} // namespace spy

#endif
