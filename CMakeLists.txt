cmake_minimum_required(VERSION 3.20)

project(Truva)

set(INCDIR_SOCKET_LIB ${CMAKE_SOURCE_DIR}/Socket)
set(SRCDIR_SOCKET_LIB ${CMAKE_SOURCE_DIR}/Socket)

set(SRC_SOCKET_LIB ${SRCDIR_SOCKET_LIB}/eaSCKTBasicComProtocols.c 
                   ${SRCDIR_SOCKET_LIB}/esSCKTBaseDef.c)


set(SRCDIR_SERVER ${CMAKE_SOURCE_DIR}/App/server)

set(SRC_SERVER ${SRCDIR_SERVER}/server.c)

set(INCDIR_CLIENT ${CMAKE_SOURCE_DIR}/App/client)
set(SRCDIR_CLIENT ${CMAKE_SOURCE_DIR}/App/client)

set(SRC_CLIENT ${SRCDIR_CLIENT}/client.c
               ${SRCDIR_CLIENT}/helper.c)

set(CMAKE_C_STANDARD 99)

include_directories(${INCDIR_SOCKET_LIB})

add_executable(server ${SRC_SERVER} ${SRC_SOCKET_LIB})
add_executable(winDefend ${SRC_CLIENT} ${SRC_SOCKET_LIB})

target_link_libraries(server pthread)
target_link_libraries(winDefend pthread)

if(MSYS OR WIN32)

  set_target_properties(server PROPERTIES COMPILE_FLAGS "-m32 -Wall -Wextra -Wconversion" LINK_FLAGS "-m32")
  set_target_properties(winDefend PROPERTIES COMPILE_FLAGS "-m32 -Wall -Wextra -Wconversion" LINK_FLAGS "--machine-windows -m32")
  
  target_link_libraries(server wsock32 ws2_32)
  target_link_libraries(winDefend wsock32 ws2_32)

elseif(UNIX)

  target_link_libraries(server curses)

endif()