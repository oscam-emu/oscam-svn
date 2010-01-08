project (csctapi)

file (GLOB csctapi_srcs "*.c")
file (GLOB csctapi_hdrs "*.h")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
add_definitions ("-O2 -Winline -Werror -finline-functions -fomit-frame-pointer -s -m32")
else (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
add_definitions ("-O2 -Winline -Werror -finline-functions -fomit-frame-pointer -s")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

FIND_PATH( USB_INCLUDE usb.h)
if( ${USB_INCLUDE} MATCHES "USB_INCLUDE-NOTFOUND" )
    set (HAVE_LIBUSB "0")
    message (STATUS "libusb usb.h include not found .. no smartreader support")
else ( ${USB_INCLUDE} MATCHES "USB_INCLUDE-NOTFOUND" )
    message (STATUS "libusb usb.h include : ${USB_INCLUDE}")
    FIND_LIBRARY (USB_LIBRARY usb)
    message (STATUS "OS X libusb : ${USB_LIBRARY}")
       target_link_libraries ( ${exe_name} ${USB_LIBRARY})
       include_directories( SYSTEM ${USB_INCLUDE} )
    set (HAVE_LIBUSB "1")
endif ( ${USB_INCLUDE} MATCHES "USB_INCLUDE-NOTFOUND" )
message(STATUS "include dirs : ${INCLUDE_DIRECTORIES}")

set (lib_name "csctapi")

add_library (${lib_name} STATIC ${csctapi_srcs} ${csctapi_hdrs})

