if (CFG_CHIP_FAMILY STREQUAL 9910)   #9910 and 9070 use the same isp driver
    add_subdirectory(it9070)
else()
    add_subdirectory(it${CFG_CHIP_FAMILY})
endif()
