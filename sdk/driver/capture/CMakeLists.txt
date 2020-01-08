#9919 144TQFP
if (${CFG_CHIP_FAMILY} STREQUAL "9910")
	set(PLATFORM_NAME "9919")
	add_subdirectory(platform${PLATFORM_NAME})
endif()

add_subdirectory(it${CFG_CHIP_FAMILY})
