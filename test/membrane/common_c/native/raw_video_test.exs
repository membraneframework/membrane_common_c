defmodule Membrane.CommonC.Native.RawVideoTest do
  use ExUnit.Case

  alias Membrane.CommonC.Native

  test "From Membrane.RawVideo to Native conversion" do
    mem_raw_video = %Membrane.RawVideo{
      width: 1920,
      height: 1080,
      pixel_format: :I420,
      framerate: {30, 1},
      aligned: true
    }

    assert native_raw_video = Native.RawVideo.from_membrane_raw_video(mem_raw_video)
    assert Native.RawVideo.to_membrane_raw_video(native_raw_video) == mem_raw_video
  end
end
