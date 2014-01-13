-- A solution contains projects, and defines the available configurations
solution "libpolymesh"
configurations { "Debug", "Release" }
includedirs {
	"T:/boost-build/include",
}
dofile "libpolymesh.lua"
dofile "pmd_test.lua"
dofile "bvh_test.lua"
dofile "mqo_test.lua"
dofile "x_test.lua"
dofile "vmd_test.lua"
