defmodule Membrane.Common.C.BundlexProject do
  use Bundlex.Project

  def project do
    [
      nif: nif(Bundlex.platform)
    ]
  end

  defp nif(_platform) do
    [
      log: [
        sources: ["membrane/log.c"]
      ]
    ]
  end

end
