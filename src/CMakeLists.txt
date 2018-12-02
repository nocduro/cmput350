# All the source files for the bot.
file(GLOB BOT_SOURCES "*.cpp" "*.h" "*.hpp")

# Enable compilation of the SC2 version of the bot.
add_definitions(-DSC2API)

include_directories(SYSTEM "${SC2Api_INCLUDE_DIRS}")

# Show more warnings at compiletime.
if (MSVC)
    # FIXME: put Windows specific options here.
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif ()

# Create the executable.
add_executable(cmput350 ${BOT_SOURCES})
target_link_libraries(cmput350 ${SC2Api_LIBRARIES})

if (APPLE)
    target_link_libraries(cmput350 "-framework Carbon")
endif ()

# Linux specific.
if (UNIX AND NOT APPLE)
    target_link_libraries(cmput350 pthread dl)
endif ()
