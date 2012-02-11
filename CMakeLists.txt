CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(beanstalkpp)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

FIND_PACKAGE( Boost 1.48 COMPONENTS system filesystem regex iostreams REQUIRED )
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

INSTALL(TARGETS beanstalkpp DESTINATION lib)
if(APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif(APPLE)
INSTALL(DIRECTORY . DESTINATION include/beanstalk++ FILES_MATCHING PATTERN "*.h" PATTERN "beanstalkpp.h" EXCLUDE PATTERN "*~" EXCLUDE PATTERN "*.git*" EXCLUDE PATTERN "*build*" EXCLUDE)
INSTALL(FILES "beanstalkpp.h" DESTINATION include)

SET(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "-g0 -O2")
