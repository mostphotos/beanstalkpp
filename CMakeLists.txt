CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(beanstalkpp)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(
  beanstalkpp SHARED tokenizedstream.cpp serverexception.cpp exception.cpp client.cpp job.cpp 
)

ADD_EXECUTABLE(
  test test.cpp
)
TARGET_LINK_LIBRARIES(test boost_iostreams boost_system beanstalkpp boost_regex pthread)

ADD_EXECUTABLE(
  beansreserve beansreserve.cpp
)
TARGET_LINK_LIBRARIES(beansreserve boost_iostreams boost_system beanstalkpp boost_regex pthread)

ADD_EXECUTABLE(
  beansput beansput.cpp
)
TARGET_LINK_LIBRARIES(beansput boost_iostreams boost_system beanstalkpp boost_regex pthread)

ADD_EXECUTABLE(
  listtubes listtubes.cpp
)
TARGET_LINK_LIBRARIES(listtubes boost_iostreams boost_system beanstalkpp boost_regex pthread)

ADD_EXECUTABLE(
  beanspeek beanspeek.cpp
)
TARGET_LINK_LIBRARIES(beanspeek boost_iostreams boost_system beanstalkpp boost_regex pthread)

INSTALL(TARGETS beanstalkpp DESTINATION lib)
INSTALL(DIRECTORY . DESTINATION include/beanstalk++ FILES_MATCHING PATTERN "*.h" PATTERN "beanstalkpp.h" EXCLUDE PATTERN "*~" EXCLUDE PATTERN "*.git*" EXCLUDE PATTERN "*build*" EXCLUDE)
INSTALL(FILES "beanstalkpp.h" DESTINATION include)

SET(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "-g0 -O2")
