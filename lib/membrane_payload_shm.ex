defmodule Membrane.Payload.Shm do
  alias __MODULE__.Native

  @type t :: %__MODULE__{
    name: binary(),
    guard: reference(),
    size: non_neg_integer(),
    capacity: pos_integer()
  }

  @enforce_keys [:name, :guard, :capacity]
  defstruct name: nil, guard: nil, size: 0, capacity: 4096

  @spec empty(pos_integer()) :: t()
  def empty(capacity \\ 4096) do
    with name = generate_name(),
         {:ok, guard} <- Native.create(name, capacity)
    do
      %__MODULE__{name: name, guard: guard, capacity: capacity}
    end
  end

  @spec new(binary()) :: t()
  def new(data) do
    with name = generate_name(),
         {:ok, guard} <- Native.create_and_init(name, data)
    do
      %__MODULE__{name: name, guard: guard, capacity: byte_size(data)}
    end
  end

  @spec set_capacity(t(), pos_integer()) :: t()
  def set_capacity(%__MODULE__{name: name, size: size} = shm, capacity) do
    Native.set_capacity(name, capacity)
    new_size =
      if capacity < size do
        capacity
      else
        size
      end

    %__MODULE__{shm | size: new_size, capacity: capacity}
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

  @spec split_at(%Shm{}, non_neg_integer) :: {%Shm{}, %Shm{}}
  def split_at(%Shm{} = shm, 0) do
    {shm, Shm.empty()}
  end

  def split_at(%Shm{size: size} = shm, at_pos) when size <= at_pos do
    {Shm.empty(), shm}
  end

  def split_at(%Shm{name: name} = shm, at_pos) do
    {%Shm{shm | size: at_pos}, }
  end

  @spec from_binary(binary()) :: Shm.t()
  def from_binary(bin) do
    Shm.new(bin)
  end

  @spec to_binary(Shm.t()) :: binary()
  def to_binary(%Shm{name: name, size: size}) do
    {:ok, bin} = Shm.Native.read(name, size)
    bin
  end

end
