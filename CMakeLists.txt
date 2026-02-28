cmake_minimum_required(VERSION 3.16)
project(Asteroid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)


FetchContent_MakeAvailable(glfw glm)

add_executable(${PROJECT_NAME} src/main.cpp src/glad.c src/stbi_image.cpp)


target_include_directories(${PROJECT_NAME} PRIVATE include)


target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw
    glm::glm
)


target_compile_definitions(${PROJECT_NAME} PRIVATE 
    ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
)