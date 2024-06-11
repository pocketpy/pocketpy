set_project("pocketpy")
set_basename("pocketpy")
set_version("2.0.0")

set_languages("cxx17", "c11")
set_targetdir("build")
set_policy("build.warning", true)

option("pk_enable_profiler")
    set_default(false)
    add_defines("PK_ENABLE_PROFILER=1")
    
target("main")
	set_default(true)
	set_kind("binary")
	add_files("src/**.cpp", "src/**.c", "src2/*.cpp")
	add_includedirs("include/")

	set_warnings("allextra")
	if is_mode("release") then
		set_strip("all")
		set_optimize("faster")
		add_defines("NDEBUG")
	end

	if is_mode("debug") then
		set_optimize("none")
		set_symbols("debug")
		add_defines("DEBUG")
	end