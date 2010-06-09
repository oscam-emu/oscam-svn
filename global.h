/*
 * global.h
 *
 *      Author: aston
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#define MAXREADER 10
#define MAXCLIENT 10

typedef unsigned char  uchar;
typedef unsigned long  ulong;
typedef unsigned short ushort;

#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#include <syslog.h>
#include <stdlib.h>
#include <string>
#include <typeinfo>
#include <stdarg.h>
#include <iostream>
#include <errno.h>


using namespace std;


class StandardException
{
private:
    std::string formated_msg;

public:
	StandardException ( std::string msg, ... )
	{
	   formated_msg.resize(512);

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

#endif /* GLOBAL_H_ */
