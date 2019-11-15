if (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(
        -DDEBUG
    )
endif()

INCLUDE_DIRECTORIES(
    .
    ..
    ${PROJECT_SOURCE_DIR}/sdk/include/
    ${PROJECT_SOURCE_DIR}/sdk/include/isp
)

if (CFG_CHIP_FAMILY STREQUAL 9070 OR CFG_CHIP_FAMILY STREQUAL 9910)
    ADD_LIBRARY(isp STATIC
        it9070/isp.c
        it9070/isp_hw.c
        it9070/isp_hw_op.c
        it9070/isp_queue.c
        it9070/isp_util.c
        it9070/mmp_isp.c

        # inlcude file
        it9070/isp.h
        it9070/isp_config.h
        it9070/isp_defs.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/isp_error.h
        it9070/isp_hw.h
        it9070/isp_hw_op.h
        it9070/isp_queue.h
        it9070/isp_queue_type.h
        it9070/isp_reg.h
        it9070/isp_types.h
        it9070/isp_util.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/mmp_isp.h
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 9850)
    ADD_LIBRARY(isp STATIC
        it9850/isp.c
        it9850/isp_hw.c
        it9850/isp_util.c
        it9850/mmp_isp.c

        # inlcude file
        it9070/isp.h
        it9070/isp_config.h
        it9070/isp_defs.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/isp_error.h
        it9070/isp_hw.h
        it9070/isp_reg.h
        it9070/isp_types.h
        it9070/isp_util.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/mmp_isp.h
    )
endif()

if (CFG_CHIP_FAMILY STREQUAL 970)
    ADD_LIBRARY(isp STATIC
        it970/isp.c
        it970/isp_hw.c
        it970/isp_hw_op.c
        it970/isp_queue.c
        it970/isp_util.c
        it970/mmp_isp.c

        # inlcude file
        it970/isp.h
        it970/isp_config.h
        it970/isp_defs.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/isp_error.h
        it970/isp_hw.h
        it970/isp_hw_op.h
        it970/isp_queue.h
        it970/isp_queue_type.h
        it970/isp_reg.h
        it970/isp_types.h
        it970/isp_util.h
        ${PROJECT_SOURCE_DIR}/sdk/include/isp/mmp_isp_970.h
    )
endif()