project (csctapi)

file (GLOB csctapi_srcs "*.c")
file (GLOB csctapi_hdrs "*.h")

check_include_file ("usb.h" HAVE_LIBUSB)
if (HAVE_LIBUSB)
    add_definitions ("-DHAVE_LIBUSB")
endif (HAVE_LIBUSB)

if (HAVE_LIBUSB)
    target_link_libraries (${exe_name} usb)
endif (HAVE_LIBUSB)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions ("-O2 -Winline -Werror -finline-functions -fomit-frame-pointer -s -m32")
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32" )
    FIND_PATH( USB_INCLUDE usb.h)
    if( ${USB_INCLUDE} MATCHES "USB_INCLUDE-NOTFOUND" )
        set (HAVE_LIBUSB "0")
    else ( ${USB_INCLUDE} MATCHES "USB_INCLUDE-NOTFOUND" )
        FIND_LIBRARY (USB_LIBRARY usb)
        target_link_libraries ( ${exe_name} ${USB_LIBRARY})
        include_directories( SYSTEM ${USB_INCLUDE} )
        set (HAVE_LIBUSB "1")
        add_definitions ("-DHAVE_LIBUSB")
    endif ( ${USB_INCLUDE} MATCHES "USB_INCLUDE-NOTFOUND" )
else (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions ("-O2 -Winline -Werror -finline-functions -fomit-frame-pointer -s")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

set (lib_name "csctapi")
add_library (${lib_name} STATIC ${csctapi_srcs} ${csctapi_hdrs})

