cmake_minimum_required(VERSION 3.16)

project(CG_VR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)

# Shared include directories
include_directories(
    external/glad/include
    common
)

# Find every folder inside labs/ and reconfigure when labs are added/removed.
file(GLOB LABS CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR} "labs/*")

foreach(LAB ${LABS})

    if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/${LAB}")

        # Executable name = folder name
        get_filename_component(LAB_NAME ${LAB} NAME)

        # Collect ALL .cpp files in the lab folder
        file(GLOB LAB_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_SOURCE_DIR}/${LAB}/*.cpp"
        )

        # Skip folders with no cpp files
        if(NOT LAB_SOURCES)
            continue()
        endif()

        add_executable(${LAB_NAME}
            ${LAB_SOURCES}
            external/glad/src/gl.c
        )

        target_include_directories(${LAB_NAME}
            PRIVATE
                external/glad/include
                ${CMAKE_SOURCE_DIR}/${LAB}
        )

        target_link_libraries(${LAB_NAME}
            glfw
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
        )

    endif()

endforeach()
