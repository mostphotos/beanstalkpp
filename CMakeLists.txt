CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

##############################--g++4.7-plus--###################################
if(EXISTS "gentoo_rules.cmake")
  find_program(CXX NAMES g++-4.8.1 g++-4.7.2 g++-4.7.1 g++-4.7 g++)
  execute_process(COMMAND sed -i 
	"s|CMAKE_CXX_COMPILER [^ ]*|CMAKE_CXX_COMPILER ${CXX}|g" 
	"gentoo_rules.cmake")
  find_program(CC NAMES gcc-4.8.1 gcc-4.7.2 gcc-4.7.1 gcc-4.7 gcc)
  execute_process(COMMAND sed -i 
	"s|CMAKE_C_COMPILER [^ ]*|CMAKE_C_COMPILER ${CC}|g"
	"gentoo_rules.cmake")
else()
  find_program(CMAKE_CXX_COMPILER NAMES g++-4.8.1 g++-4.7.2 g++-4.7.1 g++-4.7 g++)
  find_program(CMAKE_C_COMPILER NAMES gcc-4.8.1 gcc-4.7.2 gcc-4.7.1 gcc-4.7 gcc)
endif()
################################################################################
PROJECT(beanstalkpp)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

FIND_PACKAGE( Boost COMPONENTS system filesystem regex iostreams REQUIRED )
link_directories(${Boost_LIBRARY_DIR})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

ADD_LIBRARY(
  beanstalkpp SHARED tokenizedstream.cpp serverexception.cpp exception.cpp client.cpp job.cpp 
)

ADD_EXECUTABLE(
  test test.cpp
)
TARGET_LINK_LIBRARIES(test ${Boost_LIBRARIES} beanstalkpp pthread)

ADD_EXECUTABLE(
  beansreserve beansreserve.cpp
)
TARGET_LINK_LIBRARIES(beansreserve ${Boost_LIBRARIES} beanstalkpp pthread)

ADD_EXECUTABLE(
  beansput beansput.cpp
)
TARGET_LINK_LIBRARIES(beansput ${Boost_LIBRARIES} beanstalkpp pthread)

ADD_EXECUTABLE(
  listtubes listtubes.cpp
)
TARGET_LINK_LIBRARIES(listtubes ${Boost_LIBRARIES} beanstalkpp pthread)

ADD_EXECUTABLE(
  beanspeek beanspeek.cpp
)
TARGET_LINK_LIBRARIES(beanspeek ${Boost_LIBRARIES} beanstalkpp pthread)

INSTALL(TARGETS beanstalkpp LIBRARY DESTINATION lib)
if(APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif(APPLE)
INSTALL(DIRECTORY . DESTINATION include/beanstalk++ FILES_MATCHING PATTERN "*.h" PATTERN "beanstalkpp.h" EXCLUDE PATTERN "*~" EXCLUDE PATTERN "*.git*" EXCLUDE PATTERN "*build*" EXCLUDE)
INSTALL(FILES "beanstalkpp.h" DESTINATION include)

SET(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -g -O0 -std=c++11")
SET(CMAKE_CXX_FLAGS_RELEASE "-g0 -O2")
SET(CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++")
SET(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
