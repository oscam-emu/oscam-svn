#ifndef __CSCRYPT_H__
#  define __CSCRYPT_H__

#  ifdef HAVE_AES
#    include <openssl/aes.h>
#  else
#    include "aes/aes.h"
#  endif

#  include "bn.h"
#  include "crc32.h"
#  include "des.h"
#  include "md5.h"

#  ifdef  __cplusplus
extern "C" {
#  endif

#  if !defined(OS_SOLARIS7) && !defined (OS_AIX42)
#    include <sys/cdefs.h>
#  endif

#  if !defined(__P)
#    define __P(a)	a
#  endif

#  if defined(OS_SOLARIS) || defined (OS_AIX)
#    define u_int32_t unsigned long
#  endif

#  ifdef  __cplusplus
}
#  endif

#endif // __CSCRYPT_H__
