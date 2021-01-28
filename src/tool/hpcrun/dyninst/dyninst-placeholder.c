// -*-Mode: C++;-*- // technically C99

// * BeginRiceCopyright *****************************************************
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
// system includes
//******************************************************************************

#include <assert.h>
#include <pthread.h>
#include <string.h>



//******************************************************************************
// local includes
//******************************************************************************

#include <hpcrun/cct/cct.h>
#include "dyninst-placeholder.h"



//******************************************************************************
// macros
//******************************************************************************

#define SET_LOW_N_BITS(n, type) (~(((type) ~0) << n))



//******************************************************************************
// type declarations
//******************************************************************************

typedef struct dyninst_op_placeholders_t {
  placeholder_t ph[dyninst_placeholder_type_count];
} dyninst_op_placeholders_t;



//******************************************************************************
// public data
//******************************************************************************

dyninst_op_placeholder_flags_t dyninst_op_placeholder_flags_none = 0; 

dyninst_op_placeholder_flags_t dyninst_op_placeholder_flags_all =
  SET_LOW_N_BITS(dyninst_placeholder_type_count, dyninst_op_placeholder_flags_t);



//******************************************************************************
// local data
//******************************************************************************

static dyninst_op_placeholders_t dyninst_op_placeholders;


static pthread_once_t is_initialized = PTHREAD_ONCE_INIT;



//******************************************************************************
// placeholder functions
// 
// note: 
//   placeholder functions are not declared static so that the compiler 
//   doesn't eliminate their names from the symbol table. we need their
//   names in the symbol table to convert them into the appropriate placeholder 
//   strings in hpcprof
//******************************************************************************

void 
dyninst_instrumentation_op
(
  void
)
{
  // this function is not meant to be called
  assert(0);
}

//******************************************************************************
// private operations
//******************************************************************************

static void 
dyninst_op_placeholder_init
(
 dyninst_placeholder_type_t type,
 void *pc
 )
{
  init_placeholder(&dyninst_op_placeholders.ph[type], pc);
}


static void
dyninst_op_placeholders_init
(
 void
)
{
  dyninst_op_placeholder_init(dyninst_placeholder_instrumentation,    &dyninst_instrumentation_op);
}


// debugging support
bool
dyninst_op_ccts_empty
(
 dyninst_op_ccts_t *dyninst_op_ccts
)
{
  int i;
  for (i = 0; i < dyninst_placeholder_type_count; i++) {
    if (dyninst_op_ccts->ccts[i] != NULL) return false;
  }
  return true;
}



//******************************************************************************
// interface operations
//******************************************************************************

ip_normalized_t
dyninst_op_placeholder_ip
(
 dyninst_placeholder_type_t type
)
{
  pthread_once(&is_initialized, dyninst_op_placeholders_init);

  return dyninst_op_placeholders.ph[type].pc_norm;
}


cct_node_t *
dyninst_op_ccts_get
(
 dyninst_op_ccts_t *dyninst_op_ccts,
 dyninst_placeholder_type_t type
)
{
  return dyninst_op_ccts->ccts[type];
}


void
dyninst_op_ccts_insert
(
 cct_node_t *api_node,
 dyninst_op_ccts_t *dyninst_op_ccts,
 dyninst_op_placeholder_flags_t flags
)
{
  int i;
  for (i = 0; i < dyninst_placeholder_type_count; i++) {
    cct_node_t *node = NULL;

    if (flags & (1 << i)) {
      node = hpcrun_cct_insert_ip_norm(api_node, dyninst_op_placeholder_ip(i));
    }

    dyninst_op_ccts->ccts[i] = node;
  }
}


void
dyninst_op_placeholder_flags_set
(
 dyninst_op_placeholder_flags_t *flags,
 dyninst_placeholder_type_t type
)
{
  *flags |= (1 << type);
}


bool
dyninst_op_placeholder_flags_is_set
(
 dyninst_op_placeholder_flags_t flags,
 dyninst_placeholder_type_t type
)
{
  return (flags & (1 << type)) ? true : false;
}
