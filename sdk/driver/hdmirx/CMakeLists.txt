INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
)

if($ENV{CFG_PLATFORM} STREQUAL win32)
    ADD_DEFINITIONS(
        -DHDMITX_EXPORTS
        )
endif($ENV{CFG_PLATFORM} STREQUAL win32)

ADD_LIBRARY(hdmirx
        it6607/csc.c
        it6607/io.c
        it6607/timerprocess.c
        it6607/it6607.c
        it6607/mmp_it6607.c
        mmp_hdmirx.c
    )

