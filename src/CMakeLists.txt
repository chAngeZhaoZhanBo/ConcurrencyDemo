
add_library(LibThreadDemo INTERFACE)
target_include_directories(LibThreadDemo INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(LibThreadDemo INTERFACE pthread)

target_compile_options(LibThreadDemo INTERFACE -Wall -Werror -O3)