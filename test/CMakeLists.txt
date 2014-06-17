CMAKE_MINIMUM_REQUIRED (VERSION 2.6.2)
PROJECT (puppetdbquery_test C CXX)

# Set specfic compile flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-missing-field-initializers")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-local-typedefs")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
endif()

# Library settings
SET (APPLICATION_NAME "puppetdbquery")
SET (APPLICATION_CODENAME "${PROJECT_NAME}")
SET (APPLICATION_COPYRIGHT_YEARS "2013")
SET (APPLICATION_VERSION_MAJOR 0)
SET (APPLICATION_VERSION_MINOR 1)
SET (APPLICATION_VERSION_PATCH 0)
SET (APPLICATION_VERSION_TYPE SNAPSHOT)
SET (APPLICATION_VERSION_STRING "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH}-${APPLICATION_VERSION_TYPE}")
SET (APPLICATION_VENDOR_ID "com.puppetlabs")
SET (APPLICATION_VENDOR_NAME "Puppet Labs")
SET (APPLICATION_VENDOR_URL "puppetlabs.com")
SET (APPLICATION_ID "${APPLICATION_VENDOR_ID}.${PROJECT_NAME}")

# Find libcurl
FIND_PACKAGE (CURL)
if (NOT CURL_FOUND)
    MESSAGE (FATAL_ERROR "libcurl-dev with SSL support is required. Please "
             "install it before using ${APPLICATION_NAME}")
endif ()

# Vendor directory and libraries
SET (VENDOR_DIRECTORY "${PROJECT_SOURCE_DIR}/../vendor")
INCLUDE (${VENDOR_DIRECTORY}/gmock.cmake)

# Project Search Paths
LIST (APPEND CMAKE_PREFIX_PATH "${MAINFOLDER}")
SET (CMAKE_MODULE_PATH "${MAINFOLDER}/../vendor")
INCLUDE_DIRECTORIES("${MAINFOLDER}/../include")

# Test configuration

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
    curl
)

ADD_CUSTOM_TARGET (test
    ${test_BIN}
)
