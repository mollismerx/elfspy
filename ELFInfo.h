#ifndef ELFSPY_ELFINFO_H
#define ELFSPY_ELFINFO_H

#include <string>
#include <unordered_map>
#include "elfspy/ELFObject.h"

namespace spy
{

/**
 * @namespace spy
 * @class ELFInfo
 * can read data in ELF format and extract information from it
 */

class ELFInfo : public ELFObject
{
public:
  /**
   * @param data ELF data
   * @param name name of ELF data for diagnostics
   */
  ELFInfo(unsigned char* data, const char* name);

  /// @return true if data is in ELF format
  bool is_elf() const;
  struct Symbol
  {
    const char* name_ = nullptr;
    size_t rela_offset_ = 0;
  };
  /**
   * find symbol and offset name in .rela.plt if symbol is defined in file
   * @param value symbol offset value expected to be in symtab
   * @return name and offset (empty if not defined in file)
   */
  Symbol get_symbol_rela(size_t value);
  /**
   * find symbol and offset name in .rela.plt if symbol is defined as STT_IFUNC
   * @param base base address of ELF object in memory
   * @param function function pointer
   * @return name and offset (0 if not defined in file)
   */
  Symbol get_indirect_symbol_rela(unsigned char* base, void* function);
  /**
   * find symbol offset in .rela.dyn if symbol is found as undefined in file
   * @param name symbol name
   * @return offset (0 if not defined in file)
   */
  size_t get_symbol_rela_dyn(const char* name);
  /**
   * remove write protection from the areas in memory that need to change
   * @param base base address in memory
   */
  void unprotect(unsigned char* base);
  /**
   * find vtable adresses from symbol table
   * @param base - offset to return addresses relative to
   * @return map of type name to vtable address
   */
  std::unordered_map<std::string, unsigned char*>
  get_vtables(unsigned char* base = nullptr);

private:
  unsigned char* data_;
  void* find_section(const char* name);
  void unprotect(unsigned char* base, const char* name);
};

} // namespace spy

#endif
