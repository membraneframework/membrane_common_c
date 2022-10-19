defmodule Membrane.CommonC.Native.RawVideo do
  @moduledoc """
  Unifex compatible implementation of the Membrane.RawVideo struct.
  """

  @typedoc """
  Currently supported formats used to encode the color of every pixel in each video frame.
  """
  @type pixel_format_t ::
          :I420 | :I422 | :I444

  @typedoc """
  Width of single frame in pixels.
  """
  @type width_t :: pos_integer()

  @typedoc """
  Height of single frame in pixels.
  """
  @type height_t :: pos_integer()

  @typedoc """
  Numerator of number of frames per second. To avoid using tuple type,
  it is described by 2 separate integers number.
  """
  @type framerate_num_t :: non_neg_integer

  @typedoc """
  Denominator of number of frames per second. To avoid using tuple type,
  it is described by 2 separate integers number. Default value is 1.
  """
  @type framerate_den_t :: pos_integer

  @type t :: %__MODULE__{
          width: width_t(),
          height: height_t(),
          pixel_format: pixel_format_t(),
          framerate_num: framerate_num_t(),
          framerate_den: framerate_den_t(),
          aligned: boolean()
        }
  @enforce_keys [:width, :height, :pixel_format, :framerate_num, :aligned]
  defstruct width: nil,
            height: nil,
            pixel_format: nil,
            framerate_num: nil,
            framerate_den: 1,
            aligned: nil

  @supported_pixel_formats [:I420, :I422, :I444]

  @doc """
  Creates unifex compatible struct from Membrane.RawVideo struct.
  It may raise error when RawVideo with not supported pixel format is provided.
  """
  @spec from_membrane_raw_video(Membrane.RawVideo.t()) :: t()

  def from_membrane_raw_video(%Membrane.RawVideo{pixel_format: format} = membrane_raw_video)
      when format in @supported_pixel_formats do
    {framerate_num, framerate_den} = membrane_raw_video.framerate

    %__MODULE__{
      width: membrane_raw_video.width,
      height: membrane_raw_video.height,
      pixel_format: membrane_raw_video.pixel_format,
      framerate_num: framerate_num,
      framerate_den: framerate_den,
      aligned: membrane_raw_video.aligned
    }
  end

  def from_membrane_raw_video(_raw_video) do
    {:error, :not_supported_pixel_format}
  end

  @doc """
  Convert a native raw video to the Membrane.RawVideo counterpart.
  """
  @spec to_membrane_raw_video(t()) :: Membrane.RawVideo.t()
  def to_membrane_raw_video(native_raw_video) do
    %Membrane.RawVideo{
      width: native_raw_video.width,
      height: native_raw_video.height,
      pixel_format: native_raw_video.pixel_format,
      framerate: {native_raw_video.framerate_num, native_raw_video.framerate_den},
      aligned: native_raw_video.aligned
    }
  end
end
