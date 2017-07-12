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

#define MEMBRANE_UTIL_PARSE_RESOURCE_ARG(position, var_name, var_type, res_type) \
  var_type * var_name; \
  if(!enif_get_resource(env, argv[position], res_type, (void **) & var_name)) { \
    return membrane_util_make_error_args(env, #var_name, "enif_get_resource"); \
  }


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



#define MEMBRANE_LOG_LEVEL_DEBUG 0
#define MEMBRANE_LOG_LEVEL_INFO 1
#define MEMBRANE_LOG_LEVEL_WARN 2

#define MEMBRANE_DEBUG(env, message, ...) membrane_log_format(env, MEMBRANE_LOG_LEVEL_DEBUG, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )
#define MEMBRANE_INFO(env, message, ...) membrane_log_format(env, MEMBRANE_LOG_LEVEL_INFO, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )
#define MEMBRANE_WARN(env, message, ...) membrane_log_format(env, MEMBRANE_LOG_LEVEL_WARN, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )
#define MEMBRANE_THREADED_DEBUG(message, ...) membrane_log_format(NULL, MEMBRANE_LOG_LEVEL_DEBUG, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )
#define MEMBRANE_THREADED_INFO(message, ...) membrane_log_format(NULL, MEMBRANE_LOG_LEVEL_INFO, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )
#define MEMBRANE_THREADED_WARN(message, ...) membrane_log_format(NULL, MEMBRANE_LOG_LEVEL_WARN, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )


/**
 * Sends `term` as a erlang message to the log router.
 * Expects exactly one of `env` or `msg_env` arguments to be NULL.
 *
 * On success, returns true. If log could not be sent, false is returned.
 */
static int membrane_route_log(ErlNifEnv *env, ErlNifEnv *msg_env, ERL_NIF_TERM term) {
  ErlNifPid router_pid;
  ERL_NIF_TERM router_module;

  if(env) {
    router_module = enif_make_atom(env, "Elixir.Membrane.Log.Router");
  } else {
    router_module = enif_make_atom(msg_env, "Elixir.Membrane.Log.Router");
  }

  if(enif_whereis_pid(env, router_module, &router_pid)) {
    // router_pid found
    if (enif_send(env, &router_pid, msg_env, term)) {
      return 1;
    }
  }

  return 0;
}


/**
 * Converts integer containing log level to atom.
 */
static ERL_NIF_TERM membrane_log_level_to_term(ErlNifEnv *env, int level) {
  switch(level) {
    case 0:
      return enif_make_atom(env, "debug");
    case 1:
      return enif_make_atom(env, "info");
    case 2:
      return enif_make_atom(env, "warn");
  }
}


/**
 * Wraps erlang terms to the format understandable by loggers, i.e.:
 * `{:membrane_log, :info, ["message"], timestamp_nsec, [:nif]}`
 * It also supplies message with current monotonic time.
 */
static ERL_NIF_TERM membrane_wrap_log_msg(ErlNifEnv *env, int level, ERL_NIF_TERM message_term, ERL_NIF_TERM tags_term) {
  ErlNifTime monotonic_time;
  ERL_NIF_TERM time_term;

  monotonic_time = enif_monotonic_time(ERL_NIF_NSEC);
  time_term = enif_make_long(env, monotonic_time);


  ERL_NIF_TERM output_term = enif_make_tuple5(
    env,
    enif_make_atom(env, "membrane_log"),
    membrane_log_level_to_term(env, level),
    message_term,
    time_term,
    tags_term
  );

  return output_term;
}


/**
 * Builds log message and sends it to the log router.
 *
 * `env` - the environment of the calling process
 * `msg_env` - the environment of the message and tag term
 * `level` - integer
 * `message` - term containing message
 * `tags` - term containing list of terms
 *
 * Expects exactly one of `env` or `msg_env` variables to be a NULL.
 * `message` and `tags` terms should be previously created with the second environment,
 * which is not NULL.
 *
 * On success, returns true. Otherwise, false is returned.
 */
static int membrane_log(ErlNifEnv *env, ErlNifEnv *msg_env, int level, ERL_NIF_TERM message, ERL_NIF_TERM tags) {
  ErlNifEnv *used_env;
  used_env = (env ? env : msg_env);

  ERL_NIF_TERM wrapped_msg = membrane_wrap_log_msg(used_env, level, message, tags);
  return membrane_route_log(env, msg_env, wrapped_msg);
}


/**
 * Builds log message from string containing format and from va_list with parameters.
 * It also attaches :nif tag, and sends message to the log router.
 *
 * `env` - the environment of the calling process.
 *    Must be NULL if calling from a created thread.
 * `level` - integer describing log level
 * `format` and `va_args` - describe string that should be send
 *
 * On success, returns true. Otherwise, false is returned.
 */
static int membrane_log_format(ErlNifEnv *env, int level, const char *format, ...) {
  va_list args;
  unsigned char* binary_data;
  ErlNifEnv *used_env, *msg_env;
  ERL_NIF_TERM bin_term, tags_term, nif_tag;

  // check if received process-bound or process-independent environment
  if(env) {
    msg_env = NULL;
    used_env = env;
  } else {
    msg_env = enif_alloc_env();
    used_env = msg_env;
  }


  // compute the space that is needed to store whole message
  va_start(args, format);
  size_t output_size = vsnprintf(NULL, 0, format, args);
  va_end( args );

  // allocate binary
  binary_data = enif_make_new_binary(used_env, output_size, &bin_term);

  // print string to the binary
  va_start(args, format);
  vsnprintf(binary_data, output_size, format, args);
  va_end( args );

  // create tags_term
  nif_tag = enif_make_atom(used_env, "nif");
  tags_term = enif_make_list1(used_env, nif_tag);

  int ret = membrane_log(env, msg_env, level, bin_term, tags_term);

  if(!env) {
    enif_free_env(msg_env);
  }

  return ret;
}



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
