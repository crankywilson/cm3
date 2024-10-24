cmake_minimum_required(VERSION 3.5.0)
project(cmul VERSION 0.1.0 LANGUAGES C CXX)

add_executable(cmul 
 main.cpp
 messages.cpp
 messages_reg.cpp
 Game.cpp
 Player.cpp
)

add_compile_definitions(UWS_NO_ZLIB LIBUS_NO_SSL)
include_directories( include include/uWebSockets include/uSockets )
target_link_libraries(cmul ${CMAKE_SOURCE_DIR}/lib/uSockets.a)

