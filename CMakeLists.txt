cmake_minimum_required(VERSION 3.20)

project(glDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(glDemo
    src/main/Main.cpp
    src/renderer/BlackHoleRenderer.cpp
    src/camera/Camera.cpp
    src/read/read.cpp
    src/image/Image.cpp
    src/shader/Shader.cpp
    src/renderer/SrceenSpaceReflection.cpp
    src/pawn/Player.cpp
    src/pawn/Collision.cpp
)

target_include_directories(glDemo PRIVATE
    src/main
    src/renderer
    src/camera
    src/read
    src/image
    src/shader
    src/pawn
    third_party/stb
)

target_compile_options(glDemo PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

target_link_libraries(glDemo PRIVATE
    OpenGL::GL
    glfw
    glad::glad
    glm::glm
    assimp::assimp
)

add_custom_command(TARGET glDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets/shader
            $<TARGET_FILE_DIR:glDemo>/shader
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets/res
            $<TARGET_FILE_DIR:glDemo>/res
)

set_property(TARGET glDemo PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:glDemo>"
)
