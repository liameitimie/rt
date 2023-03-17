add_requires("assimp")

target("shape-mesh")
    set_kind("shared")
    add_files("mesh.cpp","mesh.cu")
    add_includedirs(".")
    add_deps("base")
    add_packages("assimp")
target_end()