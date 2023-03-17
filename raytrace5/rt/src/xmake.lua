add_includedirs("$curdir")

includes("rt")

target("rtapp")
    set_kind("binary")
    add_files("rtapp.cpp")