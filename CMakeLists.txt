cmake_minimum_required(VERSION 3.16)
project(Asteroid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

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

FetchContent_Declare(
  freetype
  GIT_REPOSITORY https://github.com/freetype/freetype.git
  GIT_TAG        VER-2-13-2
 )

set(FT_DISABLE_ZLIB  ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG   ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw glm freetype)


add_executable(${PROJECT_NAME} src/main.cpp src/glad.c src/stbi_image.cpp src/Interface.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include)


target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw
    glm::glm
    freetype
)
target_include_directories(${PROJECT_NAME} PRIVATE ${freetype_SOURCE_DIR}/include)


target_compile_definitions(${PROJECT_NAME} PRIVATE 
    ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
    SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
    FONTS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/fonts/"
)