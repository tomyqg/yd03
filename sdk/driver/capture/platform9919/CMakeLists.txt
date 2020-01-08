include_directories(
.
)

add_library(platform9919 STATIC
	capture_interface.c
	sensor/io_sensor_ar0130.c
	sensor/io_sensor_novatek_nt99141.c
	sensor/io_sensor_omnivision_ov7725.c
	sensor/io_sensor_pixelplus_po3100.c
	sensor/io_adv7180.c
	sensor/io_cat9883.c
	sensor/io_hdmirx.c
	sensor/io_externalhdmirx.c
)