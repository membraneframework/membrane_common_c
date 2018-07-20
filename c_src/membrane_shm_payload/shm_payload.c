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

int create(ErlNifEnv* env, ErlNifBinary * name, unsigned capacity, int *fd, ERL_NIF_TERM *return_term) {
  if (name->size > NAME_MAX) {
    *return_term = membrane_util_make_error_args(env, "name", "Name to long");
    return -1;
  }

  ShmGuard *guard = enif_alloc_resource(RES_SHM_PAYLOAD_GUARD_TYPE, sizeof(*guard));
  strncpy(guard->name, (char *) name->data, name->size);
  guard->name[name->size] = '\0';

  ERL_NIF_TERM guard_term = enif_make_resource(env, guard);
  enif_release_resource(guard);

  *fd = shm_open(guard->name, O_RDWR | O_CREAT | O_EXCL, 0666);
  if (*fd < 0) {
    *return_term = membrane_util_make_error_errno(env, "shm_open");
    return -1;
  }

  int res = ftruncate(*fd, capacity);
  if (res < 0) {
    *return_term = membrane_util_make_error_errno(env, "ftruncate");
    return -1;
  }

  *return_term = membrane_util_make_ok_tuple(env, guard_term);
  return 0;
}

static ERL_NIF_TERM export_create(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(0, name);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, capacity);
  ERL_NIF_TERM return_term;
  int fd = -1;

  create(env, &name, capacity, &fd, &return_term);

  if (fd > 0) {
    close(fd);
  }
  return return_term;
}

static ERL_NIF_TERM export_create_with_data(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(0, name);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(1, data);
  ERL_NIF_TERM return_term;
  int fd = -1;

  int res = create(env, &name, data.size, &fd, &return_term);
  if (res < 0) {
    goto create_with_data_exit;
  }

  write(fd, data.data, data.size);

create_with_data_exit:
  if (fd > 0) {
    close(fd);
  }
  return return_term;
}

static ERL_NIF_TERM export_set_capacity(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(0, name);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, capacity);

  ERL_NIF_TERM return_term;
  int fd = -1;

  fd = shm_open((char *) name.data, O_RDWR, 0666);
  if (fd < 0) {
    return_term = membrane_util_make_error_errno(env, "shm_open");
    goto set_capacity_exit;
  }

  int res = ftruncate(fd, capacity);
  if (res < 0) {
    return_term = membrane_util_make_error_errno(env, "ftruncate");
    goto set_capacity_exit;
  }

  return_term = membrane_util_make_ok(env);
set_capacity_exit:
  if (fd > 0) {
    close(fd);
  }
  return return_term;
}

static int open_and_mmap(ErlNifEnv * env, ErlNifBinary * name, unsigned size, char ** content, ERL_NIF_TERM * return_term) {
  int fd = -1;
  char * name_cstr = malloc(name->size);
  strncpy(name_cstr, (char *) name->data, name->size);
  name_cstr[name->size] = '\0';
  int result = 0;

  fd = shm_open(name_cstr, O_RDONLY, 0666);
  if (fd < 0) {
    *return_term = membrane_util_make_error_errno(env, "shm_open");
    result = -1;
    goto open_and_mmap_exit;
  }

  *content = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  if (MAP_FAILED == content) {
    *return_term = membrane_util_make_error_errno(env, "mmap");
    result = -1;
    goto open_and_mmap_exit;
  }
open_and_mmap_exit:
  free(name_cstr);
  if (fd > 0) {
    close(fd);
  }
  return result;
}

static ERL_NIF_TERM export_read(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(0, name);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, size);

  ERL_NIF_TERM return_term;
  char * content = MAP_FAILED;

  if (open_and_mmap(env, &name, size, &content, &return_term) < 0) {
    goto read_exit;
  }

  ERL_NIF_TERM out_bin_term;
  unsigned char * output_data = enif_make_new_binary(env, size, &out_bin_term);
  memcpy(output_data, content, size);

  return_term = membrane_util_make_ok_tuple(env, out_bin_term);
read_exit:
  if (MAP_FAILED != content) {
    munmap(content, size);
  }
  return return_term;
}

static ERL_NIF_TERM export_split_at(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  UNUSED(argc);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(0, old_name);
  MEMBRANE_UTIL_PARSE_UINT_ARG(1, old_size);
  MEMBRANE_UTIL_PARSE_BINARY_ARG(2, new_name);
  MEMBRANE_UTIL_PARSE_UINT_ARG(3, split_pos);

  ERL_NIF_TERM return_term;
  char * content = MAP_FAILED;
  int new_fd = -1;

  if (open_and_mmap(env, &old_name, old_size, &content, &return_term) < 0) {
    goto split_at_exit;
  }

  int new_size = old_size - split_pos;
  if (create(env, &new_name, new_size, &new_fd, &return_term) < 0) {
    goto split_at_exit;
  }

  write(new_fd, content + split_pos, new_size);
  memset(content + split_pos, 0, new_size);

split_at_exit:
  if (MAP_FAILED != content) {
    munmap(content, old_size);
  }
  if (new_fd > 0) {
    close(new_fd);
  }
  return return_term;
}

static ErlNifFunc nif_funcs[] = {
  {"create", 2, export_create, 0},
  {"create_and_init", 2, export_create_with_data, 0},
  {"set_capacity", 2, export_set_capacity, 0},
  {"read", 2, export_read, 0},
  {"split_at", 4, export_split_at, 0}
};

ERL_NIF_INIT(Elixir.Membrane.Payload.Shm.Native.Nif, nif_funcs, load, NULL, NULL, NULL)
