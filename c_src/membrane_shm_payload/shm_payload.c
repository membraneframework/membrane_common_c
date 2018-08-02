#include "shm_payload.h"
#include "lib.h"

ErlNifResourceType *RES_SHM_PAYLOAD_GUARD_TYPE;

void res_shm_payload_guard_destructor(ErlNifEnv* env, void * resource) {
  UNUSED(env);

  ShmGuard *guard = (ShmGuard *) resource;
  shm_unlink(guard->name);
}

int load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info) {
  UNUSED(load_info);
  UNUSED(priv_data);

  int flags = ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER;
  RES_SHM_PAYLOAD_GUARD_TYPE =
    enif_open_resource_type(env, NULL, "ShmGuard", res_shm_payload_guard_destructor, flags, NULL);
  return 0;
}

static int create(ErlNifEnv* env, ShmPayload * payload, int *fd, ERL_NIF_TERM *return_term) {
  if (payload->name_len > NAME_MAX) {
    *return_term = membrane_util_make_error_args(env, "name", "Name to long");
    return -1;
  }

  *fd = shm_open(payload->name, O_RDWR | O_CREAT | O_EXCL, 0666);
  if (*fd < 0) {
    *return_term = membrane_util_make_error_errno(env, "shm_open");
    return -1;
  }

  int res = ftruncate(*fd, payload->capacity);
  if (res < 0) {
    *return_term = membrane_util_make_error_errno(env, "ftruncate");
    return -1;
  }

  ShmGuard *guard = enif_alloc_resource(RES_SHM_PAYLOAD_GUARD_TYPE, sizeof(*guard));
  strcpy(guard->name, payload->name);
  payload->guard = enif_make_resource(env, guard);
  enif_release_resource(guard);
  return 0;
}

static ERL_NIF_TERM export_create(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_SHM_PAYLOAD_ARG(0, payload);
  ERL_NIF_TERM return_term;
  int fd = -1;

  if (create(env, &payload, &fd, &return_term) == 0) {
    return_term = membrane_util_make_ok_tuple(env, shm_payload_make_term(env, &payload));
  }

  if (fd > 0) {
    close(fd);
  }
  return return_term;
}

static ERL_NIF_TERM export_set_capacity(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_SHM_PAYLOAD_ARG(0, payload);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, capacity);

  ShmPayloadLibResult result = shm_payload_set_capacity(&payload, capacity);
  if (SHM_PAYLOAD_RES_OK == result) {
    return membrane_util_make_ok_tuple(env, shm_payload_make_term(env, &payload));
  } else {
    return shm_payload_make_error_term(env, result);
  }
}

static ERL_NIF_TERM export_read(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_SHM_PAYLOAD_ARG(0, payload);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, size);

  ERL_NIF_TERM return_term;
  char * payload_ptr = MAP_FAILED;

  ShmPayloadLibResult result = shm_payload_open_and_mmap(&payload, &payload_ptr);
  if (SHM_PAYLOAD_RES_OK != result) {
    return_term = shm_payload_make_error_term(env, result);
    goto read_exit;
  }

  ERL_NIF_TERM out_bin_term;
  unsigned char * output_data = enif_make_new_binary(env, size, &out_bin_term);
  memcpy(output_data, payload_ptr, size);

  return_term = membrane_util_make_ok_tuple(env, out_bin_term);
read_exit:
  if (MAP_FAILED != payload_ptr) {
    munmap(payload_ptr, size);
  }
  return return_term;
}

static ERL_NIF_TERM export_write(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_SHM_PAYLOAD_ARG(0, payload);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(1, data);
  ERL_NIF_TERM return_term;

  if (payload.capacity < data.size) {
    shm_payload_set_capacity(&payload, data.size);
  }

  char * payload_ptr = MAP_FAILED;
  ShmPayloadLibResult result = shm_payload_open_and_mmap(&payload, &payload_ptr);
  if (SHM_PAYLOAD_RES_OK != result) {
    return_term = shm_payload_make_error_term(env, result);
    goto write_exit;
  }

  memcpy(payload_ptr, (void *)data.data, data.size);
  payload.size = data.size;
  return_term = membrane_util_make_ok_tuple(env, shm_payload_make_term(env, &payload));
write_exit:
  if (MAP_FAILED != payload_ptr) {
    munmap(payload_ptr, payload.capacity);
  }
  return return_term;
}

static ERL_NIF_TERM export_split_at(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_SHM_PAYLOAD_ARG(0, old_payload);
  MEMBRANE_UTIL_PARSE_SHM_PAYLOAD_ARG(1, new_payload);
  MEMBRANE_UTIL_PARSE_UINT_ARG(2, split_pos);

  ERL_NIF_TERM return_term;
  char * payload_ptr = MAP_FAILED;
  int new_fd = -1;
  int old_capacity = old_payload.capacity; //used to unmap payload
  ShmPayloadLibResult result = shm_payload_open_and_mmap(&old_payload, &payload_ptr);

  if (SHM_PAYLOAD_RES_OK != result) {
    return_term = shm_payload_make_error_term(env, result);
    goto split_at_exit;
  }

  if (create(env, &new_payload, &new_fd, &return_term) < 0) {
    goto split_at_exit;
  }

  int new_size = old_payload.size - split_pos;

  int res = write(new_fd, payload_ptr + split_pos, new_size);
  if (res < 0) {
    return_term = membrane_util_make_error_errno(env, "write");
    goto split_at_exit;
  }

  old_payload.size = split_pos;
  new_payload.size = res;

  return_term = membrane_util_make_ok_tuple(
    env,
    enif_make_tuple2(
      env,
      shm_payload_make_term(env, &old_payload),
      shm_payload_make_term(env, &new_payload)
    )
  );

split_at_exit:
  if (MAP_FAILED != payload_ptr) {
    munmap(payload_ptr, old_capacity);
  }
  if (new_fd > 0) {
    close(new_fd);
  }
  return return_term;
}

static ErlNifFunc nif_funcs[] = {
  {"create", 1, export_create, 0},
  {"set_capacity", 2, export_set_capacity, 0},
  {"read", 2, export_read, 0},
  {"write", 2, export_write, 0},
  {"split_at", 3, export_split_at, 0},
};

ERL_NIF_INIT(Elixir.Membrane.Payload.Shm.Native.Nif, nif_funcs, load, NULL, NULL, NULL)
