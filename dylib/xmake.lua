set_languages("c11")

target("pocketpy-shared")
    set_kind("shared")
    add_files(path.join(os.projectdir(), "src2", "pocketpy_c.c"))

target("test")
    set_kind("shared")
    add_files("src/test.c")
    add_deps("pocketpy-shared")
