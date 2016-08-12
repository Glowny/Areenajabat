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
		path.join(BX_DIR, "include/compat/msvc")
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

project ("arena")
	kind ("ConsoleApp")

	includedirs {
		path.join(BX_DIR, "include"),
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty"),
		path.join(ARENA_THIRD_DIR),
		ARENA_DIR
	}
    
    defines {
        "RAPIDXML_NO_EXCEPTIONS=1"
    }

	links {
		"bgfx",
		"SDL2",
		"Box2D",
		"spriterengine",
		"enet",
		"common"
	}

	files {
		ARENA_DIR .. "src/**.cpp",
		ARENA_DIR .. "src/**.h",
		ARENA_DIR .. "src/**.inl"
	}

	configuration { "vs*"}
		links {
			"ws2_32", --winsock
			"winmm",
		}

	configuration { "vs*" }
	postbuildcommands {
		"XCOPY \"" .. path.join(ARENA_DIR, "assets") .. "\" \"$(TargetDir)assets\\\" /D /K /Y /E"
	}

	configuration { "vs*" and "x32"}
	postbuildcommands {
		"XCOPY \"" .. path.join(ARENA_THIRD_DIR, "lib", "win32_" .. _ACTION) .. "\" \"$(TargetDir)\" /D /K /Y"
	}

	configuration { "vs*" and "x64"}
	postbuildcommands {
		"XCOPY \"" .. path.join(ARENA_THIRD_DIR, "lib", "win64_" .. _ACTION) .. "\" \"$(TargetDir)\" /D /K /Y"
	}

    configuration { "linux*" }
        buildoptions_cpp {
            "-std=c++11"
        }
        links { 
            "dl",
            "pthread"
        }

	configuration {}

project "common"
	kind "StaticLib"
	language "C++"

	files {
		path.join(ARENA_DIR, "common", "**.cpp"),
		path.join(ARENA_DIR, "common", "**.h"),
		path.join(ARENA_DIR, "common", "**.inl")
	}

	includedirs {
		ARENA_THIRD_DIR,
		path.join(BX_DIR , "include")
	}


    configuration { "linux*" }
        buildoptions_cpp {
            "-std=c++11"
        }

    configuration { }


project "server"
	kind "ConsoleApp"

	includedirs {
		ARENA_THIRD_DIR,
		path.join(BX_DIR, "include/"),
		path.join(ARENA_DIR)
	}

	files {
		path.join(ARENA_DIR, "network", "**.cpp"),
		path.join(ARENA_DIR, "network", "**.h"),
		path.join(ARENA_DIR, "network", "**.inl")
	}

	links {
		"common",
		"enet",
		"ws2_32",
		"winmm",
		"Box2D",
		"minini"
	}


    configuration { "linux*" }
        buildoptions_cpp {
            "-std=c++11"
        }


	configuration {}

group("libs")

bgfxProject("", "StaticLib", os.is("windows") and { "BGFX_CONFIG_RENDERER_DIRECT3D11=1" } or { "BGFX_CONFIG_RENDERER_OPENGL=44" })

project "spriterengine"
	kind "StaticLib"

	files {
		path.join(ARENA_THIRD_DIR, "spriterengine", "**.cpp"),
		path.join(ARENA_THIRD_DIR, "spriterengine", "**.h")
	}

	configuration { "vs*" }

	buildoptions {
		"/wd4244", --return': conversion from '__int64' to 'int', possible loss of data
		"/wd4267",  --'return': conversion from 'size_t' to 'int', possible loss of data
		"/wd4800", --'int': forcing value to bool 'true' or 'false' (performance warning)
		"/wd4018", --warning C4018: '<': signed/unsigned mismatch
	}

    configuration { "linux*" }
        buildoptions_cpp {
            "-std=c++11"
        }


	configuration {}
if os.is("windows") then
project "enet"
	kind "StaticLib"
	language "C"

	files {
		path.join(ARENA_THIRD_DIR, "enet", "*.c")
	}

	includedirs { ARENA_THIRD_DIR }

	configuration { "debug" }
		defines { "DEBUG"}
		flags { "Symbols" }

	configuration { "Release" }
		defines { "NDEBUG"}
		flags { "Optimize" }

	configuration { "vs*" }
		defines { "_WINSOCK_DEPRECATED_NO_WARNINGS" }

	configuration {}
end

project "Box2D"
	kind "StaticLib"
	files {
		path.join(ARENA_THIRD_DIR, "Box2D", "**.h"),
		path.join(ARENA_THIRD_DIR, "Box2D", "**.cpp")
	}
	vpaths { [""] = "Box2D" }
	includedirs {
		path.join(ARENA_THIRD_DIR)
	}

project "minini"
	kind "StaticLib"
	language "C"
	files {
		path.join(ARENA_THIRD_DIR, "minini", "**.h"),
		path.join(ARENA_THIRD_DIR, "minini", "**.c")
	}
	vpaths { [""] = "minini" }
	includedirs {
		path.join(ARENA_THIRD_DIR)
	}


group("tools")
dofile(ARENA_DIR .. "scripts/shaderc.lua")
dofile(ARENA_DIR .. "scripts/texturec.lua")
