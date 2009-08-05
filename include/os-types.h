#ifndef __OS_TYPES_H__
#  define __OS_TYPES_H__

#  if !defined(OS_AIX)
typedef unsigned char uchar;
#  endif

//typedef unsigned short ushort;

#  if defined(OS_CYGWIN32) || defined(OS_HPUX) || defined(OS_FREEBSD)  || defined(OS_MACOSX)
typedef unsigned long ulong;
#  endif

typedef unsigned long long ullong;

#  ifndef NO_ENDIAN_H
#    ifdef OS_MACOSX
#      include <machine/endian.h>
#    else
#      include <endian.h>
#      include <byteswap.h>
#    endif
#  endif

#  if defined(CS_EMBEDDED) || defined(OS_LINUX)

#    ifdef ntohl
#      undef ntohl
#    endif
#    ifdef ntohs
#      undef ntohs
#    endif
#    ifdef htonl
#      undef htonl
#    endif
#    ifdef htons
#      undef htons
#    endif

#    if __BYTE_ORDER == __BIG_ENDIAN
#      define ntohl(x)	(x)
#      define ntohs(x)	(x)
#      define htonl(x)	(x)
#      define htons(x)	(x)
#    else
#      if __BYTE_ORDER == __LITTLE_ENDIAN
#        define ntohl(x)	__bswap_32 (x)
#        define ntohs(x)	__bswap_16 (x)
#        define htonl(x)	__bswap_32 (x)
#        define htons(x)	__bswap_16 (x)
#      endif
#    endif

#  endif // CS_EMBEDDED || OS_LINUX

#define CS_LOGHISTORY
#define NO_PAR_SWITCH

#ifdef OS_FREEBSD
#  define NO_ENDIAN_H
#  define NO_FTIME
#endif

#ifdef TUXBOX
#  ifdef MIPSEL
#    define CS_LOGFILE "/dev/null"
#  else
#    define CS_LOGFILE "/dev/tty"
#  endif
#  define CS_EMBEDDED
#  define CS_NOSHM
#  define NO_FTIME
#  define CS_HW_DBOX2	1
#  define CS_HW_DREAM	2
#  define SCI_DEV 1
#  ifndef NO_PAR_SWITCH
#    define NO_PAR_SWITCH
#  endif
#endif

#ifdef UCLIBC
#  define CS_EMBEDDED
#  define CS_NOSHM
#  define NO_FTIME
#  ifndef NO_PAR_SWITCH
#    define NO_PAR_SWITCH
#  endif
#endif

#ifdef OS_CYGWIN32
#  define CS_NOSHM
#  define CS_MMAPFILE "oscam.mem"
#  define CS_LOGFILE "/dev/tty"
#  define NO_ENDIAN_H
#  ifndef NO_PAR_SWITCH
#    define NO_PAR_SWITCH
#  endif
#endif

#ifdef OS_SOLARIS
#  define NO_ENDIAN_H
#  define NEED_DAEMON
#endif

#ifdef OS_OSF
#  define NO_ENDIAN_H
#  define NEED_DAEMON
#endif

#ifdef OS_AIX
#  define NO_ENDIAN_H
#  define NEED_DAEMON
#  define socklen_t unsigned long
#endif

#ifdef OS_IRIX
#  define NO_ENDIAN_H
#  define NEED_DAEMON
#  define socklen_t unsigned long
#endif

#ifdef OS_HPUX
#  define NO_ENDIAN_H
#  define NEED_DAEMON
#endif

#ifdef ARM
#  define CS_EMBEDDED
#  define CS_NOSHM
#  define NO_FTIME
#endif

//#ifdef ALIGNMENT
//#  define STRUCTS_PACKED
//#endif

#endif // __OS_TYPES_H__
