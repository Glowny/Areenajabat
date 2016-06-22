ARENA_DIR = (path.getabsolute("..") .. "/")
local ARENA_THIRD_DIR = (ARENA_DIR .. "3rdparty/")
local ARENA_BUILD_DIR = (ARENA_DIR .. ".build/")
BGFX_DIR = (ARENA_THIRD_DIR .. "bgfx/")
BX_DIR = (ARENA_THIRD_DIR .. "bx/")

solution "arena"
	configurations {
		"debug",
		"release"
	}

	platforms {
		"x32",
		"x64"
	}

	language "C++"
	startproject "arena"
	configuration {}


function copyLib() end
function strip() end

configuration { "vs*" }
	includedirs { 
		BX_DIR .. "include/compat/msvc",
		path.join(ARENA_THIRD_DIR, "SDL2/include")
	} 
	links { "psapi"}

buildoptions {
	"/Oy-", -- Suppresses creation of frame pointers on the call stack.
	"/Ob2", -- The Inline Function Expansion
}
linkoptions {
	"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
	"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
	"/ignore:4099", -- LNK4099: The linker was unable to find your .pdb file.
}

defines {
	"__STDC_CONSTANT_MACROS",
	"__STDC_FORMAT_MACROS",
	"__STDC_LIMIT_MACROS",
	"BX_CONFIG_ENABLE_MSVC_LEVEL4_WARNINGS=1"
}

configuration {}

dofile("toolchain.lua")
dofile(BGFX_DIR .. "scripts/bgfx.lua")
toolchain(ARENA_BUILD_DIR, ARENA_THIRD_DIR)
--os.is("windows") and { "BGFX_CONFIG_RENDERER_DIRECT3D9=1" } or {
bgfxProject("", "StaticLib", os.is("windows") and { "BGFX_CONFIG_RENDERER_DIRECT3D11=1" } or { "BGFX_CONFIG_RENDERER_OPENGL=44" })
dofile(ARENA_DIR .. "scripts/shaderc.lua")
dofile(ARENA_DIR .. "scripts/texturec.lua")

project "Box2D"
	kind "StaticLib"
	files { 
		path.join(ARENA_THIRD_DIR, "Box2D", "**.h"),
		path.join(ARENA_THIRD_DIR, "Box2D", "**.cpp"),
	}
	vpaths { [""] = "Box2D" }
	includedirs { path.join(ARENA_THIRD_DIR) }	
	
project ("arena")
	kind ("ConsoleApp")

	includedirs {
		path.join(BX_DIR, "include"),
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty"),
		path.join(ARENA_THIRD_DIR),
		path.join(BGFX_DIR, "examples/common/imgui"),
	}

	links {
		"bgfx",
		"SDL2",
		"Box2D"
	}

	files {
		ARENA_DIR .. "src/**.cpp",
	}

	configuration { "vs*" and "x32"}
	links { "enet" }
	postbuildcommands {
		"XCOPY \"" .. path.join(ARENA_THIRD_DIR, "lib", "win32_" .. _ACTION) .. "\" \"$(TargetDir)\" /D /K /Y"
	}

	configuration { "vs*" and "x64"}
	links { "enet64" }
	postbuildcommands {
		"XCOPY \"" .. path.join(ARENA_THIRD_DIR, "lib", "win64_" .. _ACTION) .. "\" \"$(TargetDir)\" /D /K /Y"
	}

	configuration {}
	
project "server"
	kind "ConsoleApp"
	
	defines { 
		"ARENA_SERVER",
		"SFML_STATIC",
		"_HAS_ITERATOR_DEBUGGING=1"
	}
	
	includedirs { 
		ARENA_THIRD_DIR
	}
	
	files {
		path.join(ARENA_DIR, "network", "*.cpp"),
		path.join(ARENA_DIR, "network", "*.h"),
	}
	
	configuration { "vs*" and "x32"}
	links { 
		"enet",
		"ws2_32",
		"winmm"
	}
	
	configuration { "vs*" and "x64"}
	links { 
		"enet64",
		"ws2_32",
		"winmm",
		"sfml-window-s-d",
		"sfml-graphics-s-d",
		"sfml-system-s-d",
		"opengl32",
		"gdi32",
		"freetype",
		"jpeg"
	}
	
	configuration {}
	
project "client_sandbox"
	kind "ConsoleApp"

	defines { 
		"ARENA_CLIENT",
		"SFML_STATIC",
		"_HAS_ITERATOR_DEBUGGING=1"
	}
	
	includedirs { 
		ARENA_THIRD_DIR
	}
	
	files {
		path.join(ARENA_DIR, "network", "*.cpp"),
		path.join(ARENA_DIR, "network", "*.h"),
	}
	
	configuration { "vs*" and "x32"}
		links { 
		"enet",
		"ws2_32",
		"winmm"
		}

	configuration { "vs*" and "x64"}
	links { 
		"enet64",
		"ws2_32",
		"winmm",
		"sfml-window-s-d",
		"sfml-graphics-s-d",
		"sfml-system-s-d",
		"opengl32",
		"gdi32",
		"freetype",
		"jpeg"
	}
	
	
	configuration {}