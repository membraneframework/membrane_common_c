defmodule Mix.Tasks.Compile.Membrane.Native do
  def run(_args) do
    {command, target} = case :os.type do
      {:win32, _}     -> {"nmake", "windows"}
      {:unix, family} -> {"make", family}
    end

    case System.cmd(command, [target], stderr_to_stdout: true) do
      {result, 0} ->
        IO.binwrite(result)

      {result, _} ->
        IO.binwrite("Native compilation error:\n")
        IO.binwrite(result)
        throw :make
    end
  end
end


defmodule Membrane.Common.C.Mixfile do
  use Mix.Project

  def project do
    [app: :membrane_common_c,
     version: "0.0.1",
     elixir: "~> 1.3",
     elixirc_paths: elixirc_paths(Mix.env),
     description: "Membrane Multimedia Framework (C language common routines)",
     maintainers: ["Marcin Lewandowski"],
     licenses: ["LGPL"],
     name: "Membrane Common: C",
     source_url: "https://bitbucket.com/radiokit/membrane-common-c",
     preferred_cli_env: [espec: :test],
     deps: []]
  end


  defp elixirc_paths(:test), do: ["lib", "test/support"]
  defp elixirc_paths(_),     do: ["lib",]
end
