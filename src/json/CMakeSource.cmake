set(json_SOURCES
        json/Boost.cxx
        json/ParserOutputStream.cxx
        json/Serialize.cxx
        json/Reader.cpp
)

set(SCRIPT_FILES
    CMakeSource.cmake
)

file(GLOB_RECURSE HEADER_FILES "*.h;*.hxx;*.hpp")
