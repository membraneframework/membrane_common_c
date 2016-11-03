defmodule Mix.Tasks.Compile.Membrane.Compile.C do
  use Mix.Task

  @shortdoc "Compiles C code extensions for Membrane"

  
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
