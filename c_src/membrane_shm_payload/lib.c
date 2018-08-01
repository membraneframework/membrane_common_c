#include "lib.h"

int payload_from_record(ErlNifEnv * env, ERL_NIF_TERM record, ShmPayload *payload) {
  int result;
  int arity;
  const ERL_NIF_TERM *record_array;
  result = enif_get_tuple(env, record, &arity, &record_array);
  if (!result) {
    return 0;
  }

  payload->guard = record_array[RECORD_GUARD_INDEX];

  result = enif_get_uint(env, record_array[RECORD_SIZE_INDEX], &payload->size);
  if (!result) {
    return 0;
  }

  result = enif_get_uint(env, record_array[RECORD_CAPACITY_INDEX], &payload->capacity);
  if (!result) {
    return 0;
  }

  ErlNifBinary name_binary;
  result = enif_inspect_binary(env, record_array[RECORD_NAME_INDEX], &name_binary);
  if (!result) {
    return 0;
  }
  payload->name = malloc(name_binary.size + 1);
  memcpy(payload->name, (char *) name_binary.data, name_binary.size);
  payload->name[name_binary.size] = '\0';
  payload->name_len = name_binary.size;

  return 1;
}

ERL_NIF_TERM record_from_payload(ErlNifEnv * env, ShmPayload * payload) {
  ERL_NIF_TERM record_array[RECORD_FIELDS_NUM];

  record_array[RECORD_ATOM_INDEX] = enif_make_atom(env, RECORD_ATOM);
  void * name_ptr = enif_make_new_binary(env, payload->name_len, &record_array[RECORD_NAME_INDEX]);
  memcpy(name_ptr, payload->name, payload->name_len);
  free(payload->name);

  record_array[RECORD_GUARD_INDEX] = payload->guard;
  record_array[RECORD_SIZE_INDEX] = enif_make_int(env, payload->size);
  record_array[RECORD_CAPACITY_INDEX] = enif_make_int(env, payload->capacity);

  return enif_make_tuple_from_array(env, record_array, RECORD_FIELDS_NUM);
}

ERL_NIF_TERM make_error_for_shm_payload_res(ErlNifEnv * env, ShmPayloadLibResult result) {
  switch (result) {
    case SHM_PAYLOAD_RES_OK:
      return membrane_util_make_error_internal(env, "ok_is_not_error");
    case SHM_PAYLOAD_ERROR_SHM_OPEN:
      return membrane_util_make_error_errno(env, "shm_open");
    case SHM_PAYLOAD_ERROR_FTRUNCATE:
      return membrane_util_make_error_errno(env, "ftruncate");
    case SHM_PAYLOAD_ERROR_MMAP:
      return membrane_util_make_error_errno(env, "mmap");
    default:
      return membrane_util_make_error_internal(env, "unknown_err");
  }
}

ShmPayloadLibResult set_capacity(ShmPayload * payload, size_t capacity) {
  ShmPayloadLibResult result;
  int fd = -1;

  fd = shm_open(payload->name, O_RDWR, 0666);
  if (fd < 0) {
    result = SHM_PAYLOAD_ERROR_SHM_OPEN;
    goto set_capacity_exit;
  }

  int res = ftruncate(fd, capacity);
  if (res < 0) {
    result = SHM_PAYLOAD_ERROR_FTRUNCATE;
    goto set_capacity_exit;
  }
  payload->capacity = capacity;
  if (payload->size > capacity) {
    // data was discarded with ftruncate, update size
    payload->size = capacity;
  }
  result = SHM_PAYLOAD_RES_OK;
set_capacity_exit:
  if (fd > 0) {
    close(fd);
  }
  return result;
}

ShmPayloadLibResult open_and_mmap(ShmPayload * payload, char ** content) {
  ShmPayloadLibResult result;
  int fd = -1;

  fd = shm_open(payload->name, O_RDWR, 0666);
  if (fd < 0) {
    result = SHM_PAYLOAD_ERROR_SHM_OPEN;
    goto open_and_mmap_exit;
  }

  *content = mmap(NULL, payload->capacity, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (MAP_FAILED == *content) {
    result = SHM_PAYLOAD_ERROR_MMAP;
    goto open_and_mmap_exit;
  }

  result = SHM_PAYLOAD_RES_OK;
open_and_mmap_exit:
  if (fd > 0) {
    close(fd);
  }
  return result;
}
