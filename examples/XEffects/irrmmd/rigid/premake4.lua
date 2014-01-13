-- A solution contains projects, and defines the available configurations
solution "sample"
configurations { "Debug", "Release" }

dofile "rigid.lua"
dofile "rigid_test.lua"
