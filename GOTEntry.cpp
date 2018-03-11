#include "GOTEntry.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <link.h>
#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <unordered_map>

#include "elfspy/MFile.h"
#include "elfspy/ELFInfo.h"
#include "elfspy/Fail.h"
#include "elfspy/Report.h"

#ifdef __x86_64__
#define Elf_Phdr Elf64_Phdr
#define Elf_Ehdr Elf64_Ehdr
#else
#define Elf_Phdr Elf32_Phdr 
#define Elf_Ehdr Elf32_Ehdr
#endif

namespace
{
int argc;
char** argv;
std::string program_name;
bool debug = getenv("ELFSPY_DEBUG") != nullptr;
// TODO: just copy the char it's in memory
std::unordered_map<std::string, unsigned char*> vtables;
std::vector<spy::ELFObject> elf_objects;

void resolve_program_name()
{
  struct stat file_info;
  std::string file_name = argv[0];
  const char* path = getenv("PATH");
  if (!path || argv[0][0] == '/') {
    path = "";
  }
  char* path_copy = strdup(path);
  char* path_token = path_copy;
  char* save;
  for ( ; ; ) {
    int rv = stat(file_name.c_str(), &file_info);
    if (rv == 0) {
      program_name = file_name;
      break;
    }
    const char* dir = strtok_r(path_token, ":", &save);
    if (dir == nullptr) {
      break;
    }
    path_token = nullptr; // for second call to strtok_r
    file_name = dir;
    file_name.push_back('/');
    file_name.append(argv[0]);
  }
  free(path_copy);
}

int read_shared_object(struct dl_phdr_info* info, size_t size, void* data)
{
  // the ELF header for each loaded object
  unsigned char* elf_root;
  const char* name;
  if (!info->dlpi_addr) {
    // this is the executable itself - sadly the name was not included in info
    elf_root = nullptr;
    name = program_name.c_str();
  } else {
    elf_root = reinterpret_cast<unsigned char*>(info->dlpi_addr);
    name = info->dlpi_name;
  }
  if (name && *name && strncmp(name, "linux-vdso.so", 13) != 0) {
    spy::MFile file(name);
    spy::ELFInfo elf(file.address(), name);
    elf.base_ = elf_root;
    elf.size_ = file.size();
    elf.unprotect(elf_root);
    elf_objects.push_back(elf);
    auto entries = elf.get_vtables(elf_root);
    vtables.insert(entries.begin(), entries.end());
  }
  return 0;
}

} // namespace

namespace spy
{
// arguments left in for future compatibility
void GOTEntry::initialise(int argc, char** argv)
{
  ::argc = argc;
  ::argv = argv;
  resolve_program_name();
  if (program_name.empty()) {
    Fail() << "Cannot determine absolute file name of executable";
  }
  dl_iterate_phdr(&read_shared_object, nullptr);
}

// base + rela.plt if symbol is found and defined
// base + rela.dyn if symbol is found and undefined
void* GOTEntry::set(void* function, const char* name)
{
  if (entries_.empty()) {
    auto address = reinterpret_cast<unsigned char*>(function);
    // first find ELF object where it is defined
    ELFInfo::Symbol symbol;
    std::string symbol_name;
    ELFObject* defined = nullptr;
    for (auto& object : elf_objects) {
      // check if it is even possible for the function to exist in the object
      size_t offset = address - object.base_;
      if (offset < object.size_) {
        // that the offset is inside the size is a tell-tale the symbol can be
        // found
        MFile elf_file(object.name_);
        ELFInfo elf(elf_file.address(), object.name_);
        // find name in elf file where it is defined
        symbol = elf.get_symbol_rela(offset);
        if (symbol.rela_offset_) {
          // symbol was defined and used, offset is from the .rela.plt section
          make_entry(object.base_ + symbol.rela_offset_);
        }
        if (symbol.name_) {
          // symbol was defined
          symbol_name = symbol.name_;
          defined = &object;
          break;
        }
      }
    }
    if (!defined) {
      // rare case of looking for an STT_IFUNC when not found at all
      for (auto& object : elf_objects) {
        MFile elf_file(object.name_);
        ELFInfo elf(elf_file.address(), object.name_);
        symbol = elf.get_indirect_symbol_rela(object.base_, function);
        if (symbol.rela_offset_) {
          // symbol was defined, offset is from the .rela.plt section
          make_entry(object.base_ + symbol.rela_offset_);
          symbol_name = symbol.name_;
          defined = &object;
          break;
        }
      }
    }
    if (!defined) {
      Report() << "cannot find definition of function " << name;
      return nullptr;
    }
    // find entries where the symbol is used in other ELF functions
    for (auto& object : elf_objects) {
      if (&object == defined) {
        continue;
      }
      MFile elf_file(object.name_);
      ELFInfo elf(elf_file.address(), object.name_);
      size_t rela_offset = elf.get_symbol_rela_dyn(symbol_name.c_str());
      if (rela_offset) {
        // symbol was undefined, offset is from the .rela.dyn section
        make_entry(object.base_ + rela_offset);
      }
    }
  }
  // in file where it is defined find the .rela.plt
  // if undefined find the name in the .rela.dyn using .symtab index for name
  // and with strcmp
  if (original_ == nullptr) {
    original_ = function;
  }
  current_ = function;
  return original_;
}

void GOTEntry::make_entry(void** address)
{
  // store the previous value of the entry which points to @plt+6 as it has been
  // resolved, @plt there is a indirect jump to the contents of the entry, so
  // if it is set to @plt instead @plt+6 an infinite loop will occur
  Entry entry;
  entry.address_ = address;
  entry.restore_ = *entry.address_;
  entries_.push_back(entry);
}

void** GOTEntry::get_vtable_entry(const std::type_info& type,
                                  const MethodPointer& method)
{
  if (method.is_virtual()) {
    // this is a virtual function
    auto seek = vtables.find(type.name());
    if (seek != vtables.end()) {
      unsigned char* vtable = seek->second;
      // there seems to be two entries that are not accounted for in the index
      auto location = vtable + sizeof(void*) * 2 + (method.index_ & ~1UL);
      return reinterpret_cast<void**>(location);
    }
    Report() << "Could not find virtual function";
    return nullptr;
  }
  // non virtual function
  return nullptr;
}

void* GOTEntry::spy_with(void* function)
{
  if (!spy_count_) {
    patch(function);
  }
  ++spy_count_;
  return original_;
}

void* GOTEntry::patch(void* function)
{
  for (auto entry : entries_) {
    *entry.address_ = function;
  }
  void* previous = current_;
  current_ = function;
  return previous;
}

void GOTEntry::restore()
{
  --spy_count_;
  if (!spy_count_) {
    for (auto entry : entries_) {
      *entry.address_ = entry.restore_;
    }
  }
  current_ = original_;
}

} // namespace spy
