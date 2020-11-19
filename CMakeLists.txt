cmake_minimum_required(VERSION 2.8)

project(Truva)

#set(INC_DIR ${CMAKE_SOURCE_DIR}/inc)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/App/server)

set(EX_INC_DIR ${CMAKE_SOURCE_DIR}/Socket/inc)
set(EX_SRC_DIR ${CMAKE_SOURCE_DIR}/Socket)

set(SRC_FILES   ${SRC_DIR}/server.c)

set(EX_SRC_FILES    ${EX_SRC_DIR}/eaSCKTBasicComProtocols.c 
                    ${EX_SRC_DIR}/esSCKTBaseDef.c)

include_directories(${EX_INC_DIR})

add_executable(server ${SRC_FILES} ${EX_SRC_FILES})

target_link_libraries(server ${CMAKE_DL_LIBS} pthread)
if(WIN32)
  target_link_libraries(server wsock32)
endif()
if(UNIX)
  target_link_libraries(server curses)
endif()