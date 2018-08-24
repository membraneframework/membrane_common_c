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
  @type t :: %__MODULE__{
          name: binary(),
          guard: reference(),
          size: non_neg_integer(),
          capacity: pos_integer()
        }

  @enforce_keys [:name]
  defstruct name: nil, guard: nil, size: 0, capacity: 4096

  @doc """
  Creates a new, empty Shm payload
  """
  @spec empty(pos_integer()) :: t()
  def empty(capacity \\ 4096) do
    name = generate_name()
    {:ok, payload} = Native.create(%__MODULE__{name: name, capacity: capacity})
    payload
  end

  @doc """
  Creates a new Shm payload from existing data.
  """
  @spec new(binary()) :: t()
  def new(data) when is_binary(data) do
    new(data, byte_size(data))
  end

  @doc """
  Creates a new Shm payload initialized with `data` and set capacity.

  The actual capacity is the greater of passed capacity and data size
  """
  @spec new(data :: binary(), capacity :: pos_integer()) :: t()
  def new(data, capacity) when capacity > 0 do
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
  def set_capacity(payload, capacity) do
    {:ok, new_payload} = Native.set_capacity(payload, capacity)
    new_payload
  end

  defp generate_name do
    "/membrane-#{inspect(System.system_time(:nanosecond))}-#{inspect(:rand.uniform(100))}"
  end
end

defimpl Membrane.Payload, for: Membrane.Payload.Shm do
  alias Membrane.Payload.Shm
  @spec size(payload :: Shm.t()) :: non_neg_integer
  def size(%Shm{size: size}) do
    size
  end

  @spec split_at(payload :: Shm.t(), pos_integer) :: {Shm.t(), Shm.t()}
  def split_at(%Shm{name: name, size: size} = shm, at_pos) when 0 < at_pos and at_pos < size do
    new_name = name <> "-2"
    {:ok, payloads} = Shm.Native.split_at(shm, %Shm{name: new_name}, at_pos)
    payloads
  end

  @spec concat(left :: Shm.t(), right :: Shm.t()) :: Shm.t()
  def concat(left, right) do
    {:ok, res} = Shm.Native.concat(left, right)
    res
  end

  @spec to_binary(payload :: Shm.t()) :: binary()
  def to_binary(payload) do
    {:ok, bin} = Shm.Native.read(payload)
    bin
  end

  @spec type(payload :: Shm.t()) :: :shm
  def type(_), do: :shm
end
