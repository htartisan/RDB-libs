cmake_minimum_required(VERSION 3.10)

set(TARGET_NAME IPC-lib)
set(TARGET_DESC "IPC comm module")

project(${TARGET_NAME}
        VERSION 1.0.0
        DESCRIPTION ${TARGET_DESC}
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")   
        add_definitions(-DDEBUG) 
endif()

#
# Build ipcTest-writer app
#

include(IpcTest-writer.cmake)

#
# Build ipcTest-reader app
#

include(IpcTest-reader.cmake)

#
# Build ipcTest-writer2 app
#

include(IpcTest-writer2.cmake)

#
# Build ipcTest-reader2 app
#

include(IpcTest-reader2.cmake)
