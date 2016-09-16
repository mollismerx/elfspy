#include "elfspy/ELFInfo.h"

#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>
#include <string.h>

#include "elfspy/Report.h"
#include "elfspy/Fail.h"

#ifdef __x86_64__
using Elf_Phdr = Elf64_Phdr;
using Elf_Ehdr = Elf64_Ehdr;
using Elf_Shdr = Elf64_Shdr;
using Elf_Rel = Elf64_Rel;
using Elf_Rela = Elf64_Rela;
using Elf_Sym = Elf64_Sym;
#define ELF_STTYPE(X) ELF64_ST_TYPE(X)
#define ELF_STBIND(X) ELF64_ST_BIND(X)
#define ELF_R_SYM(X) ELF64_R_SYM(X)
#define ELF_R_TYPE(X) ELF64_R_TYPE(X)
#else
using Elf_Phdr = Elf32_Phdr;
using Elf_Ehdr = Elf32_Ehdr;
using Elf_Shdr = Elf32_Shdr;
using Elf_Rel = Elf32_Rel;
using Elf_Rela = Elf32_Rela;
using Elf_Sym = Elf32_Sym;
#define ELF_STTYPE(X) ELF32_ST_TYPE(X)
#define ELF_STBIND(X) ELF32_ST_BIND(X)
#define ELF_R_SYM(X) ELF32_R_SYM(X)
#define ELF_R_TYPE(X) ELF32_R_TYPE(X)
#endif

namespace
{

long page_size = sysconf(_SC_PAGESIZE);
union Address
{
  unsigned char* pointer_;
  size_t value_;
  void round_down();
  void round_up();
};

inline void Address::round_down()
{
  value_ /= page_size;
  value_ *= page_size;
}

inline void Address::round_up()
{
  value_ += page_size - 1;
  value_ /= page_size;
  value_ *= page_size;
}

} // namespace

namespace spy
{

ELFInfo::ELFInfo(unsigned char* data, const char* name)
{
  data_ = data;
  name_ = name;
  if (!is_elf()) {
    Fail() << "Not ELF data - no ELF header found in " << name;
  }
}

bool ELFInfo::is_elf() const
{
  auto elf = reinterpret_cast<Elf_Ehdr*>(data_);
  return elf->e_ident[EI_MAG0] == ELFMAG0
      && elf->e_ident[EI_MAG1] == ELFMAG1
      && elf->e_ident[EI_MAG2] == ELFMAG2
      && elf->e_ident[EI_MAG3] == ELFMAG3;
}

void* ELFInfo::find_section(const char* name)
{
  auto elf = reinterpret_cast<Elf_Ehdr*>(data_);
  auto header = data_ + elf->e_shoff;
  auto section_names =
      reinterpret_cast<Elf_Shdr*>(header + elf->e_shstrndx * elf->e_shentsize);
  auto names = reinterpret_cast<char*>(elf) + section_names->sh_offset;
  // .shstrtab is the "section header" string table, it is indexed in elf header
  for (size_t n = 0; n != elf->e_shnum; ++n, header += elf->e_shentsize) {
    auto section = reinterpret_cast<Elf_Shdr*>(header);
    if (strcmp(names + section->sh_name, name) == 0) {
      return section;
    }
  }
  if (strncmp(name_, "/lib", 4) != 0 && strncmp(name_, "/usr", 4) != 0) {
    Report() << "no " << name << " section found in " << name_;
  }
  return nullptr;
}

void ELFInfo::unprotect(unsigned char* base, const char* name)
{
  auto section = reinterpret_cast<Elf_Shdr*>(find_section(name));
  if (section) {
    Address begin;
    Address end;
    begin.pointer_ = base + section->sh_addr;
    end.pointer_ = begin.pointer_ + section->sh_size;
    begin.round_down();
    end.round_up();
    size_t size = end.value_ - begin.value_;
    if (mprotect(begin.pointer_, size,
                 PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
      spy::Fail() << "Cannot set " << name << " to writable for " << name_;
    }
  }
}

void ELFInfo::unprotect(unsigned char* base)
{
  // by default this memory is read only - ELFspy needs to change it
  unprotect(base, ".got");
  unprotect(base, ".got.plt");
}

ELFInfo::Symbol ELFInfo::get_symbol_rela(size_t value)
{
  Symbol result;
  auto str_tab = reinterpret_cast<Elf_Shdr*>(find_section(".dynstr"));
  if (!str_tab) {
    return result;
  }
  auto strings = reinterpret_cast<const char*>(data_ + str_tab->sh_offset);
  // find dynamic symbol table
  auto symbol_table = reinterpret_cast<Elf_Shdr*>(find_section(".dynsym"));
  if (!symbol_table) {
    return result;
  }
  // find symbol by value in dynamic symbol table
  size_t index = 0;
  auto symbols = data_ + symbol_table->sh_offset;
  auto end = symbols + symbol_table->sh_size;
  for ( ; symbols < end; symbols += symbol_table->sh_entsize) {
    auto symbol = reinterpret_cast<Elf_Sym*>(symbols);
    if (ELF_STTYPE(symbol->st_info) == STT_FUNC && symbol->st_value == value) {
      // the symbol is defined in this file as 0 is undefined.
      // an STT_GNU_IFUNC will not match here as the function value will be the
      // the resulting function of the resolver function and therefore at a
      // different address
      result.name_ = strings + symbol->st_name;
      break;
    }
    ++index;
  }
  if (!result.name_) {
    return result; // not found
  }
  auto rela_plt = reinterpret_cast<Elf_Shdr*>(find_section(".rela.plt"));
  if (!rela_plt) {
    return result;
  }
  size_t rela_plt_entries = rela_plt->sh_size / rela_plt->sh_entsize;
  unsigned char* rela = data_ + rela_plt->sh_offset;
  // attempt to find symbol in .rela.plt
  for (size_t n = 0; n != rela_plt_entries; ++n) {
    auto reloc = reinterpret_cast<Elf_Rela*>(rela);
    if (ELF_R_TYPE(reloc->r_info) == R_X86_64_JUMP_SLOT) {
      // find symbol by index
      size_t symbol_index = ELF_R_SYM(reloc->r_info);
      if (symbol_index == index) {
        result.rela_offset_ = reloc->r_offset;
        break;
      }
    }
    rela += rela_plt->sh_entsize;
  }
  // a symbol will only be present in .rela.plt iff it was used in the ELF
  // object - otherwise only .dynsym and .symtab will contain it
  // this means, no GOT entry for function and in this case result.rela_plt
  // returns 0
  return result;
}

// look for STT_IFUNC symbols by finding the function address in the relocated
// .rela.plt entries. STT_IFUNC are rare, but time(time_t*) is one of them
ELFInfo::Symbol ELFInfo::get_indirect_symbol_rela(unsigned char* base,
                                                  void* function)
{
  Symbol result;
  auto str_tab = reinterpret_cast<Elf_Shdr*>(find_section(".dynstr"));
  if (!str_tab) {
    return result;
  }
  auto strings = reinterpret_cast<const char*>(data_ + str_tab->sh_offset);
  // find dynamic symbol table
  auto symbol_table = reinterpret_cast<Elf_Shdr*>(find_section(".dynsym"));
  if (!symbol_table) {
    return result;
  }
  auto symbols = data_ + symbol_table->sh_offset;
  // find function in relocated GOT
  auto rela_plt = reinterpret_cast<Elf_Shdr*>(find_section(".rela.plt"));
  if (!rela_plt) {
    return result;
  }
  size_t rela_plt_entries = rela_plt->sh_size / rela_plt->sh_entsize;
  unsigned char* rela = data_ + rela_plt->sh_offset;
  for (size_t n = 0; n != rela_plt_entries; ++n) {
    auto reloc = reinterpret_cast<Elf_Rela*>(rela);
    if (ELF_R_TYPE(reloc->r_info) == R_X86_64_IRELATIVE
        && *reinterpret_cast<void**>(base + reloc->r_offset) == function) {
      result.rela_offset_ = reloc->r_offset;
      // find symbol by r_addend in dynamic symbol table
      auto end = symbols + symbol_table->sh_size;
      for ( ; symbols < end; symbols += symbol_table->sh_entsize) {
        auto symbol = reinterpret_cast<Elf_Sym*>(symbols);
        if (ELF_STTYPE(symbol->st_info) == STT_GNU_IFUNC
            && symbol->st_value == reloc->r_addend) {
          result.name_ = strings + symbol->st_name;
          break;
        }
      }
      break;
    }
    rela += rela_plt->sh_entsize;
  }
  return result; // whether found or not
}

size_t ELFInfo::get_symbol_rela_dyn(const char* name)
{
  auto str_tab = reinterpret_cast<Elf_Shdr*>(find_section(".dynstr"));
  if (!str_tab) {
    return 0;
  }
  auto strings = reinterpret_cast<const char*>(data_ + str_tab->sh_offset);
  // find symbol table
  auto symbol_table = reinterpret_cast<Elf_Shdr*>(find_section(".dynsym"));
  if (symbol_table == nullptr) {
    return 0;
  }
  auto symbols = data_ + symbol_table->sh_offset;
  size_t symbol_entries = symbol_table->sh_size / symbol_table->sh_entsize;
  // find symbol in .rela.dyn
  for (const char* section : { ".rela.dyn", ".rela.plt" }) {
    auto rela_dyn = reinterpret_cast<Elf_Shdr*>(find_section(section));
    if (!rela_dyn) {
      return 0;
    }
    size_t rela_entries = rela_dyn->sh_size / rela_dyn->sh_entsize;
    unsigned char* rela = data_ + rela_dyn->sh_offset;
    for (size_t n = 0; n != rela_entries; ++n) {
      auto reloc = reinterpret_cast<Elf_Rel*>(rela);
      size_t symbol_index = ELF_R_SYM(reloc->r_info);
      if (symbol_index < symbol_entries) {
        symbol_index *= symbol_table->sh_entsize;
        auto symbol = reinterpret_cast<Elf_Sym*>(symbols + symbol_index);
        const char* symbol_name = strings + symbol->st_name;
        if (strcmp(name, symbol_name) == 0) {
          return reloc->r_offset;
        }
      }
      rela += rela_dyn->sh_entsize;
    }
  }
  return 0;
}

std::unordered_map<std::string, unsigned char*>
ELFInfo::get_vtables(unsigned char* base)
{
  if (!is_elf()) {
    return { };
  }
  auto elf = reinterpret_cast<Elf_Ehdr*>(data_);
  auto header = data_ + elf->e_shoff;
  // get string table by finding the right strtab section
  // .dynstr is for DYNSYM section (it has a non-zero address)
  // .shstrtab is the "section header" string table, it is indexed in elf header
  // .strtab is for SYMTAB, which is what we want here
  auto str_tab = reinterpret_cast<Elf_Shdr*>(find_section(".strtab"));
  if (!str_tab) {
    return { };
  }
  auto strings = reinterpret_cast<const char*>(data_ + str_tab->sh_offset);
  // find symbol table
  auto symbol_table = reinterpret_cast<Elf_Shdr*>(find_section(".symtab"));
  if (symbol_table == nullptr) {
    return { };
  }
  std::unordered_map<std::string, unsigned char*> vtables;
  // look for virtual tables
  auto symbols = data_ + symbol_table->sh_offset;
  auto end = symbols + symbol_table->sh_size;
  for ( ; symbols < end; symbols += symbol_table->sh_entsize) {
    auto symbol = reinterpret_cast<Elf_Sym*>(symbols);
    // check if is defined (value != 0)
    if (symbol->st_value && ELF_STTYPE(symbol->st_info) == STT_OBJECT) {
      // check if it is a virtual table (starts with "_ZTV")
      auto name = strings + symbol->st_name;
      if (strncmp(name, "_ZTV", 4) == 0) {
        vtables[name + 4] = base + symbol->st_value;
      }
    }
  }
  return vtables;
}

} // namespace spy
