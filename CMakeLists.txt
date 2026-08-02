cmake_minimum_required(VERSION 3.20)
project(Bucket VERSION 0.1.0 LANGUAGES C CXX)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.28)
  cmake_policy(SET CMP0169 NEW)
endif()

# --- Clang enforce ---
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  message(FATAL_ERROR "MSVC is not allowed. Use Clang.")
elseif(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(WARNING "Compiler is ${CMAKE_CXX_COMPILER_ID}, expected Clang")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Dependencies ---
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchGLAD)
include(FetchGLFW)
include(FetchGLM)
include(FetchSTB)
include(FetchImGui)
include(FetchImGuizmo)

# --- Engine (static library) ---
file(GLOB_RECURSE ENGINE_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.cpp"
)

add_library(bucket_engine STATIC ${ENGINE_SOURCES})
target_include_directories(bucket_engine PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/engine"
  "${CMAKE_CURRENT_SOURCE_DIR}/engine/core"
  "${CMAKE_CURRENT_SOURCE_DIR}/engine/renderer"
  "${CMAKE_CURRENT_SOURCE_DIR}/engine/resource"
  "${CMAKE_CURRENT_SOURCE_DIR}/engine/platform"
)
target_link_libraries(bucket_engine PUBLIC
  glad
  glfw
  glm::glm
  stb_image
)

target_compile_options(bucket_engine PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,Clang>:
    -Wall -Wextra -Wpedantic -Wno-c++98-compat-pedantic
  >
)

# --- Copy shaders to build directory ---
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders" DESTINATION "${CMAKE_BINARY_DIR}")

# --- Game executable ---
file(GLOB_RECURSE GAME_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/game/*.cpp"
)

string(RANDOM LENGTH 4 ALPHABET 0123456789 GAME_NAME_SUFFIX)

add_executable(game ${GAME_SOURCES})
target_link_libraries(game PRIVATE bucket_engine)
target_include_directories(game PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/game")

set_target_properties(game PROPERTIES
  OUTPUT_NAME "game${GAME_NAME_SUFFIX}"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
message(STATUS "Game executable: game${GAME_NAME_SUFFIX}.exe")

# Remove previously built game*.exe files (old random names) after linking.
add_custom_command(TARGET game POST_BUILD
  COMMAND ${CMAKE_COMMAND} -P
          "${CMAKE_CURRENT_SOURCE_DIR}/cmake/clean_game_exe.cmake"
          "${CMAKE_BINARY_DIR}"
          "$<TARGET_FILE:game>"
)

# --- Editor executable ---
file(GLOB_RECURSE EDITOR_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/editor/*.cpp"
)

if(EDITOR_SOURCES)
  add_executable(editor ${EDITOR_SOURCES})
  target_link_libraries(editor PRIVATE bucket_engine imgui imguizmo)
  target_include_directories(editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/editor")

  set_target_properties(editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  )
endif()
