cmake_minimum_required(VERSION 3.16)

project(cod4x_geo_welcome_plugin
    VERSION 2.0.0
    DESCRIPTION "Geolocation based welcome plugin for CoD4X"
    LANGUAGES C
)

# The engine derives a plugin's short name from its filename without extension
# (see libs/plugin_declarations.h), and the config file is read as "geoip.cfg",
# so the module is named to match.
set(PLUGIN_NAME geoip)

# Enable the DBG() tracing in include/log.h
option(GEOIP_VERBOSE "Compile the DBG() tracing calls in" OFF)

find_package(CURL REQUIRED)

# Neither PCRE2 nor cJSON ships a CMake config on every distro, and this
# toolchain has no PCRE2Config.cmake, so resolve both through pkg-config.
# libpcre2-8.pc is the 8-bit code unit build, matching the PCRE2_CODE_UNIT_WIDTH
# that src/cjson.c defines before including pcre2.h.
find_package(PkgConfig REQUIRED)
pkg_check_modules(PCRE2 REQUIRED IMPORTED_TARGET libpcre2-8)
pkg_check_modules(CJSON REQUIRED IMPORTED_TARGET libcjson)

# MODULE, not SHARED: this is dlopen()ed by the server, so it must not carry a
# SONAME and is always position independent.
#
# No --no-undefined: Plugin_Printf, Plugin_ChatPrintf, Plugin_GetPlayerName and
# friends are declared but not defined here. They are resolved at load time from
# the server executable, so undefined symbols are expected and required.
add_library(${PLUGIN_NAME} MODULE
    src/main.c
    src/curl.c
    src/cjson.c
    src/log.c
    src/structs.c
)

# No target_include_directories() on purpose: every source already reaches its
# own headers with "../include/..." and "../libs/..." relative paths. Adding
# include/ to the search path would let include/curl.h shadow curl/curl.h.
target_link_libraries(${PLUGIN_NAME} PRIVATE
    CURL::libcurl
    PkgConfig::PCRE2
    PkgConfig::CJSON
)

set_target_properties(${PLUGIN_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    # Matches PCL_LOCAL in libs/pinc.h. Everything without an explicit PCL
    # marker stays internal, so only the On* entry points are exported.
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    PREFIX ""    # geoip.so, not libgeoip.so
)

target_compile_options(${PLUGIN_NAME} PRIVATE
    -Wall
    # The SDK macros PCL and __iceimport put __attribute__((cdecl)) on every
    # declaration. That attribute is a no-op on x86-64 SysV, so GCC reports
    # "'cdecl' attribute ignored" ~150 times per translation unit. Marking the
    # SDK directory SYSTEM does not help, because src/ includes it through the
    # relative path "../libs/pinc.h" rather than through the include path.
    -Wno-attributes
)

if(GEOIP_VERBOSE)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE VERBOSE)
endif()

# Where the built module lands. Configure with -B build, so this is build/.
set_target_properties(${PLUGIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Deploy into the server's plugin directory:
#   cmake --install build --prefix /path/to/cod4x
# which writes <prefix>/plugins/geoip.so
install(TARGETS ${PLUGIN_NAME}
    LIBRARY DESTINATION plugins
)
