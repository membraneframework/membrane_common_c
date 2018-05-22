defmodule Membrane.Common.C.Mixfile do
  use Mix.Project
  Application.put_env(:bundlex, :membrane_common_c, __ENV__)

  def project do
    [
      app: :membrane_common_c,
      version: "0.0.1",
      elixir: "~> 1.6",
      elixirc_paths: elixirc_paths(Mix.env()),
      compilers: [:bundlex] ++ Mix.compilers(),
      description: "Membrane Multimedia Framework (C language common routines)",
      package: package(),
      name: "Membrane Common: C",
      source_url: "https://github.com/membraneframework/membrane-common-c",
      deps: deps()
    ]
  end

  defp elixirc_paths(:test), do: ["lib", "test/support"]
  defp elixirc_paths(_), do: ["lib"]

  defp package do
    [
      maintainers: ["Membrane Team"],
      licenses: ["Apache 2.0"]
    ]
  end

  defp deps() do
    [
      {:bundlex, git: "git@github.com:radiokit/bundlex.git"}
    ]
  end
end
