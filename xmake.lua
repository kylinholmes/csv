add_rules("mode.debug", "mode.release")

target("csv")
    set_kind("binary")
    add_files("*.cpp")
