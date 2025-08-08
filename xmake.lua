add_rules("mode.debug", "mode.release")
set_languages("c++17")  

target("reader")
    set_kind("binary")
    add_files("bench.cpp")


target("writer")
    set_kind("binary")
    add_files("writer.cpp")