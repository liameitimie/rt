includes("*")

target("rt")
    add_files("rt.cpp")
    add_deps("base")
    set_rundir("..")
target_end()