includes("a")
includes("b")

target("test")
    add_files("test.cpp")
    add_deps("a","b")
target_end()