include_directories(
    .
    ..
    ${SMEDIA_INC_DIRS}
    ${PROJECT_SOURCE_DIR}/sdk/include/capture/capture_9850
    ${PROJECT_SOURCE_DIR}/sdk/include/capture
    ${PROJECT_SOURCE_DIR}/sdk/include/isp
    ${PROJECT_SOURCE_DIR}/sdk/include
)

add_library(capture STATIC
    capture_hw.h
    capture_reg.h
    capture_hw.c
    ite_capture.c
)