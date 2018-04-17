defmodule Membrane.Common.C.BundlexProject do
  use Bundlex.Project

  def project do
    [
      nif: nif(Bundlex.platform)
    ]
  end

  defp nif(_platform) do
    [
      membrane: [
        export_only?: Mix.env != :test,
        src_base: "membrane",
        sources: ["membrane.c", "log.c"]
      ],
      membrane_ringbuffer: [
        export_only?: Mix.env != :test,
        src_base: "membrane_ringbuffer",
        sources: ["ringbuffer.c", "portaudio/pa_ringbuffer.c"]
      ]
    ]
  end

end
