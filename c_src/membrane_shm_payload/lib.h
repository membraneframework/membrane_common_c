#pragma once

#define NAME_MAX 255
#define _POSIX_C_SOURCE 200809L

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <erl_nif.h>
#include <membrane/membrane.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct {
  char * name;
  size_t name_len;
  ERL_NIF_TERM guard;
  unsigned int size;
  unsigned int capacity;
  void * mapped_memory;
  ERL_NIF_TERM elixir_struct_atom;
} ShmPayload;

#define SHM_PAYLOAD_ELIXIR_STRUCT_ENTRIES 5
#define SHM_PAYLOAD_ELIXIR_STRUCT_ATOM "Elixir.Membrane.Payload.Shm"

typedef enum ShmPayloadLibResult {
  SHM_PAYLOAD_RES_OK,
  SHM_PAYLOAD_ERROR_SHM_OPEN,
  SHM_PAYLOAD_ERROR_FTRUNCATE,
  SHM_PAYLOAD_ERROR_MMAP,
  SHM_PAYLOAD_ERROR_SHM_MAPPED,
  SHM_PAYLOAD_ERROR_NAME_TOO_LONG
} ShmPayloadLibResult;

int shm_payload_get_from_term(ErlNifEnv * env, ERL_NIF_TERM record, ShmPayload * payload);
ShmPayloadLibResult shm_payload_create(ShmPayload * payload);
void shm_payload_free(ShmPayload *payload);
ERL_NIF_TERM shm_payload_make_term(ErlNifEnv * env, ShmPayload * payload);
ERL_NIF_TERM shm_payload_make_error_term(ErlNifEnv * env, ShmPayloadLibResult result);
ShmPayloadLibResult shm_payload_set_capacity(ShmPayload * payload, size_t capacity);
ShmPayloadLibResult shm_payload_open_and_mmap(ShmPayload * payload);
void shm_payload_unmap(ShmPayload * payload);

#define MEMBRANE_UTIL_PARSE_SHM_PAYLOAD_ARG(position, var_name) \
  MEMBRANE_UTIL_PARSE_ARG(position, var_name, ShmPayload var_name, shm_payload_get_from_term, &var_name)
