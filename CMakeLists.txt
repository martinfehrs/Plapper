cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

project(Plapper LANGUAGES CXX)

find_package(Catch2 REQUIRED)

add_library(PlapperLib)

target_compile_features(PlapperLib
        PRIVATE   cxx_std_23
        INTERFACE cxx_std_20)

target_sources(PlapperLib
        PUBLIC FILE_SET CXX_MODULES
        FILES
        plapper.ixx
        stack.ixx
        error.ixx
        terminal.ixx
        input_buffer.ixx
        environment.ixx
        dictionary.ixx
        core_words.ixx
        core_extension_words.ixx
        programming_tools_words.ixx
        core_types.ixx
        core_constants.ixx
        core_word_entries.ixx
        interpreter.ixx
        settings.ixx
        constant_size_literals.ixx
        type_traits.ixx
        memory_buffer.ixx
        type_params.ixx
)

set_property(TARGET PlapperLib PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set_property(TARGET PlapperLib PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET PlapperLib PROPERTY VISIBILITY_INLINES_HIDDEN ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(PlapperLib PRIVATE "$<$<CONFIG:Release>:-fwhole-program-vtables>")
    target_link_options(PlapperLib PRIVATE "$<$<CONFIG:Release>:-fwhole-program-vtables>")
endif()

target_include_directories(PlapperLib PRIVATE ${Boost_INCLUDE_DIRS})

add_executable(Plapper)

target_sources(Plapper
        PRIVATE
        main.cpp
)

set_property(TARGET Plapper PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set_property(TARGET Plapper PROPERTY CXX_VISIBILITY_PRESET hidden)
set_property(TARGET Plapper PROPERTY VISIBILITY_INLINES_HIDDEN ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(Plapper PRIVATE "$<$<CONFIG:Release>:-fwhole-program-vtables>")
    target_link_options(Plapper PRIVATE "$<$<CONFIG:Release>:-fwhole-program-vtables>")
endif()


target_link_libraries(Plapper PlapperLib)


add_executable(PlapperTest)

target_sources(PlapperTest
        PRIVATE
        test.cpp
)

target_link_libraries(PlapperTest PlapperLib)
target_link_libraries(PlapperTest Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(PlapperTest)