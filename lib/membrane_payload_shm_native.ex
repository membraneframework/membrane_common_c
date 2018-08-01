defmodule Membrane.Payload.Shm.Native do
  alias Membrane.Payload.Shm
  import Shm
  alias Membrane.Type
  use Bundlex.Loader, nif: :membrane_shm_payload

  @spec create(payload_record :: Shm.t()) :: Type.try_t(Shm.t())
  defnif create(payload_record)

  @spec create_and_init(payload_record :: Shm.t(), data :: binary()) :: Type.try_t(Shm.t())
  defnif create_and_init(payload_record, data)

  @spec set_capacity(payload_record :: Shm.t(), capacity :: pos_integer()) :: Type.try_t()
  defnif set_capacity(payload_record, capacity)

  @spec read(payload_record :: Shm.t()) :: Type.try_t(binary())
  def read(shm_payload(size: size) = payload_record) do
    read(payload_record, size)
  end

  @spec read(payload_record :: Shm.t(), size :: non_neg_integer()) :: Type.try_t(binary())
  defnif read(payload_record, size)

  @spec write(payload_record :: Shm.t(), data :: binary()) :: Type.try_t(Shm.t())
  defnif write(payload_record, data)

  @spec split_at(payload_record :: Shm.t(), new_payload_record :: Shm.t(), position :: non_neg_integer()) :: Type.try_t({Shm.t(), Shm.t()})
  defnif split_at(payload_record, new_payload_record, position)

  defnif test(record)
end
