add_rules("mode.debug", "mode.release")

target("rt")
    set_kind("binary")
    add_files("rtBVH.cu")
    add_files("rtBuffer.cu")
    add_files("main.cpp")