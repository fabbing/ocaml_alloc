/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*              Damien Doligez, projet Para, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 1996 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

/* Free lists of heap blocks. */

#ifndef CAML_FREELIST_H
#define CAML_FREELIST_H

#ifdef CAML_INTERNALS

#include "misc.h"
#include "mlvalues.h"

extern asize_t caml_fl_cur_wsz;

/* See [freelist.c] for usage info on these functions. */
extern header_t *(*caml_fl_p_allocate) (mlsize_t wo_sz);
extern void (*caml_fl_p_init_merge) (void);
extern header_t *(*caml_fl_p_merge_block) (value bp, char *limit);
extern void (*caml_fl_p_add_blocks) (value bp);
extern void (*caml_fl_p_make_free_blocks)
  (value *p, mlsize_t size, int do_merge, int color);
#ifdef DEBUG
extern void (*caml_fl_p_check) (void);
#endif

Caml_inline header_t *caml_fl_allocate (mlsize_t wo_sz)
  { return (*caml_fl_p_allocate) (wo_sz); }

Caml_inline void caml_fl_init_merge (void)
  { (*caml_fl_p_init_merge) (); }

Caml_inline header_t *caml_fl_merge_block (value bp, char *limit)
  { return (*caml_fl_p_merge_block) (bp, limit); }

Caml_inline void caml_fl_add_blocks (value bp)
  { (*caml_fl_p_add_blocks) (bp); }

Caml_inline void caml_make_free_blocks
  (value *p, mlsize_t size, int do_merge, int color)
  { (*caml_fl_p_make_free_blocks) (p, size, do_merge, color); }

enum {
  caml_policy_next_fit = 0,
  caml_policy_first_fit = 1,
  caml_policy_best_fit = 2,
  caml_policy_alloc_five = 3,
};
extern void caml_set_allocation_policy (uintnat);

extern void caml_fl_reset_and_switch_policy (intnat);
/* -1 means do not change the allocation policy */

#ifdef DEBUG
Caml_inline void caml_fl_check (void)
  { (*caml_fl_p_check) (); }
#endif

#endif /* CAML_INTERNALS */

#endif /* CAML_FREELIST_H */
