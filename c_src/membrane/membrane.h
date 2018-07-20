#pragma once

#include <stdarg.h>
#include <erl_nif.h>
#include <string.h>
#include <errno.h>
#ifdef __GNUC__
#include <stdint.h>
#endif

#define UNUSED(x) (void)(x)

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

#define MEMBRANE_UTIL_PARSE_RESOURCE_ARG(position, var_name, var_type, res_type) \
  var_type * var_name; \
  if(!enif_get_resource(env, argv[position], res_type, (void **) & var_name)) { \
    return membrane_util_make_error_args(env, #var_name, "enif_get_resource"); \
  }

#define MEMBRANE_UTIL_PARSE_PID_ARG(position, var_name) \
  MEMBRANE_UTIL_PARSE_ARG(position, var_name, ErlNifPid var_name, enif_get_local_pid, &var_name)


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


ERL_NIF_TERM membrane_util_make_error(ErlNifEnv* env, ERL_NIF_TERM reason);
ERL_NIF_TERM membrane_util_make_ok_tuple(ErlNifEnv* env, ERL_NIF_TERM arg);
ERL_NIF_TERM membrane_util_make_ok_tuple2(ErlNifEnv* env, ERL_NIF_TERM arg1, ERL_NIF_TERM arg2);
ERL_NIF_TERM membrane_util_make_ok(ErlNifEnv* env);
ERL_NIF_TERM membrane_util_make_todo(ErlNifEnv* env);
ERL_NIF_TERM membrane_util_make_error_args(ErlNifEnv* env, const char* field, const char *description);
ERL_NIF_TERM membrane_util_make_error_errno(ErlNifEnv* env, const char* call);
ERL_NIF_TERM membrane_util_make_error_internal(ErlNifEnv* env, const char* reason);
