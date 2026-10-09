cmake_minimum_required(VERSION 3.16)

project(cod4x_geo_welcome_plugin
    VERSION 2.0.0
    DESCRIPTION "Geolocation based welcome plugin for CoD4X"
    LANGUAGES C
)

# ---------------------------------------------------------------------------
# Force 32-bit build for COD4X plugin
# ---------------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR i386)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -Wall -O1 -fvisibility=hidden -mtune=core2")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32 -shared")

# ---------------------------------------------------------------------------
# Force 32-bit libcurl (CMake otherwise picks the 64-bit version)
# ---------------------------------------------------------------------------
set(CURL_INCLUDE_DIR "/usr/include")
set(CURL_LIBRARY "/usr/lib/i386-linux-gnu/libcurl.so")
find_package(CURL REQUIRED)

# ---------------------------------------------------------------------------
# Embed cJSON (Debian Trixie ships no libcjson.pc for i386)
# ---------------------------------------------------------------------------
add_library(CJSON STATIC libs/cjson/cJSON.c)
target_include_directories(CJSON PUBLIC libs/cjson)

# ---------------------------------------------------------------------------
# Threads (pthread)
# ---------------------------------------------------------------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------
# Build plugin module (dlopen()ed by COD4X)
# ---------------------------------------------------------------------------
set(PLUGIN_NAME geoip)

add_library(${PLUGIN_NAME} MODULE
    src/main.c
    src/thread.c
    src/curl.c
    src/cjson.c
    src/log.c
    src/structs.c
)

# Include paths for plugin headers + COD4X SDK + cJSON
target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/cjson
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    CURL::libcurl
    CJSON
    Threads::Threads
)

# COD4X plugin requirements
set_target_properties(${PLUGIN_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    PREFIX ""    # geoip.so, not libgeoip.so
)

target_compile_options(${PLUGIN_NAME} PRIVATE
    -Wall
    -Wno-attributes
)

# Optional verbose logging
option(GEOIP_VERBOSE "Compile the DBG() tracing calls in" OFF)
if(GEOIP_VERBOSE)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE VERBOSE)
endif()

# Output directory
set_target_properties(${PLUGIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Install into COD4X plugin directory
install(TARGETS ${PLUGIN_NAME}
    LIBRARY DESTINATION plugins
)
