# Set compiler-specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -pthread")
endif ()


SET (SOURCES
    puppetdbquery_test.cpp
)

SET (test_BIN ${PROJECT_NAME})

INCLUDE_DIRECTORIES(
    ../include
    ${GMOCK_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

ADD_EXECUTABLE (${test_BIN} ${SOURCES})

TARGET_LINK_LIBRARIES (
    ${test_BIN}
    ${GMOCK_LIBRARIES}
    # ${CURL_LIBCURL}
    curl
)
