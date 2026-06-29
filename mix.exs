defmodule Membrane.Common.C.Mixfile do
  use Mix.Project

  @version "0.16.2"
  @github_url "https://github.com/membraneframework/membrane-common-c"

  def project do
    [
      app: :membrane_common_c,
      version: @version,
      elixir: "~> 1.12",
      elixirc_paths: elixirc_paths(Mix.env()),
      compilers: [:unifex, :bundlex] ++ Mix.compilers(),
      deps: deps(),

      # hex
      description: "Membrane Multimedia Framework (C language common routines)",
      package: package(),

      # docs
      name: "Membrane: Common C",
      source_url: @github_url,
      docs: docs(),
      homepage_url: "https://membrane.stream",
      aliases: [docs: ["docs", &prepend_llms_links/1]]
    ]
  end

  defp elixirc_paths(:test), do: ["lib", "test/support"]
  defp elixirc_paths(_), do: ["lib"]

  defp package do
    [
      maintainers: ["Membrane Team"],
      licenses: ["Apache-2.0"],
      files: ["lib", "c_src", "mix.exs", "README*", "LICENSE*", ".formatter.exs", "bundlex.exs"],
      links: %{
        "GitHub" => @github_url,
        "Membrane Framework Homepage" => "https://membraneframework.org"
      }
    ]
  end

  defp docs do
    [
      main: "readme",
      extras: ["README.md"],
      source_ref: "v#{@version}"
    ]
  end

  defp deps() do
    [
      {:ex_doc, ">= 0.40.0", only: :dev, runtime: false},
      {:membrane_core, "~> 1.0"},
      {:shmex, "~> 0.5.0"},
      {:unifex, "~> 1.0"}
    ]
  end

  defp prepend_llms_links(_) do
    output_dir = docs()[:output] || "doc"
    path = Path.join(output_dir, "llms.txt")

    if File.exists?(path) do
      existing = File.read!(path)

      footer = """


      ## See Also

      - [Membrane Framework AI Skill](https://hexdocs.pm/membrane_core/skill.md)
      - [Membrane Core](https://hexdocs.pm/membrane_core/llms.txt)
      """

      File.write!(path, String.trim_trailing(existing) <> footer)
    else
      IO.warn("#{path} not found — llms.txt was not generated, check your ex_doc configuration")
    end
  end
end
