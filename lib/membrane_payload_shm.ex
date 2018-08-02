defmodule Membrane.Payload.Shm do
  alias __MODULE__.Native

  @type t :: %__MODULE__{
    name: binary(),
    guard: reference(),
    size: non_neg_integer(),
    capacity: pos_integer()
  }

  @enforce_keys [:name]
  defstruct name: nil, guard: nil, size: 0, capacity: 4096

  @spec new(binary() | pos_integer()) :: t()
  def new(capacity \\ 4096)
  def new(capacity) when is_integer(capacity) do
    name = generate_name()
    {:ok, payload} = Native.create(%__MODULE__{name: name, capacity: capacity})
    payload
  end

  def new(data) when is_binary(data) do
    name = generate_name()
    {:ok, payload} = Native.create_and_init(%__MODULE__{name: name, capacity: byte_size(data)}, data)
    payload
  end

  @spec new(binary(), pos_integer()) :: t()
  def new(data, capacity) when is_binary(data) and is_integer(capacity) do
    name = generate_name()
    {:ok, payload} = Native.create_and_init(%__MODULE__{name: name, capacity: capacity}, data)
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

  @spec size(payload :: Shm.t()) :: non_neg_integer
  def size(%Shm{size: size}) do
    size
  end

  @spec split_at(Shm.t(), non_neg_integer) :: {Shm.t(), Shm.t()}
  def split_at(%Shm{} = payload, 0) do
    {payload, Shm.new()}
  end

  def split_at(%Shm{size: size} = shm, at_pos) when size <= at_pos do
    {Shm.new(), shm}
  end

  def split_at(%Shm{name: name} = shm, at_pos) do
    new_name = name <> "-2"
    {:ok, payloads} = Shm.Native.split_at(shm, %Shm{name: new_name}, at_pos)
    payloads
  end

  @spec to_binary(Shm.t()) :: binary()
  def to_binary(payload) do
    {:ok, bin} = Shm.Native.read(payload)
    bin
  end
end
