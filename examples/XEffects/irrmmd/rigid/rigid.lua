-- A project defines one build target
project "rigid"
--kind "WindowedApp"
--kind "ConsoleApp"
--kind "SharedLib"
kind "StaticLib"
language "C++"
files { "**.h", "**.cpp" }
excludes { "main.cpp" }
flags {}
buildoptions {}
defines {}
linkoptions {}
libdirs {}

configuration "gmake"
do
  libdirs { "E:/mingw/lib" }
  buildoptions { "-Wall" }
end

configuration "vs*"
do
  buildoptions { "/wd4738" }
end

configuration "windows"
do
  defines {
    "NOMINMAX", "_USE_MATH_DEFINES",
  }
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

