defmodule Membrane.Payload.Shm do
  alias __MODULE__.Native

  @type t :: record(
    :shm_payload,
    name: binary(),
    guard: reference(),
    size: non_neg_integer(),
    capacity: pos_integer()
  )

  require Record
  Record.defrecord :shm_payload, [name: nil, guard: nil, size: 0, capacity: 4096]

  @spec empty(pos_integer()) :: t()
  def empty(capacity \\ 4096) do
    name = generate_name()
    {:ok, payload} = Native.create(shm_payload(name: name, capacity: capacity))
    payload
  end

  @spec new(binary()) :: t()
  def new(data) do
    name = generate_name()
    {:ok, payload} = Native.create_and_init(shm_payload(name: name, capacity: byte_size(data)), data)
    payload
  end

  @spec set_capacity(t(), pos_integer()) :: t()
  def set_capacity(payload, capacity) do
    Native.set_capacity(payload, capacity)
  end

  defp generate_name do
    "/membrane_#{inspect(:os.system_time())}"
  end

end

defimpl Membrane.Payload, for: Membrane.Payload.Shm do
  alias Membrane.Payload.Shm
  import Shm

  @spec size(payload :: Shm.t()) :: non_neg_integer
  def size(shm_payload(size: size)) do
    size
  end

  @spec split_at(Shm.t(), non_neg_integer) :: {Shm.t(), Shm.t()}
  def split_at(shm_payload() = payload, 0) do
    {payload, Shm.empty()}
  end

  def split_at(shm_payload(size: size) = shm, at_pos) when size <= at_pos do
    {Shm.empty(), shm}
  end

  def split_at(shm_payload(name: name) = shm, at_pos) do
    new_name = name <> "-2"
    {:ok, payloads} = Shm.Native.split_at(shm, shm_payload(name: new_name), at_pos)
    payloads
  end

  @spec from_binary(binary()) :: Shm.t()
  def from_binary(bin) do
    Shm.new(bin)
  end

  @spec to_binary(Shm.t()) :: binary()
  def to_binary(payload) do
    {:ok, bin} = Shm.Native.read(payload)
    bin
  end

end
