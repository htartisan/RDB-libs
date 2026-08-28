cmake_minimum_required(VERSION 3.10)

#
# Build ipcTest-writer2 app
#

project(ipcTest-writer  LANGUAGES C CXX)

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

set(writerSource
	${CMAKE_SOURCE_DIR}/IpcTest-writer2.cpp
	${CMAKE_SOURCE_DIR}/../Common/AppUtils.cpp
	${CMAKE_SOURCE_DIR}/../../Src/FileIO/CAudioFileIO.cpp
	${CMAKE_SOURCE_DIR}/../../../argspp-lib/src/args.cpp
)
add_executable(ipcTest-writer2  ${writerSource})

target_link_libraries(ipcTest-writer2 
	pthread 
)

# Install the executable
install(TARGETS ipcTest-writer2 
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

