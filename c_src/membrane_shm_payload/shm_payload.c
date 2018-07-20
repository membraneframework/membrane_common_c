#include "shm_payload.h"

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

int create(ErlNifEnv* env, ErlNifBinary * name, int *fd, unsigned capacity, ERL_NIF_TERM *retval) {
  if (name->size > NAME_MAX) {
    *retval = membrane_util_make_error_args(env, "name", "Name to long");
    return -1;
  }

  ShmGuard *guard = enif_alloc_resource(RES_SHM_PAYLOAD_GUARD_TYPE, sizeof(*guard));
  strncpy(guard->name, (char *) name->data, name->size);
  guard->name[name->size] = '\0';

  ERL_NIF_TERM guard_term = enif_make_resource(env, guard);
  enif_release_resource(guard);

  *fd = shm_open(guard->name, O_RDWR | O_CREAT | O_EXCL, 0666);
  if (*fd < 0) {
    *retval = membrane_util_make_error_errno(env, "shm_open");
    return -1;
  }

  int res = ftruncate(*fd, capacity);
  if (res < 0) {
    *retval = membrane_util_make_error_errno(env, "ftruncate");
    return -1;
  }

  *retval = membrane_util_make_ok_tuple(env, guard_term);
  return 0;
}

static ERL_NIF_TERM export_create(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(0, name);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, capacity);
  ERL_NIF_TERM retval;
  int fd = -1;

  create(env, &name, &fd, capacity, &retval);

  if (fd > 0) {
    close(fd);
  }
  return retval;
}

static ERL_NIF_TERM export_create_with_data(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(0, name);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(1, data);
  ERL_NIF_TERM retval;
  int fd = -1;

  int res = create(env, &name, &fd, data.size, &retval);
  if (res < 0) {
    goto create_with_data_exit;
  }

  write(fd, data.data, data.size);

create_with_data_exit:
  if (fd > 0) {
    close(fd);
  }
  return retval;
}

static ERL_NIF_TERM export_set_capacity(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(0, name);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, capacity);

  ERL_NIF_TERM retval;
  int fd = -1;

  fd = shm_open((char *) name.data, O_RDWR, 0666);
  if (fd < 0) {
    retval = membrane_util_make_error_errno(env, "shm_open");
    goto set_capacity_exit;
  }

  int res = ftruncate(fd, capacity);
  if (res < 0) {
    retval = membrane_util_make_error_errno(env, "ftruncate");
    goto set_capacity_exit;
  }

  retval = membrane_util_make_ok(env);
set_capacity_exit:
  if (fd > 0) {
    close(fd);
  }
  return retval;
}

static ERL_NIF_TERM export_read(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(0, name);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, size);

  ERL_NIF_TERM retval;
  int fd = -1;
  char * content = MAP_FAILED;

  fd = shm_open((char *) name.data, O_RDONLY, 0666);
  if (fd < 0) {
    retval = membrane_util_make_error_errno(env, "shm_open");
    goto read_exit;
  }

  content = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  if (MAP_FAILED == content) {
    retval = membrane_util_make_error_errno(env, "mmap");
    goto read_exit;
  }

  ERL_NIF_TERM out_bin_term;
  unsigned char * output_data = enif_make_new_binary(env, size, &out_bin_term);
  memcpy(output_data, content, size);

  retval = membrane_util_make_ok_tuple(env, out_bin_term);
read_exit:
  if (fd > 0) {
    close(fd);
  }
  if (MAP_FAILED != content) {
    munmap(content, size);
  }
  return retval;
}

static ErlNifFunc nif_funcs[] = {
  {"create", 2, export_create, 0},
  {"create_and_init", 2, export_create_with_data, 0},
  {"set_capacity", 2, export_set_capacity, 0},
  {"read", 2, export_read, 0}
};

ERL_NIF_INIT(Elixir.Membrane.Payload.Shm.Native.Nif, nif_funcs, load, NULL, NULL, NULL)
