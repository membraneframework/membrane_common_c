defmodule Membrane.Payload.Shm do
  @moduledoc """
  This module allows using payload placed in POSIX shared memory on POSIX
  compliant systems.

  Defines an opaque struct implementing protocol `Membrane.Payload`.
  Struct should be passed to elements using native code. There are functions in
  `:membrane_shm_payload_lib` (it's native library that is exported via Bundlex)
  that will allow to transorm Elixir struct into C struct
  and then access the shared memory from the native code.
  """
  alias __MODULE__.Native

  @typedoc """
  Struct describing payload kept in shared memory.

  Should not be modified directly. Shared memory should be available as long
  as the associated struct is not garbage collected.
  """
  @opaque t :: %__MODULE__{
            name: binary(),
            guard: reference(),
            size: non_neg_integer(),
            capacity: pos_integer()
          }

  @enforce_keys [:name]
  defstruct name: nil, guard: nil, size: 0, capacity: 4096

  @doc """
  Creates a new Shm payload.

  If binary is passed as parameter, SHM will be initialized with the data.
  If a parameter is a number, SHM will have capacity of this size.

  See also `new/2`
  """
  @spec new(binary() | pos_integer()) :: t()
  def new(capacity \\ 4096)

  def new(capacity) when is_integer(capacity) do
    name = generate_name()
    {:ok, payload} = Native.create(%__MODULE__{name: name, capacity: capacity})
    payload
  end

  def new(data) when is_binary(data) do
    name = generate_name()
    {:ok, payload} = Native.create(%__MODULE__{name: name, capacity: byte_size(data)})
    {:ok, payload} = Native.write(payload, data)
    payload
  end

  @doc """
  Creates a new Shm payload initialized with `data` and set capacity.
  """
  @spec new(data :: binary(), capacity :: pos_integer()) :: t()
  def new(data, capacity) when is_binary(data) and is_integer(capacity) and capacity > 0 do
    name = generate_name()
    {:ok, payload} = Native.create(%__MODULE__{name: name, capacity: capacity})
    {:ok, payload} = Native.write(payload, data)
    payload
  end

  @doc """
  Sets the capacity of SHM.

  If the capacity is smaller than the current size, data will be discarded and size modified
  """
  @spec set_capacity(t(), pos_integer()) :: t()
  defdelegate set_capacity(payload, capacity), to: Native

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
