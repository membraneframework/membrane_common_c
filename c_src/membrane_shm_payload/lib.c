#include "lib.h"

int shm_payload_get_from_term(ErlNifEnv * env, ERL_NIF_TERM record, ShmPayload *payload) {
  const ERL_NIF_TERM ATOM_STRUCT_TAG = enif_make_atom(env, "__struct__");
  const ERL_NIF_TERM ATOM_NAME = enif_make_atom(env, "name");
  const ERL_NIF_TERM ATOM_GUARD = enif_make_atom(env, "guard");
  const ERL_NIF_TERM ATOM_SIZE = enif_make_atom(env, "size");
  const ERL_NIF_TERM ATOM_CAPACITY = enif_make_atom(env, "capacity");

  int result;
  ERL_NIF_TERM tmp_term;

  // Get guard
  result = enif_get_map_value(env, record, ATOM_GUARD, &tmp_term);
  if (!result) {
    return 0;
  }
  payload->guard = tmp_term;

  // Get Elixir struct tag
  result = enif_get_map_value(env, record, ATOM_STRUCT_TAG, &tmp_term);
  if (!result) {
    return 0;
  }
  payload->elixir_struct_tag = tmp_term;

  // Get size
  result = enif_get_map_value(env, record, ATOM_SIZE, &tmp_term);
  if (!result) {
    return 0;
  }
  result = enif_get_uint(env, tmp_term, &payload->size);
  if (!result) {
    return 0;
  }

  // Get capacity
  result = enif_get_map_value(env, record, ATOM_CAPACITY, &tmp_term);
  if (!result) {
    return 0;
  }
  result = enif_get_uint(env, tmp_term, &payload->capacity);
  if (!result) {
    return 0;
  }

  // Get name as last to prevent failure after allocating memory
  result = enif_get_map_value(env, record, ATOM_NAME, &tmp_term);
  if (!result) {
    return 0;
  }
  ErlNifBinary name_binary;
  result = enif_inspect_binary(env, tmp_term, &name_binary);
  if (!result) {
    return 0;
  }
  payload->name = malloc(name_binary.size + 1);
  memcpy(payload->name, (char *) name_binary.data, name_binary.size);
  payload->name[name_binary.size] = '\0';
  payload->name_len = name_binary.size;

  return 1;
}

ERL_NIF_TERM shm_payload_make_term(ErlNifEnv * env, ShmPayload * payload) {
  ERL_NIF_TERM keys[SHM_PAYLOAD_ELIXIR_STRUCT_ENTRIES] = {
    enif_make_atom(env, "__struct__"),
    enif_make_atom(env, "name"),
    enif_make_atom(env, "guard"),
    enif_make_atom(env, "size"),
    enif_make_atom(env, "capacity")
  };

  ERL_NIF_TERM name_term;
  void * name_ptr = enif_make_new_binary(env, payload->name_len, &name_term);
  memcpy(name_ptr, payload->name, payload->name_len);
  free(payload->name);

  ERL_NIF_TERM values[SHM_PAYLOAD_ELIXIR_STRUCT_ENTRIES] = {
    payload->elixir_struct_tag,
    name_term,
    payload->guard,
    enif_make_int(env, payload->size),
    enif_make_int(env, payload->capacity)
  };

  ERL_NIF_TERM return_term;
  int res = enif_make_map_from_arrays(env, keys, values, SHM_PAYLOAD_ELIXIR_STRUCT_ENTRIES, &return_term);
  if (res) {
    return return_term;
  } else {
    return membrane_util_make_error_internal(env, "make_map_from_arrays");
  }
}

ERL_NIF_TERM shm_payload_make_error_term(ErlNifEnv * env, ShmPayloadLibResult result) {
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

ShmPayloadLibResult shm_payload_set_capacity(ShmPayload * payload, size_t capacity) {
  ShmPayloadLibResult result;
  int fd = -1;

  fd = shm_open(payload->name, O_RDWR, 0666);
  if (fd < 0) {
    result = SHM_PAYLOAD_ERROR_SHM_OPEN;
    goto shm_payload_set_capacity_exit;
  }

  int res = ftruncate(fd, capacity);
  if (res < 0) {
    result = SHM_PAYLOAD_ERROR_FTRUNCATE;
    goto shm_payload_set_capacity_exit;
  }
  payload->capacity = capacity;
  if (payload->size > capacity) {
    // data was discarded with ftruncate, update size
    payload->size = capacity;
  }
  result = SHM_PAYLOAD_RES_OK;
shm_payload_set_capacity_exit:
  if (fd > 0) {
    close(fd);
  }
  return result;
}

ShmPayloadLibResult shm_payload_open_and_mmap(ShmPayload * payload, char ** content) {
  ShmPayloadLibResult result;
  int fd = -1;

  fd = shm_open(payload->name, O_RDWR, 0666);
  if (fd < 0) {
    result = SHM_PAYLOAD_ERROR_SHM_OPEN;
    goto shm_payload_open_and_mmap_exit;
  }

  *content = mmap(NULL, payload->capacity, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (MAP_FAILED == *content) {
    result = SHM_PAYLOAD_ERROR_MMAP;
    goto shm_payload_open_and_mmap_exit;
  }

  result = SHM_PAYLOAD_RES_OK;
shm_payload_open_and_mmap_exit:
  if (fd > 0) {
    close(fd);
  }
  return result;
}
