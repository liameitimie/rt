target("accel")
    set_kind("shared")
    add_includedirs(".",{public=true})
    add_files("accel.cpp")
    add_deps("util")
target_end()