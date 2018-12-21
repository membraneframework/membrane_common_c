/**
 * Membrane Common C routines: Logging.
 *
 * All Rights Reserved, (c) 2017 Mateusz Nowak
 */

#include "log.h"

#include <stdio.h>
#include <time.h>

static char *log_level_to_string(int level);
static void current_time_as_string(char time[255]);
static int send_to_log_router(UnifexEnv *find_env, UnifexEnv *msg_env,
                              int flags, char *level, char *msg, char *time_str,
                              char *tag);

/**
 * Builds log message from string containing format and from va_list with
 * parameters. It also attaches :nif tag and sends message to the log router.
 *
 * `env` - the environment of the calling process.
 *    Must be NULL if calling from a created thread.
 * `level` - integer describing log level
 * `format` and `va_args` - describe string that should be send
 *
 * On success, returns true. Otherwise, false is returned.
 */
int membrane_log(UnifexEnv *env, int level, char *log_tag, int is_threaded,
                 const char *format, ...) {
  va_list args;

  // compute the space that is needed to store whole message
  va_start(args, format);
  size_t msg_size = vsnprintf(NULL, 0, format, args);
  va_end(args);

  char *msg = unifex_alloc(msg_size + 3);

  // print message to string
  va_start(args, format);
  vsnprintf(msg, msg_size + 1, format, args);
  va_end(args);

  memcpy(msg + msg_size, "\r\n\0", 3);

  char *level_str = log_level_to_string(level);

  char time_str[255];
  current_time_as_string(time_str);

  int res;

  if (is_threaded && env == NULL) {
    UnifexEnv *msg_env = unifex_alloc_env();
    res = send_to_log_router(NULL, msg_env, UNIFEX_SEND_THREADED, level_str,
                             msg, time_str, log_tag);
    unifex_free_env(msg_env);
  } else if (is_threaded) {
    res = send_to_log_router(NULL, env, UNIFEX_SEND_THREADED, level_str, msg,
                             time_str, log_tag);
  } else {
    res = send_to_log_router(env, env, UNIFEX_NO_FLAGS, level_str, msg,
                             time_str, log_tag);
  }
  unifex_free(msg);

  return res;
}

static int send_to_log_router(UnifexEnv *find_env, UnifexEnv *msg_env,
                              int flags, char *level, char *msg, char *time_str,
                              char *tag) {
  UnifexPid router_pid;
  char *tags[] = {"nif", tag};

  return unifex_get_pid_by_name(find_env, "Elixir.Membrane.Log.Router",
                                &router_pid) &&
         send_membrane_log(msg_env, router_pid, flags, level, msg, time_str,
                           tags, 2);
}

static char *log_level_to_string(int level) {
  switch (level) {
  case 1:
    return "info";
  case 2:
    return "warn";
  default:
    return "debug";
  }
}

static void current_time_as_string(char time_str[255]) {
  time_t raw_time = time(NULL);
  struct tm *current_time = localtime(&raw_time);
  char format[] = "%Y-%m-%dT%H:%M:%SZ";
  strftime(time_str, 255, format, current_time);
}
