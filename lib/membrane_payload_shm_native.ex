defmodule Membrane.Payload.Shm.Native do
  alias Membrane.Type
  use Bundlex.Loader, nif: :membrane_shm_payload

  @spec create(name :: String.t(), capacity :: pos_integer()) :: Type.try_t(reference())
  defnif create(name, capacity \\ 4096)

  @spec create_and_init(name :: String.t(), data :: binary()) :: Type.try_t(reference())
  defnif create_and_init(name, data)

  @spec set_capacity(name :: String.t(), capacity :: pos_integer()) :: Type.try_t()
  defnif set_capacity(name, capacity)

  @spec read(name :: String.t(), size :: non_neg_integer()) :: Type.try_t(binary())
  defnif read(name, size)

  @spec split_at(name :: String.t(), size :: non_neg_integer, name_created :: String.t(), position :: non_neg_integer()) :: Type.try_t(reference())
  defnif split_at(name, size, name_created, position)
end
