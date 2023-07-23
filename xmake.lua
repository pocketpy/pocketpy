set_project("pocketpy")

set_allowedplats("windows", "linux", "macosx", "wasm", "android")

option("dev", {default = true, showmenu = true, description = ""})

includes("dylib")

add_requires("python", {kind = "binary"})

add_rules("mode.debug", "mode.release")

set_languages("c++17")

add_cxflags("/utf-8", {tools = "cl"})

add_includedirs("include")

if is_plat("linux", "macosx") then
    add_syslinks("dl")
end

target("pocketpy")
    if has_config("dev") then
        set_kind("shared")
    else
        set_kind("$(kind)")
    end

    add_files("src/*.cpp")
    add_headerfiles("include/(**.h)")

    if is_plat("windows") and is_kind("shared") then
        add_rules("utils.symbols.export_all")
    end

    before_build(function (target)
        local python = assert(import("lib.detect.find_tool")("python3"), "python3 not found!")
        os.execv(python.program, {"prebuild.py"})
    end)

target("main")
    set_kind("binary")
    add_files("src2/main.cpp")
    add_deps("pocketpy")

    on_load(function (target)
        target:set("enabled", has_config("dev"))
    end)
