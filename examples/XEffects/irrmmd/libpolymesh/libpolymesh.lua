-- A project defines one build target
project "libpolymesh"
--kind "WindowedApp"
--kind "ConsoleApp"
--kind "SharedLib"
kind "StaticLib"
language "C++"
files { "**.h", "**.cpp" }
excludes { "*_test.cpp" }
flags {}
buildoptions {}
defines {}
includedirs {
  "../rigid",
}
linkoptions {}
libdirs {}
links {}

configuration "gmake"
do
  buildoptions { "-Wall" }
end

configuration "vs*"
do
  buildoptions { "/wd4996" }
end

configuration "Debug"
do
  defines { "DEBUG" }
  flags { "Symbols" }
  targetdir "../debug"
end

configuration "Release"
do
  defines { "NDEBUG" }
  flags { "Optimize" }
  targetdir "../release"
end

