/**
 * Membrane Common C routines: Logging.
 *
 * All Rights Reserved, (c) 2017 Mateusz Nowak
 */

#include "log.h"

static ERL_NIF_TERM membrane_wrap_log_msg(ErlNifEnv *env, int level, ERL_NIF_TERM message_term, ERL_NIF_TERM tags_term);
static int membrane_route_log(ErlNifEnv *env, ErlNifEnv *msg_env, ERL_NIF_TERM term);


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
int membrane_log(ErlNifEnv *env, ErlNifEnv *msg_env, int level, ERL_NIF_TERM message, ERL_NIF_TERM tags) {
  ErlNifEnv *used_env;
  used_env = (env ? env : msg_env);

  ERL_NIF_TERM wrapped_msg = membrane_wrap_log_msg(used_env, level, message, tags);
  return membrane_route_log(env, msg_env, wrapped_msg);
}


/**
 * Builds log message from string containing format and from va_list with parameters.
 * It also attaches :nif tag and sends message to the log router.
 *
 * `env` - the environment of the calling process.
 *    Must be NULL if calling from a created thread.
 * `level` - integer describing log level
 * `format` and `va_args` - describe string that should be send
 *
 * On success, returns true. Otherwise, false is returned.
 */
int membrane_log_format(ErlNifEnv *env, int level, const char *format, ...) {
  va_list args;
  unsigned char* binary_data;
  ErlNifEnv *used_env, *msg_env;
  ERL_NIF_TERM bin_term, tags_term, nif_tag, element_tag;

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
  va_end(args);

  // allocate binary
  binary_data = enif_make_new_binary(used_env, output_size, &bin_term);

  // print string to the binary
  va_start(args, format);
  vsnprintf((char *)binary_data, output_size, format, args);
  va_end(args);

  // create tags_term
  nif_tag = enif_make_atom(used_env, "nif");
  element_tag = enif_make_atom(used_env, MEMBRANE_DEFAULT_TAG);
  tags_term = enif_make_list2(used_env, nif_tag, element_tag);

  int ret = membrane_log(env, msg_env, level, bin_term, tags_term);

  if(!env) {
    enif_free_env(msg_env);
  }

  return ret;
}


/**
 * Converts integer containing log level to atom.
 */
ERL_NIF_TERM membrane_log_level_to_term(ErlNifEnv *env, int level) {
  switch(level) {
    case 0:
      return enif_make_atom(env, "debug");
    case 1:
      return enif_make_atom(env, "info");
    case 2:
      return enif_make_atom(env, "warn");
    default:
      return enif_make_atom(env, "debug");
  }
}



// PRIVATE API

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
 * Wraps erlang terms to the format understandable by loggers, i.e.:
 * `{:membrane_log, :info, ["message"], timestamp_nsec, [:nif]}`
 * It also supplies message with current monotonic time.
 */
static ERL_NIF_TERM membrane_wrap_log_msg(ErlNifEnv *env, int level, ERL_NIF_TERM message_term, ERL_NIF_TERM tags_term) {

  time_t raw_time = time(NULL);
  struct tm * current_time = localtime(&raw_time);
  char time_iso8601[255];
  char format[] = "%Y-%m-%dT%H:%M:%SZ";
  strftime(time_iso8601, 255, format, current_time);

  ERL_NIF_TERM time_term = enif_make_string(env, time_iso8601, ERL_NIF_LATIN1);

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
