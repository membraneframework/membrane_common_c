/**
 * Membrane Common C routines: Logging.
 *
 * All Rights Reserved, (c) 2017 Matesz Nowak
 */

#ifndef __MEMBRANE_LOG_H__
#define __MEMBRANE_LOG_H__

#include <stdio.h>
#include <erl_nif.h>
#include <stdarg.h>

#define MEMBRANE_LOG_LEVEL_DEBUG 0
#define MEMBRANE_LOG_LEVEL_INFO 1
#define MEMBRANE_LOG_LEVEL_WARN 2

#define MEMBRANE_DEFAULT_TAG "Elixir.Membrane.Element.Unknown.Native"

#ifndef MEMBRANE_LOG_TAG
#define MEMBRANE_LOG_TAG MEMBRANE_DEFAULT_TAG
#endif

#define MEMBRANE_DEBUG(env, message, ...) membrane_log_format(env, MEMBRANE_LOG_LEVEL_DEBUG, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )
#define MEMBRANE_INFO(env, message, ...) membrane_log_format(env, MEMBRANE_LOG_LEVEL_INFO, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )
#define MEMBRANE_WARN(env, message, ...) membrane_log_format(env, MEMBRANE_LOG_LEVEL_WARN, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )
#define MEMBRANE_THREADED_DEBUG(message, ...) membrane_log_format(NULL, MEMBRANE_LOG_LEVEL_DEBUG, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )
#define MEMBRANE_THREADED_INFO(message, ...) membrane_log_format(NULL, MEMBRANE_LOG_LEVEL_INFO, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )
#define MEMBRANE_THREADED_WARN(message, ...) membrane_log_format(NULL, MEMBRANE_LOG_LEVEL_WARN, "[" MEMBRANE_LOG_TAG "] " message "\r\n", ##__VA_ARGS__ )

int membrane_log(ErlNifEnv *env, ErlNifEnv *msg_env, int level, ERL_NIF_TERM message, ERL_NIF_TERM tags);
int membrane_log_format(ErlNifEnv *env, int level, const char *format, ...);
ERL_NIF_TERM membrane_log_level_to_term(ErlNifEnv *env, int level);

#endif
