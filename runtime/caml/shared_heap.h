/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*      KC Sivaramakrishnan, Indian Institute of Technology, Madras       */
/*                 Stephen Dolan, University of Cambridge                 */
/*                                                                        */
/*   Copyright 2015 Indian Institute of Technology, Madras                */
/*   Copyright 2015 University of Cambridge                               */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/
#ifndef CAML_SHARED_HEAP_H
#define CAML_SHARED_HEAP_H

#ifdef CAML_INTERNALS

#include "config.h"
#include "roots.h"
#include "domain.h"
#include "misc.h"
#include "gc_stats.h"
#include <limits.h>

#define HEADER_BITS (sizeof(header_t) * CHAR_BIT)

#define HEADER_TAG_BITS 8
#define HEADER_TAG_MASK ((1ull << HEADER_TAG_BITS) - 1ull)

#define HEADER_COLOR_BITS 2
#define HEADER_COLOR_SHIFT HEADER_TAG_BITS
#define HEADER_COLOR_MASK (((1ull << HEADER_COLOR_BITS) - 1ull) \
                            << HEADER_COLOR_SHIFT)

#define HEADER_WOSIZE_BITS (HEADER_BITS - HEADER_TAG_BITS \
                            - HEADER_COLOR_BITS - HEADER_RESERVED_BITS)
#define HEADER_WOSIZE_SHIFT (HEADER_COLOR_SHIFT  + HEADER_COLOR_BITS)
#define HEADER_WOSIZE_MASK (((1ull << HEADER_WOSIZE_BITS) - 1ull) \
                             << HEADER_WOSIZE_SHIFT)

//#define Color_hd(hd) ((hd) & HEADER_COLOR_MASK)
#define Hd_with_color(hd, color) (((hd) &~ HEADER_COLOR_MASK) | (color))

#define Reserved_hd(hd)   ((reserved_t)0)
#define Hd_reserved(res)  ((header_t)0)

#define Make_header_with_reserved(wosize, tag, color, reserved)      \
      (/*CAMLassert ((wosize) <= Max_wosize),*/                      \
       ((header_t) (Hd_reserved(reserved))                           \
                    + ((header_t) (wosize) << HEADER_WOSIZE_SHIFT)   \
                    + (color) /* colors are pre-shifted */           \
                    + (tag_t) (tag)))


//#define Make_header(wosize, tag, color) Make_header_with_reserved(wosize, tag, color, 0)


struct caml_heap_state;
struct pool;

struct caml_heap_state* caml_init_shared_heap(void);
void caml_teardown_shared_heap(struct caml_heap_state* heap);

// FIXME CAMLexport
CAMLexport value* caml_shared_try_alloc(struct caml_heap_state*,
                             mlsize_t, tag_t, reserved_t, int);

/* Copy the domain-local heap stats into a heap stats sample. */
void caml_collect_heap_stats_sample(
  struct caml_heap_state* local,
  struct heap_stats *sample);

/* Add the global orphaned heap stats into an accumulator. */
void caml_accum_orphan_heap_stats(struct heap_stats *acc);

uintnat caml_heap_size(struct caml_heap_state*);
uintnat caml_top_heap_words(struct caml_heap_state*);
uintnat caml_heap_blocks(struct caml_heap_state*);

struct pool* caml_pool_of_shared_block(value v);

void caml_shared_unpin(value v);

/* always readable by all threads
   written only by a single thread during STW periods */
typedef uintnat status;
struct global_heap_state {
  status MARKED, UNMARKED, GARBAGE;
};
//extern struct global_heap_state caml_global_heap_state;

/* CR mshinwell: ensure this matches [Emitaux] */
enum {NOT_MARKABLE = 3 << HEADER_COLOR_SHIFT};

Caml_inline int Has_status_hd(header_t hd, status s) {
  return Color_hd(hd) == s;
}

Caml_inline int Has_status_val(value v, status s) {
  return Has_status_hd(Hd_val(v), s);
}

Caml_inline header_t With_status_hd(header_t hd, status s) {
  return Hd_with_color(hd, s);
}

//Caml_inline int is_garbage(value v) {
//  return Has_status_val(v, caml_global_heap_state.GARBAGE);
//}
//
//Caml_inline int is_unmarked(value v) {
//  return Has_status_val(v, caml_global_heap_state.UNMARKED);
//}
//
//Caml_inline int is_marked(value v) {
//  return Has_status_val(v, caml_global_heap_state.MARKED);
//}

void caml_redarken_pool(struct pool*, scanning_action, void*);

intnat caml_sweep(struct caml_heap_state*, intnat);


/* must be called during STW */
void caml_cycle_heap_stw(void);

/* must be called on each domain
   (after caml_cycle_heap_stw) */
void caml_cycle_heap(struct caml_heap_state*);

/* Heap invariant verification (for debugging) */

/* caml_verify_heap must only be called while all domains are paused */
void caml_verify_heap(caml_domain_state *domain);

#ifdef DEBUG
/* [is_garbage(v)] returns true if [v] is a garbage value */
int is_garbage (value);
#endif

#endif /* CAML_INTERNALS */

#endif /* CAML_SHARED_HEAP_H */
