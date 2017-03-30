#ifndef __MEMBRANE_H__
#define __MEMBRANE_H__

#include <stdio.h>
#include <stdarg.h>
#include <erl_nif.h>
#ifdef __GNUC__
#include <stdint.h>
#endif

// varargs parse helpers
#define MEMBRANE_UTIL_PARSE_ARG(position, var_name, var_def, getter_func, ...) \
  var_def; \
  if(!getter_func(env, argv[position], __VA_ARGS__)) { \
    return membrane_util_make_error_args(env, #var_name, #getter_func); \
  }

#define MEMBRANE_UTIL_PARSE_UINT_ARG(position, var_name) \
  MEMBRANE_UTIL_PARSE_ARG(position, var_name, unsigned int var_name, enif_get_uint, &var_name)

#define MEMBRANE_UTIL_PARSE_INT_ARG(position, var_name) \
  MEMBRANE_UTIL_PARSE_ARG(position, var_name, int var_name, enif_get_int, &var_name)

#define MEMBRANE_UTIL_PARSE_ATOM_ARG(position, var_name, max_size) \
  MEMBRANE_UTIL_PARSE_ARG(position, var_name, char var_name[max_size], enif_get_atom, (char *) var_name, max_size, ERL_NIF_LATIN1)

#define MEMBRANE_UTIL_PARSE_STRING_ARG(position, var_name, max_size) \
  MEMBRANE_UTIL_PARSE_ARG(position, var_name, char var_name[max_size], enif_get_string, (char *) var_name, max_size, ERL_NIF_LATIN1)

#define MEMBRANE_UTIL_PARSE_BINARY_ARG(position, var_name) \
  MEMBRANE_UTIL_PARSE_ARG(position, var_name, ErlNifBinary var_name, enif_inspect_binary, &var_name)

#define MEMBRANE_UTIL_PARSE_RESOURCE_ARG(position, var_name, handle_resource_type, pointer_to_var) \
  MEMBRANE_UTIL_PARSE_ARG(position, var_name, var_name, enif_get_resource, handle_resource_type, pointer_to_var)


// format encoding constants
#define MEMBRANE_SAMPLE_FORMAT_TYPE ((uint32_t)(0b11 << 30))
#define MEMBRANE_SAMPLE_FORMAT_TYPE_SIGN ((uint32_t)(0b1 << 30))
#define MEMBRANE_SAMPLE_FORMAT_ENDIANITY ((uint32_t)(0b1 << 29))
#define MEMBRANE_SAMPLE_FORMAT_SIZE ((uint32_t)((0b1 << 8) - 1))
#define MEMBRANE_SAMPLE_FORMAT_TYPE_U ((uint32_t)(0b00 << 30))
#define MEMBRANE_SAMPLE_FORMAT_TYPE_S ((uint32_t)(0b01 << 30))
#define MEMBRANE_SAMPLE_FORMAT_TYPE_F ((uint32_t)(0b11 << 30))
#define MEMBRANE_SAMPLE_FORMAT_ENDIANITY_LE ((uint32_t)(0b0 << 29))
#define MEMBRANE_SAMPLE_FORMAT_ENDIANITY_BE ((uint32_t)(0b1 << 29))



#define MEMBRANE_DEBUG(message, ...) printf("[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__);


/**
 * Builds `{:error, reason}`.
 */
static ERL_NIF_TERM membrane_util_make_error(ErlNifEnv* env, ERL_NIF_TERM reason) {
  ERL_NIF_TERM tuple[2] = {
    enif_make_atom(env, "error"),
    reason
  };

  return enif_make_tuple_from_array(env, tuple, 2);
}


/**
 * Builds `{:ok, arg}`.
 */
static ERL_NIF_TERM membrane_util_make_ok_tuple(ErlNifEnv* env, ERL_NIF_TERM arg) {
  ERL_NIF_TERM tuple[2] = {
    enif_make_atom(env, "ok"),
    arg
  };

  return enif_make_tuple_from_array(env, tuple, 2);
}


/**
 * Builds `{:ok, arg1, arg2}`.
 */
static ERL_NIF_TERM membrane_util_make_ok_tuple2(ErlNifEnv* env, ERL_NIF_TERM arg1, ERL_NIF_TERM arg2) {
  ERL_NIF_TERM tuple[3] = {
    enif_make_atom(env, "ok"),
    arg1,
    arg2
  };

  return enif_make_tuple_from_array(env, tuple, 3);
}


/**
 * Builds `:ok`.
 */
static ERL_NIF_TERM membrane_util_make_ok(ErlNifEnv* env) {
  return enif_make_atom(env, "ok");
}


/**
 * Builds `:todo`.
 */
static ERL_NIF_TERM membrane_util_make_todo(ErlNifEnv* env) {
  return enif_make_atom(env, "todo");
}


/**
 * Builds `{:error, {:args, field, description}}` for returning when
 * certain constructor-style functions get invalid arguments.
 */
static ERL_NIF_TERM membrane_util_make_error_args(ErlNifEnv* env, const char* field, const char *description) {
  ERL_NIF_TERM tuple[3] = {
    enif_make_atom(env, "args"),
    enif_make_atom(env, field),
    enif_make_string(env, description, ERL_NIF_LATIN1)
  };

  return membrane_util_make_error(env, enif_make_tuple_from_array(env, tuple, 3));
}

/**
 * Builds `{:error, {:internal, reason}}` for returning when certain
 * constructor-style functions fails on internal stuff that should generally
 * speaking, not fail.
 */
static ERL_NIF_TERM membrane_util_make_error_internal(ErlNifEnv* env, const char* reason) {
  ERL_NIF_TERM tuple[3] = {
    enif_make_atom(env, "internal"),
    enif_make_atom(env, reason)
  };

  return membrane_util_make_error(env, enif_make_tuple_from_array(env, tuple, 2));
}

#endif
