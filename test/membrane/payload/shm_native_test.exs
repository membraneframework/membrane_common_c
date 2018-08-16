defmodule Membrane.Payload.Shm.NativeTest do
  use ExUnit.Case, async: false
  alias Membrane.Payload.Shm
  @module Shm.Native

  @shm_name "/asdf"
  @shm_path Path.join("/dev/shm", @shm_name)

  @tag :shm_tmpfs
  test "create/1" do
    shm = %Shm{name: @shm_name}
    assert {:ok, new_shm} = @module.create(shm)
    assert new_shm.name == shm.name
    assert new_shm.guard != nil
    assert is_reference(new_shm.guard)
    assert new_shm.size == 0
    assert new_shm.capacity == shm.capacity

    assert {:ok, stat} = File.stat(@shm_path)
    assert stat.size == new_shm.capacity
  end

  describe "add_guard/1" do
    @tag :shm_tmpfs
    test "when SHM is not guarded" do
      assert File.touch(@shm_path) == :ok
      assert {:ok, shm} = @module.add_guard(%Shm{name: @shm_name})
      assert is_reference(shm.guard)
    end

    test "when shm is already guarded" do
      assert {:ok, shm} = @module.create(%Shm{name: @shm_name})
      assert @module.add_guard(shm) == {:error, :already_guarded}
    end
  end

  @tag :shm_tmpfs
  test "set_capacity/2" do
    new_capacity = 69
    assert {:ok, shm} = @module.create(%Shm{name: @shm_name})
    assert shm.capacity != new_capacity
    assert {:ok, shm} = @module.set_capacity(shm, new_capacity)
    assert shm.capacity == new_capacity

    assert {:ok, stat} = File.stat(@shm_path)
    assert stat.size == new_capacity
  end

  describe "write/2" do
    @describetag :shm_tmpfs
    setup :testing_data

    test "when written data size is smaller than capacity", %{data: data, data_size: data_size} do
      capacity = data_size + 10
      assert {:ok, shm} = @module.create(%Shm{name: @shm_name, capacity: capacity})
      assert {:ok, shm} = @module.write(shm, data)

      assert {:ok, stat} = File.stat(@shm_path)
      assert stat.size == capacity

      assert <<tested_head::binary-size(data_size), _::binary>> = File.read!(@shm_path)
      assert tested_head == data
      # There has to be some reference to shm struct here to prevent garbage collection
      # of shm guard before accessing shm via tmpfs
      assert shm.size == data_size
      assert shm.capacity == capacity
    end

    test "when written data size is greater than capacity", %{data: data, data_size: data_size} do
      capacity = data_size - 6
      assert capacity < data_size
      assert {:ok, shm} = @module.create(%Shm{name: @shm_name, capacity: capacity})
      assert {:ok, shm} = @module.write(shm, data)

      assert {:ok, stat} = File.stat(@shm_path)
      assert stat.size == data_size

      assert File.read!(@shm_path) == data
      assert shm.size == data_size
      assert shm.capacity == data_size
    end
  end

  describe "read/1" do
    setup :testing_data

    test "from empty shm" do
      assert {:ok, shm} = @module.create(%Shm{name: @shm_name})
      assert @module.read(shm) == {:ok, ""}
    end

    test "from non-empty shm", %{data: data} do
      assert {:ok, shm} = @module.create(%Shm{name: @shm_name})
      assert {:ok, shm} = @module.write(shm, data)

      assert @module.read(shm) == {:ok, data}
    end
  end

  describe "read/2 from non-empty shm" do
    setup :testing_data

    test "of size 0", %{data: data} do
      assert {:ok, shm} = @module.create(%Shm{name: @shm_name})
      assert {:ok, shm} = @module.write(shm, data)
      assert @module.read(shm, 0) == {:ok, ""}
    end

    test "of size smaller than data size", %{data: data} do
      assert {:ok, shm} = @module.create(%Shm{name: @shm_name})
      assert {:ok, shm} = @module.write(shm, data)
      size = 6
      assert {:ok, data_read} = @module.read(shm, size)
      <<data_part::binary-size(size), _::binary>> = data
      assert data_read == data_part
    end
  end

  describe "split_at/3" do
    setup :testing_data

    test "", %{data: data, data_size: data_size} do
      assert {:ok, shm_a} = @module.create(%Shm{name: @shm_name})
      assert {:ok, shm_a} = @module.write(shm_a, data)

      new_name = @shm_name <> "2"
      split_pos = 6
      assert {:ok, {shm_a, shm_b}} = @module.split_at(shm_a, %Shm{name: new_name}, split_pos)

      <<data_a::binary-size(split_pos), data_b::binary>> = data
      assert @module.read(shm_a) == {:ok, data_a}
      assert @module.read(shm_b) == {:ok, data_b}
      assert shm_a.size == split_pos
      assert shm_b.size == data_size - split_pos
    end
  end

  def testing_data(_) do
    data = "some testing data"

    [
      data: data,
      data_size: byte_size(data)
    ]
  end
end
