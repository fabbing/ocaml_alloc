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

#ifndef CAML_MINOR_GC_H
#define CAML_MINOR_GC_H

#include "address_class.h"
#include "config.h"

/* Global variables moved to Caml_state in 4.10 */
#define caml_young_start (Caml_state_field(young_start))
#define caml_young_end (Caml_state_field(young_end))
#define caml_young_ptr (Caml_state_field(young_ptr))
#define caml_young_limit (Caml_state_field(young_limit))
#define caml_young_alloc_start (Caml_state_field(young_alloc_start))
#define caml_young_alloc_end (Caml_state_field(young_alloc_end))
#define caml_young_alloc_mid (Caml_state_field(young_alloc_mid))
#define caml_young_trigger (Caml_state_field(young_trigger))
#define caml_minor_heap_wsz (Caml_state_field(minor_heap_wsz))
#define caml_in_minor_collection (Caml_state_field(in_minor_collection))
#define caml_extra_heap_resources_minor \
  (Caml_state_field(extra_heap_resources_minor))


#define CAML_TABLE_STRUCT(t) { \
  t *base;                     \
  t *end;                      \
  t *threshold;                \
  t *ptr;                      \
  t *limit;                    \
  asize_t size;                \
  asize_t reserve;             \
}

/* Count of the total number of minor collections performed by the program */
CAMLextern atomic_uintnat caml_minor_collections_count;

struct caml_ref_table CAML_TABLE_STRUCT(value *);

struct caml_ephe_ref_elt {
  value ephe;      /* an ephemeron in major heap */
  mlsize_t offset; /* the offset that points in the minor heap  */
};

struct caml_ephe_ref_table CAML_TABLE_STRUCT(struct caml_ephe_ref_elt);

struct caml_custom_elt {
  value block;     /* The finalized block in the minor heap. */
  mlsize_t mem;    /* The parameters for adjusting GC speed. */
  mlsize_t max;
};

struct caml_custom_table CAML_TABLE_STRUCT(struct caml_custom_elt);
/* Table of custom blocks in the minor heap that contain finalizers
   or GC speed parameters. */

CAMLextern void caml_minor_collection (void);

#ifdef CAML_INTERNALS
extern void caml_set_minor_heap_size (asize_t); /* size in bytes */
extern void caml_empty_minor_heap (void);
extern void caml_gc_dispatch (void);
extern void caml_garbage_collection (void); /* runtime/signals_nat.c */
extern void caml_oldify_one (value, value *);
extern void caml_oldify_mopup (void);

extern void caml_realloc_ref_table (struct caml_ref_table *);
extern void caml_alloc_table (struct caml_ref_table *, asize_t, asize_t);
extern void caml_realloc_ephe_ref_table (struct caml_ephe_ref_table *);
extern void caml_alloc_ephe_table (struct caml_ephe_ref_table *,
                                   asize_t, asize_t);
extern void caml_realloc_custom_table (struct caml_custom_table *);
extern void caml_alloc_custom_table (struct caml_custom_table *,
                                     asize_t, asize_t);
void caml_alloc_minor_tables (void);

/* Asserts that a word is a valid header for a young object */
#define CAMLassert_young_header(hd)                \
  CAMLassert(Wosize_hd(hd) > 0 &&                  \
             Wosize_hd(hd) <= Max_young_wosize &&  \
             Color_hd(hd) == 0)

#define Oldify(p) do{ \
    value __oldify__v__ = *p; \
    if (Is_block (__oldify__v__) && Is_young (__oldify__v__)){ \
      caml_oldify_one (__oldify__v__, (p)); \
    } \
  }while(0)

Caml_inline void add_to_ref_table (struct caml_ref_table *tbl, value *p)
{
  if (tbl->ptr >= tbl->limit){
    CAMLassert (tbl->ptr == tbl->limit);
    caml_realloc_ref_table (tbl);
  }
  *tbl->ptr++ = p;
}

Caml_inline void add_to_ephe_ref_table (struct caml_ephe_ref_table *tbl,
                                          value ar, mlsize_t offset)
{
  struct caml_ephe_ref_elt *ephe_ref;
  if (tbl->ptr >= tbl->limit){
    CAMLassert (tbl->ptr == tbl->limit);
    caml_realloc_ephe_ref_table (tbl);
  }
  ephe_ref = tbl->ptr++;
  ephe_ref->ephe = ar;
  ephe_ref->offset = offset;
  CAMLassert(ephe_ref->offset < Wosize_val(ephe_ref->ephe));
}

Caml_inline void add_to_custom_table (struct caml_custom_table *tbl, value v,
                                        mlsize_t mem, mlsize_t max)
{
  struct caml_custom_elt *elt;
  if (tbl->ptr >= tbl->limit){
    CAMLassert (tbl->ptr == tbl->limit);
    caml_realloc_custom_table (tbl);
  }
  elt = tbl->ptr++;
  elt->block = v;
  elt->mem = mem;
  elt->max = max;
}

#endif /* CAML_INTERNALS */

#endif /* CAML_MINOR_GC_H */
