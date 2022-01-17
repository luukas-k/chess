-- premake5.lua
workspace "chess_gl"
   configurations { "Debug", "Release" }

project "chess_gl"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   architecture "x86_64"
   targetdir "bin/%{cfg.buildcfg}"

   files { "src/**.h", "src/**.cpp" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"