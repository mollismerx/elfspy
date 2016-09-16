#ifndef ELFSPY_GOTENTRY_H
#define ELFSPY_GOTENTRY_H

#include <typeinfo>
#include <vector>

#include "elfspy/MethodPointer.h"

namespace spy
{

/**
 * @namespace spy
 * @class GOTEntry
 * Handle a function of interest and all its references in all Global Offset
 * Tables
 */

class GOTEntry
{
public:
  GOTEntry() = default;
  /**
   * @param function address of function
   * @return original unaltered address of function
   */
  void* set(void* function);
  /**
   * replace function with other function and reference count
   * @param function to enter in Global Offset Tables instead of original
   * @return original entry in Global Offset Table instead of original
   */
  void* spy_with(void* function);
  /**
   * count down reference count from spy_with and restore original entry in
   * Global Offset Tables if count is zero
   */
  void restore();
  /**
   * replace function with other function
   * @param function to enter in Global Offset Tables instead of original
   * @return original entry in Global Offset Table instead of original
   */
  void* patch(void* function);
  /**
   * get the address of a virtual function
   * @param type type info of class
   * @param method method function pointer
   * @return address of address of virtual function for that class
   */
  static void** get_vtable_entry(const std::type_info& type,
                                 const MethodPointer& method);

  static void initialise(int argc, char** argv);

  const void* current() const;
  const void* original() const;
  /**
   * add an entry to the function pointer addresses
   * @param address address of address of function
   */
  void make_entry(void** address);

private:
  struct Entry
  {
    void** address_;
    void* restore_;
  };
  std::vector<Entry> entries_; // all entries
  size_t spy_count_ = 0;
  void* current_ = nullptr;
  void* original_ = nullptr;
  void make_entry(unsigned char* address);
};

inline const void* GOTEntry::current() const
{
  return original_;
}

inline const void* GOTEntry::original() const
{
  return original_;
}

inline void GOTEntry::make_entry(unsigned char* address)
{
  make_entry(reinterpret_cast<void**>(address));
}

} // namespace spy

#endif
