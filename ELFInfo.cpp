#include "elfspy/ELFInfo.h"

#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <unordered_map>

#include "elfspy/Report.h"
#include "elfspy/Fail.h"
#include "elfspy/MFile.h"
#include "elfspy/SectionHeader.h"

#ifdef __x86_64__
using Elf_Phdr = Elf64_Phdr;
using Elf_Ehdr = Elf64_Ehdr;
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

std::string get_lib_core_name(const char* file_name)
{
  // strip trailing [-major[.minor]].so[.n[.n]]
  const char* root_end = nullptr;
  const char* pos;
  for (pos = file_name; *pos; ++pos) {
    if (*pos == '-') {
      root_end = pos;
    } else if (strncmp(pos, ".so", 3) == 0) {
      break;
    }
  }
  if (!*pos) {
    return { }; // no .so extension - not a shared lib
  }
  if (root_end) {
    for (const char* c = root_end + 1; c != pos; ++c) {
      if (!isdigit(*c) && *c != '.') {
        // this is not major.minor format so don't exclude it
        root_end = pos;
        break;
      }
    }
  } else {
    root_end = pos;
  }
  return { file_name, root_end };
}

using Listing = std::unordered_map<std::string, std::string>;
std::unordered_map<std::string, Listing> debug_files;

// see if a library has a corresponding debug file where the symbols are kept
std::string get_debug_file_name(const char* file_name)
{
  const char* slash = nullptr;
  for (const char* seek = file_name; *seek; ++seek) {
    if (*seek == '/') slash = seek;
  }
  if (!slash) return { };
  // see if directory has already been scanned
  std::string dir_name(file_name, slash);
  auto insert = debug_files.emplace(std::move(dir_name), std::move(Listing{}));
  auto& listing = *insert.first;
  auto& entries = listing.second;
  if (insert.second) {
    const std::string& dir_name = listing.first;
    std::string name;
    name.reserve(1024); // reasonable unscientific value
    name = "/usr/lib/debug";
    name.append(dir_name);
    DIR* dir = opendir(name.c_str());
    if (!dir) return { };
    name.push_back('/');
    struct dirent* entry;
    while ((entry = readdir(dir))) {
      std::string root_name = get_lib_core_name(entry->d_name);
      if (!root_name.empty()) {
        entries.emplace(std::move(root_name), std::move(name + entry->d_name));
      }
    }
    closedir(dir);
  }
  auto seek = entries.find(get_lib_core_name(slash + 1));
  if (seek != entries.end()) {
    return seek->second;
  }
  return { };
}

} // namespace

namespace spy
{

ELFInfo::ELFInfo(const char* name)
{
  name_ = name;
  files_.emplace_back(std::make_unique<MFile>(name));
  std::string debug_file_name = get_debug_file_name(name);
  if (!debug_file_name.empty()) {
    files_.emplace_back(std::make_unique<MFile>(debug_file_name.c_str()));
  }
  if (!is_elf()) {
    Fail() << "Not ELF data - no ELF header found in " << name;
  }
}

ELFInfo::~ELFInfo()
{
}

ELFObject ELFInfo::prepare_object(unsigned char* base) const
{
  ELFObject object;
  object.name_ = name_;
  object.base_ = base;
  object.size_ = files_[0]->size();
  unprotect(base);
  return object;
}

bool ELFInfo::is_elf() const
{
  auto elf = reinterpret_cast<const Elf_Ehdr*>(files_[0]->address());
  return elf->e_ident[EI_MAG0] == ELFMAG0
      && elf->e_ident[EI_MAG1] == ELFMAG1
      && elf->e_ident[EI_MAG2] == ELFMAG2
      && elf->e_ident[EI_MAG3] == ELFMAG3;
}

SectionHeader ELFInfo::find_header(const char* name) const
{
  for (auto& file : files_) {
    auto elf = reinterpret_cast<const Elf_Ehdr*>(file->address());
    auto header = file->address() + elf->e_shoff;
    auto section_names =
        reinterpret_cast<const Elf_Shdr*>(header + elf->e_shstrndx * elf->e_shentsize);
    auto names = reinterpret_cast<const char*>(elf) + section_names->sh_offset;
    // .shstrtab is the "section header" string table
    // it is indexed in elf header
    for (size_t n = 0; n != elf->e_shnum; ++n, header += elf->e_shentsize) {
      auto section = reinterpret_cast<const Elf_Shdr*>(header);
      if (strcmp(names + section->sh_name, name) == 0) {
        return { file->address(), section };
      }
    }
  }
  if (strncmp(name_, "/lib", 4) != 0 && strncmp(name_, "/usr", 4) != 0) {
    Report() << "no " << name << " section found in " << name_;
  }
  return { };
}

void ELFInfo::unprotect(unsigned char* base, const char* name) const
{
  SectionHeader section = find_header(name);
  if (section) {
    Address begin;
    Address end;
    begin.pointer_ = base + section.header_->sh_addr;
    end.pointer_ = begin.pointer_ + section.header_->sh_size;
    begin.round_down();
    end.round_up();
    size_t size = end.value_ - begin.value_;
    if (mprotect(begin.pointer_, size,
                 PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
      spy::Fail() << "Cannot set " << name << " to writable for " << name_;
    }
  }
}

void ELFInfo::unprotect(unsigned char* base) const
{
  // by default this memory is read only - ELFspy needs to change it
  unprotect(base, ".got");
  unprotect(base, ".got.plt");
}

const char* ELFInfo::find_name(size_t name_offset) const
{
  SectionHeader str_tab = find_header(".dynstr");
  if (!str_tab) {
    return nullptr;
  }
  return reinterpret_cast<const char*>(str_tab.begin()) + name_offset;
}

ELFInfo::Symbol ELFInfo::get_symbol_rela(size_t value) const
{
  Symbol result;
  // find symbol by value in dynamic symbol table
  size_t index = 0;
  for (auto& symbol : find_header(".dynsym").as_section<Elf_Sym>()) {
    if (ELF_STTYPE(symbol.st_info) == STT_FUNC && symbol.st_value == value) {
      // the symbol is defined in this file as 0 is undefined.
      // an STT_GNU_IFUNC will not match here as the function value will be the
      // the resulting function of the resolver function and therefore at a
      // different address
      result.name_ = find_name(symbol.st_name);
      break;
    }
    ++index;
  }
  if (!result.name_) {
    return result; // not found
  }
  // attempt to find symbol in .rela.plt using index from symbol table
  for (auto& reloc : find_header(".rela.plt").as_section<Elf_Rela>()) {
    if (ELF_R_TYPE(reloc.r_info) == R_X86_64_JUMP_SLOT) {
      // find symbol by index
      size_t symbol_index = ELF_R_SYM(reloc.r_info);
      if (symbol_index == index) {
        result.rela_offset_ = reloc.r_offset;
        break;
      }
    }
  }
  // a symbol will only be present in .rela.plt iff it was used in the ELF
  // object - otherwise only .dynsym and .symtab will contain it
  // this means, no GOT entry for function and in this case result.rela_plt
  // returns 0
  return result;
}

// look for STT_IFUNC symbols by finding the function address in the relocated
// .rela.plt entries. STT_IFUNC are rare, but time(time_t*) is one of them
ELFInfo::Symbol ELFInfo::get_indirect_symbol_rela(const unsigned char* base,
                                                  void* function)
{
  Symbol result;
  // find function in relocated GOT
  for (auto& reloc : find_header(".rela.plt").as_section<Elf_Rela>()) {
    if (ELF_R_TYPE(reloc.r_info) == R_X86_64_IRELATIVE &&
        *reinterpret_cast<void* const*>(base + reloc.r_offset) == function) {
      result.rela_offset_ = reloc.r_offset;
      // find symbol by r_addend in dynamic symbol table
      for (auto& symbol : find_header(".dynsym").as_section<Elf_Sym>()) {
        if (ELF_STTYPE(symbol.st_info) == STT_GNU_IFUNC
            && symbol.st_value == reloc.r_addend) {
          result.name_ = find_name(symbol.st_name);
          break;
        }
      }
      break;
    }
  }
  return result; // whether found or not
}

size_t ELFInfo::get_symbol_rela_dyn(const char* name)
{
  // find symbol table
  auto symbol_table = find_header(".dynsym").as_section<Elf_Sym>();
  // find symbol in .rela.dyn
  for (const char* section : { ".rela.dyn", ".rela.plt" }) {
    for (auto& reloc : find_header(section).as_section<Elf_Rel>()) {
      size_t symbol_index = ELF_R_SYM(reloc.r_info);
      if (symbol_index < symbol_table.entries()) {
        auto& symbol = symbol_table[symbol_index];
        const char* symbol_name = find_name(symbol.st_name);
        if (symbol_name && strcmp(name, symbol_name) == 0) {
          return reloc.r_offset;
        }
      }
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
  // get string table by finding the right strtab section
  // .dynstr is for DYNSYM section (it has a non-zero address)
  // .shstrtab is the "section header" string table, it is indexed in elf header
  // .strtab is for SYMTAB, which is what we want here
  SectionHeader str_tab = find_header(".strtab");
  if (!str_tab) {
    return { };
  }
  auto strings = reinterpret_cast<const char*>(str_tab.begin());
  std::unordered_map<std::string, unsigned char*> vtables;
  // look for virtual tables in symbol table
  for (auto& symbol : find_header(".symtab").as_section<Elf_Sym>()) {
    // check if is defined (value != 0)
    if (symbol.st_value && ELF_STTYPE(symbol.st_info) == STT_OBJECT) {
      // check if it is a virtual table (starts with "_ZTV")
      auto name = strings + symbol.st_name;
      if (strncmp(name, "_ZTV", 4) == 0) {
        vtables[name + 4] = base + symbol.st_value;
      }
    }
  }
  return vtables;
}

} // namespace spy
