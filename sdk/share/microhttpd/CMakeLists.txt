include_directories(
    .
    include
    microhttpd
    )

add_library(microhttpd STATIC
    MHD_config.h
    microhttpd/base64.c
    microhttpd/base64.h
    microhttpd/basicauth.c
    microhttpd/connection.c
    microhttpd/connection.h
    #microhttpd/connection_https.c
    #microhttpd/connection_https.h
    microhttpd/daemon.c
    #microhttpd/digestauth.c
    microhttpd/internal.c
    microhttpd/internal.h
    #microhttpd/md5.c
    #microhttpd/md5.h
    microhttpd/memorypool.c
    microhttpd/memorypool.h
    microhttpd/postprocessor.c
    microhttpd/reason_phrase.c
    microhttpd/reason_phrase.h
    microhttpd/response.c
    microhttpd/response.h
    microhttpd/tsearch.c
    microhttpd/tsearch.h
    include/microhttpd.h
    include/platform.h
    include/platform_interface.h
    )
