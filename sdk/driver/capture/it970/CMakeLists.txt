include_directories(
    .
    ..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/capture
    ${PROJECT_SOURCE_DIR}/sdk/include/encoder
    ${PROJECT_SOURCE_DIR}/sdk/include
)

add_library(capture STATIC
    mmp_capture.c
    capture_hw.c
    capture_hw.h
    capture_config.h
    capture_reg.h
    capture_util.h
    capture.c
    capture.h
    cap_util.c
    io_video.c
)
