
cmake_minimum_required(VERSION 2.8)

project(weechat-plugin C)

include(FindPkgConfig)
pkg_check_modules(WEECHAT REQUIRED "weechat >= 0.3.3")

add_library(notion SHARED notion.c)
target_link_libraries(notion ${WEECHAT_LIBRARIES})
include_directories(${WEECHAT_INCLUDE_DIRS})

install(TARGETS notion LIBRARY DESTINATION ${WEECHAT_LIBDIR}/weechat/plugins)

