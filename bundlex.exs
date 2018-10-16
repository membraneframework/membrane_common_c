defmodule Membrane.Common.C.BundlexProject do
  use Bundlex.Project

  def project do
    [
      nifs: nifs(Bundlex.platform())
    ]
  end

  defp nifs(_platform) do
    [
      membrane: [
        deps: [unifex: :unifex],
        export_only?: Mix.env() != :test,
        src_base: "membrane",
        sources: ["log.c", "_generated/log.c"]
      ],
      membrane_ringbuffer: [
        deps: [unifex: :unifex],
        export_only?: Mix.env() != :test,
        src_base: "membrane_ringbuffer",
        sources: ["ringbuffer.c"]
      ]
    ]
  end
end
