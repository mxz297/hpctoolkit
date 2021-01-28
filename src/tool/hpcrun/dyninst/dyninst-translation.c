// -*-Mode: C++;-*- // technically C99

// * BeginRiceCopyright *****************************************************
//
// $HeadURL$
// $Id$
//
// --------------------------------------------------------------------------
// Part of HPCToolkit (hpctoolkit.org)
//
// Information about sources of support for research and development of
// HPCToolkit is at 'hpctoolkit.org' and in 'README.Acknowledgments'.
// --------------------------------------------------------------------------
//
// Copyright ((c)) 2002-2021, Rice University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of Rice University (RICE) nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// This software is provided by RICE and contributors "as is" and any
// express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular
// purpose are disclaimed. In no event shall RICE or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or
// business interruption) however caused and on any theory of liability,
// whether in contract, strict liability, or tort (including negligence
// or otherwise) arising in any way out of the use of this software, even
// if advised of the possibility of such damage.
//
// ******************************************************* EndRiceCopyright *

//******************************************************************************
// global include files
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//---------------------------------------------------------------------
// local include files
//---------------------------------------------------------------------

#include "dyninst/dyninst-translation.h"
#include "loadmap.h"

//---------------------------------------------------------------------
// macros
//---------------------------------------------------------------------

#define DT_DYNINST_RELOCMAP 0x6D191959
#define IP_OFFSET_IN_CURSOR 3

//******************************************************************************
// type
//******************************************************************************

typedef struct inst_mapping_table_entry {
  int64_t reloc_addr;
  int64_t orig_addr;
  int64_t size;
} inst_mapping_table_entry_t;

typedef struct inst_mapping_table {
  int64_t total;
  inst_mapping_table_entry_t entries[];
} inst_mapping_table_t;

static inst_mapping_table_t* inst_mapping = NULL;

//---------------------------------------------------------------------
// private operations
//---------------------------------------------------------------------

static void
dyninst_translation_notify_map(load_module_t* lm)
{
  if (lm == NULL || lm->dso_info == NULL) return;
  if (inst_mapping == NULL) {
    // Get DYNAMIC section
    struct dl_phdr_info* map = &(lm->phdr_info);
    ElfW(Dyn)* dynamic_loc = NULL;
    for(size_t i = 0; i < map->dlpi_phnum; i++) {
      if(map->dlpi_phdr[i].p_type == PT_DYNAMIC) {
        dynamic_loc = (void*)map->dlpi_addr + map->dlpi_phdr[i].p_vaddr;
        break;
      }
    }
    for(const ElfW(Dyn)* d = dynamic_loc; d->d_tag != DT_NULL; d++) {
      if(d->d_tag != DT_DYNINST_RELOCMAP) continue;
      inst_mapping = (inst_mapping_table_t*)d->d_un.d_ptr;
      break;
    }
    /*
    if (inst_mapping != NULL) {
      fprintf(stderr, "inst mapping %p\n", inst_mapping);
      fprintf(stderr, "inst mapping entry number: %u\n", inst_mapping->total);
      for (int64_t i = 0; i < inst_mapping->total; ++i) {
        inst_mapping_table_entry_t* e = &(inst_mapping->entries[i]);
        fprintf(stderr, "entry %d: %lx %lx %lx\n", i, e->reloc_addr, e->orig_addr, e->size);
      }
    }
    */
  }
}

static void
dyninst_translation_notify_unmap(load_module_t* lm)
{
  return;
}

//---------------------------------------------------------------------
// interface operations
//---------------------------------------------------------------------

dyninst_translation_result_type_t
dyninst_translation_lookup
(
  void* input_addr,
  void** output_addr_ptr
)
{
  if (inst_mapping == NULL) {
    *output_addr_ptr = input_addr;
    return DYNINST_TRANSLATION_ORIGINAL;
  }
  uint64_t addr = (uint64_t)input_addr;
  //fprintf(stderr, "enter uw_translation_lookup: addr %p\n", input_addr);
  if (addr < inst_mapping->entries[0].reloc_addr) {
    *output_addr_ptr = input_addr;
    return DYNINST_TRANSLATION_ORIGINAL;
  }

  uint64_t upper_bound = inst_mapping->entries[inst_mapping->total - 1].reloc_addr + inst_mapping->entries[inst_mapping->total - 1].size;
  if (addr >= upper_bound) {
    *output_addr_ptr = input_addr;
    return DYNINST_TRANSLATION_ORIGINAL;
  }

  unsigned long left = 0, right = inst_mapping->total - 1, mid;
  if (addr >= inst_mapping->entries[right].reloc_addr) {    
    if (inst_mapping->entries[right].orig_addr == -1) {
      dyninst_translation_lookup((void*)(inst_mapping->entries[right].reloc_addr + inst_mapping->entries[right].size), output_addr_ptr);      
      return DYNINST_TRANSLATION_INSTRUMENTATION;
    } else {
      *output_addr_ptr = (void*)(addr - inst_mapping->entries[right].reloc_addr + inst_mapping->entries[right].orig_addr);
      return DYNINST_TRANSLATION_RELOCATION;
    }
  }
  while (left <= right) {
    mid = (left + right) / 2;
    if (addr >= inst_mapping->entries[mid].reloc_addr && addr < inst_mapping->entries[mid + 1].reloc_addr) {
      break;
    } else if (addr < inst_mapping->entries[mid].reloc_addr) {
      right = mid - 1;
    } else {
      left = mid + 1;
    }
  }
  
  if (inst_mapping->entries[mid].orig_addr == -1) {
    dyninst_translation_lookup((void*)(inst_mapping->entries[mid].reloc_addr + inst_mapping->entries[mid].size), output_addr_ptr);    
    return DYNINST_TRANSLATION_INSTRUMENTATION;
  } else {
    *output_addr_ptr = (void*)(addr - inst_mapping->entries[mid].reloc_addr + inst_mapping->entries[mid].orig_addr);
    return DYNINST_TRANSLATION_RELOCATION;
  }
}

void
dyninst_translation_init
(

)
{
   static loadmap_notify_t dyninst_translation_notifiers;

   dyninst_translation_notifiers.map = dyninst_translation_notify_map;
   dyninst_translation_notifiers.unmap = dyninst_translation_notify_unmap;
   hpcrun_loadmap_notify_register(&dyninst_translation_notifiers);
}

dyninst_translation_result_type_t
dyninst_translation_update_libunwind_cursor
(
  unw_cursor_t* uc
)
{
  unw_word_t* typed_cursor = (unw_word_t*) uc;
  void *ip, *new_ip;
  ip = (void*)(typed_cursor[IP_OFFSET_IN_CURSOR]);
  dyninst_translation_result_type_t ret = dyninst_translation_lookup(ip, &new_ip);
  if (ret != DYNINST_TRANSLATION_ORIGINAL) {
    typed_cursor[IP_OFFSET_IN_CURSOR] = (unw_word_t)new_ip;
  }
  return ret;
}