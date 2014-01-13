-- A project defines one build target
project "sample"
--kind "WindowedApp"
kind "ConsoleApp"
--kind "SharedLib"
--kind "StaticLib"
language "C++"
files { "main.cpp" }
flags { "WinMain" }
buildoptions {}
defines {}
linkoptions {}

configuration "gmake"
do
links { "gtest", "gtest_main", "rigid"  }
  libdirs { "E:/mingw/lib" }
  includedirs { "E:/mingw/include/gtest" }
  buildoptions { "-Wall" }
end

configuration "vs*"
do
links { "rigid"  }
  includedirs { "E:/gtest-1.4.0/include" }
  buildoptions { "-Wall" }
end

configuration "Debug"
do
  defines { "DEBUG" }
  flags { "Symbols" }
  targetdir "../debug"
  libdirs{ 
    "E:/gtest-1.4.0/msvc/gtest-md/Debug",
    "../debug",
  }
  links {
    "gtest_main-mdd", "gtestd", 
  }
end

configuration "Release"
do
  defines { "NDEBUG" }
  flags { "Optimize" }
  targetdir "../release"
  libdirs{ 
    "E:/gtest-1.4.0/msvc/gtest-md/Release", 
    "../release",
  }
  links {
    "gtest_main-md", "gtest", 
  }
end

