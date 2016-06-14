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

	configurations {}

function copyLib() end

configuration { "vs*" }
	includedirs { BX_DIR .. "include/compat/msvc"} 

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
bgfxProject("", "StaticLib", { "BGFX_CONFIG_RENDERER_OPENGL=44" })

project ("arena")
	kind ("ConsoleApp")

	includedirs {
		path.join(BX_DIR, "include"),
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty"),
		path.join(BGFX_DIR, "examples/common/imgui")
	}

	links {
		"bgfx"
	}

	files {
		ARENA_DIR .. "src/**.cpp",
	}

	configuration {}
