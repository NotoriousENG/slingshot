# FILEPATH: /home/michael/Repos/matt/dodge_the_anyas/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project("Slingshot" C)

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_compile_definitions("EMSCRIPTEN")
    set(USE_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2 -s USE_MPG123=1 --preload-file assets --use-preload-plugins -s SDL2_IMAGE_FORMATS=\"['png']\"")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
else()
    find_package(SDL2 REQUIRED)
    find_package(sdl2-mixer REQUIRED)
    find_package(sdl2-image REQUIRED)
    find_package(sdl2-ttf REQUIRED)
endif()

# Find Python3
find_package(Python3 REQUIRED) # required for post build steps

# Print the Python3 executable path
message(STATUS "Python 3 executable: ${Python3_EXECUTABLE}")

# Add library subdirectories
add_subdirectory(lib/clm)

# Add the executable
add_executable(${PROJECT_NAME} "src/main.c")

# Link the required libraries
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_MIXER_LIBRARIES}) # ensure sdl2-mixer "extensions" are installed for .ogg: https://www.reddit.com/r/cataclysmdda/comments/glxgtb/fix_for_sound_problem_when_compiling_in_windows/
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_MIXER_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_TTF_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_TTF_INCLUDE_DIRS})

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${MATH_LIBRARY})
endif()

if (WIN32)
    # /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
    set(SUBSYSTEM_LINKER_OPTIONS "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
endif()

# self published libraries
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/clm/include)
target_link_libraries(${PROJECT_NAME} PRIVATE clm)

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/include")

# Define the Python script path
set(PY_COPY_ASSETS ${CMAKE_CURRENT_LIST_DIR}/scripts/copy_assets.py)
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Running Python script to copy and update HTML..."
    COMMAND ${Python3_EXECUTABLE} ${PY_COPY_ASSETS} ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Copying and updating HTML with Python script"
)

add_dependencies(${PROJECT_NAME} copy_assets)