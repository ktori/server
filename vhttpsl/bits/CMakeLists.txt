add_library(bits)

target_sources(bits PUBLIC
        include/vhttpsl/bits/shutdown.h
        )

target_sources(bits PRIVATE
        src/shutdown.c
        )

target_include_directories(bits PUBLIC include)