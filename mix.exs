defmodule Membrane.Common.C.Mixfile do
  use Mix.Project
  Application.put_env(:bundlex, :membrane_common_c, __ENV__)

  def project do
    [
      app: :membrane_common_c,
      version: "0.0.1",
      elixir: "~> 1.6",
      elixirc_paths: elixirc_paths(Mix.env),
      compilers: [:bundlex] ++ Mix.compilers,
      description: "Membrane Multimedia Framework (C language common routines)",
      maintainers: ["Membrane Team"],
      licenses: ["LGPL"],
      name: "Membrane Common: C",
      source_url: "https://bitbucket.com/radiokit/membrane-common-c",
      deps: deps()
   ]
  end


  defp elixirc_paths(:test), do: ["lib", "test/support"]
  defp elixirc_paths(_),     do: ["lib"]

  defp deps() do
    [
      {:bundlex, git: "git@github.com:radiokit/bundlex.git"}
    ]
  end
end
