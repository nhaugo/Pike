/*
 * $Id: invert.c,v 1.2 1996/11/11 14:23:27 grubba Exp $
 *
 * INVERT crypto module for Pike
 *
 * /precompiled/crypto/invert
 *
 * Henrik Grubbström 1996-11-08
 */

/*
 * Includes
 */

/* From the Pike distribution */
#include "global.h"
#include "stralloc.h"
#include "interpret.h"
#include "svalue.h"
#include "constants.h"
#include "macros.h"
#include "threads.h"
#include "object.h"
#include "stralloc.h"
#include "builtin_functions.h"

/* Module specific includes */
#include "precompiled_crypto.h"

/*
 * Globals
 */

struct program *pike_invert_program;

/*
 * Functions
 */

void init_pike_invert(struct object *o)
{
}

void exit_pike_invert(struct object *o)
{
}

/*
 * efuns and the like
 */

/* string name(void) */
static void f_name(INT32 args)
{
  if (args) {
    error("Too many arguments to invert->name()\n");
  }
  push_string(make_shared_string("INVERT"));
}

/* int query_block_size(void) */
static void f_query_block_size(INT32 args)
{
  if (args) {
    error("Too many arguments to invert->query_block_size()\n");
  }
  push_int(8);
}

/* int query_key_length(void) */
static void f_query_key_length(INT32 args)
{
  if (args) {
    error("Too many arguments to invert->query_key_length()\n");
  }
  push_int(0);
}

/* void set_key(string) */
static void f_set_key(INT32 args)
{
  if (args != 1) {
    error("Wrong number of args to invert->set_key()\n");
  }
  if (sp[-1].type != T_STRING) {
    error("Bad argument 1 to invert->set_key()\n");
  }

  pop_n_elems(args);
}

/* string crypt_block(string) */
static void f_crypt_block(INT32 args)
{
  char *buffer;
  int i;
  int len;

  if (args != 1) {
    error("Wrong number of arguments to invert->crypt_block()\n");
  }
  if (sp[-1].type != T_STRING) {
    error("Bad argument 1 to invert->crypt_block()\n");
  }
  if (sp[-1].u.string->len % 8) {
    error("Bad length of argument 1 to invert->crypt_block()\n");
  }

  if (!(buffer = alloca(len = sp[-1].u.string->len))) {
    error("invert->crypt_block(): Out of memory\n");
  }

  for (i=0; i<len; i++) {
    buffer[i] = ~sp[-1].u.string->str[i];
  }

  pop_n_elems(args);

  push_string(make_shared_binary_string(buffer, len));

  MEMSET(buffer, 0, len);
}

/*
 * Module linkage
 */

void init_invert_efuns(void)
{
  /* add_efun()s */
}

void init_invert_programs(void)
{
  /*
   * start_new_program();
   *
   * add_storage();
   *
   * add_function();
   * add_function();
   * ...
   *
   * set_init_callback();
   * set_exit_callback();
   *
   * program = end_c_program();
   * program->refs++;
   *
   */

  /* /precompiled/crypto/invert */
  start_new_program();

  add_function("name", f_name, "function(void:string)", OPT_TRY_OPTIMIZE);
  add_function("query_block_size", f_query_block_size, "function(void:int)", OPT_TRY_OPTIMIZE);
  add_function("query_key_length", f_query_key_length, "function(void:int)", OPT_TRY_OPTIMIZE);
  add_function("set_encrypt_key", f_set_key, "function(string:void)", OPT_SIDE_EFFECT);
  add_function("set_decrypt_key", f_set_key, "function(string:void)", OPT_SIDE_EFFECT);
  add_function("crypt_block", f_crypt_block, "function(string:string)", OPT_SIDE_EFFECT);

  set_init_callback(init_pike_invert);
  set_exit_callback(exit_pike_invert);

  pike_invert_program = end_c_program("/precompiled/crypto/invert");
  pike_invert_program->refs++;
}

void exit_invert(void)
{
  /* free_program()s */
  free_program(pike_invert_program);
}


