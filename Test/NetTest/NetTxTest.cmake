
message(STATUS "Building NetTxTest application ")

if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

if (POLICY CMP0177)
  cmake_policy(SET CMP0177 NEW)
endif()

#message(STATUS "CMAKE CURRENT SOURCE DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(NetTxTest
              ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/NetIO/CNetworkIO.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/../../Src/NetIO/CClientIO.cpp
              #${CMAKE_CURRENT_SOURCE_DIR}/../../Src/FileIO/CAudioFileIO.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/AppConfigMgr.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/AppUtils.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/NetTxTest.cpp
              )


target_include_directories(NetTxTest PUBLIC
                          ${CMAKE_CURRENT_SOURCE_DIR}
                          ${PROJECT_SOURCE_DIR}/../..
                          ${INCLUDE_DIRECTORIES}
                          )

target_include_directories(NetTxTest PRIVATE
                          "Include"
                          )

target_link_libraries(NetTxTest PRIVATE
                      Common::Common
                      libconfig::libconfig
                      ${PLATFORM_LINK_LIBRARIES}
                      )

install(TARGETS NetTxTest DESTINATION ${CMAKE_INSTALL_APPDIR})
