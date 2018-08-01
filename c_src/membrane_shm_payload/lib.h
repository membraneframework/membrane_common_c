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
} ShmPayload;

#define RECORD_ATOM "shm_payload"

enum RecordLayout {
  RECORD_ATOM_INDEX,
  RECORD_NAME_INDEX,
  RECORD_GUARD_INDEX,
  RECORD_SIZE_INDEX,
  RECORD_CAPACITY_INDEX,
  RECORD_FIELDS_NUM
};

typedef enum ShmPayloadLibResult {
  SHM_PAYLOAD_RES_OK,
  SHM_PAYLOAD_ERROR_SHM_OPEN,
  SHM_PAYLOAD_ERROR_FTRUNCATE,
  SHM_PAYLOAD_ERROR_MMAP
} ShmPayloadLibResult;

int payload_from_record(ErlNifEnv * env, ERL_NIF_TERM record, ShmPayload * payload);
ERL_NIF_TERM record_from_payload(ErlNifEnv * env, ShmPayload * payload);
ERL_NIF_TERM make_error_for_shm_payload_res(ErlNifEnv * env, ShmPayloadLibResult result);
ShmPayloadLibResult set_capacity(ShmPayload * payload, size_t capacity);
ShmPayloadLibResult open_and_mmap(ShmPayload * payload, char ** content);

#define MEMBRANE_UTIL_PARSE_SHM_PAYLOAD_ARG(position, var_name) \
  MEMBRANE_UTIL_PARSE_ARG(position, var_name, ShmPayload var_name, payload_from_record, &var_name)
