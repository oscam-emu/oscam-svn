/*
 * global.h
 *
 *      Author: aston
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

// TEST
#define HAVE_DVBAPI
#define WEBIF
#define CS_ANTICASC
// END

#define MAXREADER 10
#define MAXCLIENT 10

typedef unsigned char  uchar;
typedef unsigned long  ulong;
typedef unsigned short ushort;

#include <sys/resource.h>
#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fstream>
#include <iostream>

using namespace std;
//---------------------------------------------------------------------------

#define CS_VERSION     "1.00.0svn"
#define CS_SVN_VERSION "1"
#define CS_VERSION_X   "1"
#define CS_OSTYPE      "sh4"
#define CS_OS_SYS      "linux"
#define CS_OS_CPU      "st40"
#define CS_OS_HW       "ipbox"

#ifndef CS_CONFDIR
#define CS_CONFDIR    "/usr/local/etc"
#endif


#ifndef CS_LOGFILE
#define CS_LOGFILE    "/var/log/oscam.log"
#endif

#ifdef HAVE_DVBAPI
#define BOXTYPE_DREAMBOX	1
#define BOXTYPE_DUCKBOX		2
#define BOXTYPE_UFS910		3
#define BOXTYPE_DBOX2		4
#define BOXTYPE_IPBOX		5
#define BOXTYPE_IPBOX_PMT	6
#define BOXTYPES			6
#define BOXDESC  "none", "dreambox", "duckbox", "ufs910", "dbox2", "ipbox", "ipbox-pmt"
#endif

//---------------------------------------------------------------------------
class StandardException
{
private:
    std::string formated_msg;

public:
	StandardException ( std::string msg, ... )
	{
	   formated_msg.reserve(512);

       va_list params;
	   va_start(params, msg);
	   vsprintf(&formated_msg[0], msg.c_str(), params);
	   va_end(params);
	}
   ~StandardException () {};

  std::string descriptStr()
  {
 	 return formated_msg;
  }
  const char *descrptChar()
  {
 	 return formated_msg.c_str();
  }

};
//---------------------------------------------------------------------------
#endif /* GLOBAL_H_ */
