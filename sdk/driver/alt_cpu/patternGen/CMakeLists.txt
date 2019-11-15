if (CFG_CHIP_FAMILY EQUAL 9850)
    file(COPY
        ${PROJECT_SOURCE_DIR}/build/$ENV{CFG_BUILDPLATFORM}/alt_cpu_it9850/lib/sm32/patternGen.hex
        DESTINATION ${PROJECT_SOURCE_DIR}/sdk/driver/alt_cpu/patternGen/.
    )
endif()

if (DEFINED CFG_PATTERN_GEN AND NOT $ENV{CFG_PLATFORM} STREQUAL win32)
    add_library(patternGen STATIC
        patternGenDevice.c
    )
endif()