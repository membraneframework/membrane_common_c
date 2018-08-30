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
  @behaviour Membrane.Payload.Behaviour

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

  defstruct name: nil, guard: nil, size: 0, capacity: 4096

  @impl true
  @doc """
  Creates a new, empty Shm payload
  """
  def empty() do
    empty(4096)
  end

  @doc """
  Creates a new, empty Shm payload with the given capacity
  """
  @spec empty(pos_integer()) :: t()
  def empty(capacity) do
    {:ok, payload} = create(capacity)
    payload
  end

  @impl true
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
    {:ok, payload} = create(capacity)
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

  defp create(capacity) do
    shm_struct = %__MODULE__{capacity: capacity}
    Native.allocate(shm_struct)
  end
end

defimpl Membrane.Payload, for: Membrane.Payload.Shm do
  alias Membrane.Payload.Shm

  @spec empty(payload :: Shm.t()) :: Shm.t()
  def empty(%Shm{}), do: Shm.empty()

  @spec new(payload :: Shm.t(), binary()) :: Shm.t()
  def new(%Shm{}, data), do: Shm.new(data)

  @spec size(payload :: Shm.t()) :: non_neg_integer
  def size(%Shm{size: size}) do
    size
  end

  @spec split_at(payload :: Shm.t(), pos_integer) :: {Shm.t(), Shm.t()}
  def split_at(%Shm{size: size} = shm, at_pos) when 0 < at_pos and at_pos < size do
    {:ok, payloads} = Shm.Native.split_at(shm, at_pos)
    payloads
  end

  @spec concat(left :: Shm.t(), right :: Shm.t()) :: Shm.t()
  def concat(left, right) do
    {:ok, res} = Shm.Native.concat(left, right)
    res
  end

  @spec drop(payload :: Shm.t(), n :: non_neg_integer()) :: Shm.t()
  def drop(payload, n) do
    {:ok, new_payload} = Shm.Native.trim(payload, n)
    new_payload
  end

  @spec to_binary(payload :: Shm.t()) :: binary()
  def to_binary(payload) do
    {:ok, bin} = Shm.Native.read(payload)
    bin
  end

  @spec module(payload :: Shm.t()) :: module()
  def module(_), do: Shm
end
