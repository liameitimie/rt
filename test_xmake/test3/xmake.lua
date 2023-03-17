target("a")
    add_rules("utils.symbols.export_all", {export_classes = true})
    set_kind("shared")
    add_files("a.cpp")

target("test")
    add_files("test.cpp")
    add_deps("a")
