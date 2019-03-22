defmodule Membrane.Common.C.BundlexProject do
  use Bundlex.Project

  def project do
    [
      libs: libs()
    ]
  end

  defp libs do
    [
      membrane: [
        deps: [unifex: :unifex],
        src_base: "membrane",
        sources: ["log.c", "_generated/log.c"]
      ],
      membrane_ringbuffer: [
        deps: [unifex: :unifex],
        src_base: "membrane_ringbuffer",
        sources: ["ringbuffer.c"]
      ]
    ]
  end
end
