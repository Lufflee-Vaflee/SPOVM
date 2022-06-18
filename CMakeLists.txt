cmake_minimum_required(VERSION 3.16.0)
project(SPOVM_labs VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
set(CMAKE_PROJECT_INCLUDE_BEFORE ON)

set(SOURCES 
    "${PROJECT_SOURCE_DIR}/main.cpp"
    )

#set(HEADERS
#    "${PROJECT_SOURCE_DIR}/something.hpp"
#)

add_executable(${PROJECT_NAME}
    ${SOURCES}
#    ${HEADERS}
)


target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_PROJECT_SOURCE_DIR}
    )
