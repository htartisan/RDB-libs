cmake_minimum_required(VERSION 3.10)

#
# Build ipcTest-reader app
#

project(ipcTest-reader  LANGUAGES C CXX)

#include(FindPkgConfig)

#include(GNUInstallDirs)

include_directories(
	${CMAKE_SOURCE_DIR}/../..
	${CMAKE_SOURCE_DIR}/../../Src
	${CMAKE_SOURCE_DIR}/../../..	
	${CMAKE_SOURCE_DIR}/../../../spdlog
	${CMAKE_SOURCE_DIR}/../../../spdlog/include
	${CMAKE_SOURCE_DIR}/../../../argspp-lib
	${CMAKE_SOURCE_DIR}/../Common
	${CMAKE_SOURCE_DIR}/..
	${CMAKE_SOURCE_DIR}
)

set(readerSource
	${CMAKE_SOURCE_DIR}/IpcTest-reader.cpp
	${CMAKE_SOURCE_DIR}/../Common/AppUtils.cpp
	${CMAKE_SOURCE_DIR}/../../Src/FileIO/CAudioFileIO.cpp
	${CMAKE_SOURCE_DIR}/../../../argspp-lib/src/args.cpp
)
add_executable(ipcTest-reader  ${readerSource})

target_link_libraries(ipcTest-reader 
	pthread
)

# Install the executable
install(TARGETS ipcTest-reader 
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

