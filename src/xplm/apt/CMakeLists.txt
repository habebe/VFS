SET(SRC
    Airport.h Airport.cpp
    Log.h Log.cpp
)

add_library(apt ${SRC})
add_executable(apt_test main.cpp)
target_link_libraries(apt_test apt sqlite3)

