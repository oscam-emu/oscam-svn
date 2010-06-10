/*
 * logger.c
 *
 *      Author: aston
 */

#include "cthread.h"
#include "logger.h"
#include "main.h"

//---------------------------------------------------------------------------
t_logger::t_logger()
{
   logStream = NULL;
   statisticStream = NULL;
}

//---------------------------------------------------------------------------
t_logger::~t_logger()
{
   if (logStream)
	  if (!logStream->logOnDisplay) {
	     logStream->close();
         delete logStream;
      }
   if (statisticStream) {
	  statisticStream->close();
	  delete statisticStream;
   }
}

//---------------------------------------------------------------------------
bool t_logger::isFileExists(string *fileName)
{
   ifstream file(fileName->c_str());
   bool weHaveFile = file.is_open();
   file.close();
   return weHaveFile;
}

//---------------------------------------------------------------------------
int t_logger::checkFileSize(oscamFstream *fileStream)
{
   fileStream->seekg(0, ios::end);
   return fileStream->tellg();
}

//---------------------------------------------------------------------------
void t_logger::execWrite(string *data, oscamFstream *stream)
{
   if (!stream->logOnDisplay && mainClass->config->oscamConf->max_log_size) {
      if (checkFileSize(stream) > mainClass->config->oscamConf->max_log_size * 1024) {
	    *stream << "switch log file" << endl;
  	     stream->close();
	     string newFile = stream->getFileName() + "_prev";
	     rename(stream->getFileName().c_str(), newFile.c_str());
  	     remove(stream->getFileName().c_str());
  	     logStream->open(stream->getFileName().c_str(), ios::in | ios::out | ios::trunc);
  	     if (!logStream->is_open())
  		    throw StandardException("Couldn't open logfile: %s", stream->getFileName().c_str());
      }
   }

   time_t t; time(&t);
   struct tm *lt = localtime(&t);
   string log_buf; log_buf.resize(700);
   sprintf(&log_buf[0], "%4d/%02d/%02d %2d:%02d:%02d %s\n",
			lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday,
			lt->tm_hour, lt->tm_min, lt->tm_sec, data->c_str());

   *logStream << log_buf.c_str();
}

//---------------------------------------------------------------------------
void t_logger::openLogFile()
{
   string *file = &mainClass->config->oscamConf->logfile;
   if (!file->length()) {
	  //TODO
	  return;
   }
   if (file->compare("stdout") == 0) {
	  logStream = (oscamFstream*)&cout;
	 *logStream << ">> OSCam++ <<  cardserver started version " << CS_VERSION <<
	  		       ", build #" << CS_SVN_VERSION << " (" << CS_OSTYPE << ")" << endl;
	  logStream->logOnDisplay = true;
	  return;
   }
   ios_base::openmode mode = ios::in | ios::out;
   mode |= isFileExists(file) ? ios::ate : ios::trunc;
   logStream = new oscamFstream();
   logStream->open(mainClass->config->oscamConf->logfile, mode);
   if (!logStream->is_open())
	  throw StandardException("Couldn't open logfile: %s", file->c_str());
   string line;
   for (int i = 0; i < 80; i++)
	  line += "-";
   time_t sysTime; time(&sysTime);
   // set header
   *logStream << endl << line.c_str() << endl;
   *logStream << ">> OSCam++ <<  cardserver started at " << ctime(&sysTime);
   *logStream << line.c_str() << endl;
   // add some data into the log
   writeLogNormal("version=%s, build #%s, system=%s-%s-%s, nice=%d",
		           CS_VERSION_X, CS_SVN_VERSION, CS_OS_CPU, CS_OS_HW,
                   CS_OS_SYS, mainClass->config->oscamConf->nice);
   writeLogNormal("max. clients=%d, client max. idle=%d sec", MAXCLIENT, mainClass->config->oscamConf->cmaxidle);
   int max_log_size = mainClass->config->oscamConf->max_log_size;
   line = max_log_size ? mainClass->itoa(max_log_size, 10) + " Kb" : "unlimited";
   writeLogNormal("max. logsize=%s", line.c_str());
   writeLogNormal("client timeout=%lu ms, fallback timeout=%lu ms, cache delay=%d ms",
		           mainClass->config->oscamConf->ctimeout,
		           mainClass->config->oscamConf->ftimeout,
		           mainClass->config->oscamConf->delay);
}

//---------------------------------------------------------------------------
void t_logger::writeLogNormal(string msg, ...)
{
   if (!logStream)
	  return;

   mutex.Lock();
   string formated_msg;
   formated_msg.reserve(512);
   va_list params;
   va_start(params, msg);
   vsprintf(&formated_msg[0], msg.c_str(), params);
   va_end(params);
   execWrite(&formated_msg, logStream);
   mutex.Unlock();
}

//---------------------------------------------------------------------------
void t_logger::writeLogDebug(string msg, ...)
{
   if (!logStream)
	  return;
   //TODO
}

//---------------------------------------------------------------------------
void t_logger::writeLogDump(uchar *buf, int n, string msg, ...)
{
   if (!logStream)
	  return;

   if (msg.length())
	  writeLogNormal(msg);

   mutex.Lock();
   string formated_msg;
   formated_msg.reserve(512);
   for (int i = 0; i < n; i += 16 ) {
	  sprintf(&formated_msg[0], "%s", mainClass->cs_hexdump(1, buf + i, (n - i > 16) ? 16 : n - i).c_str());
	  execWrite(&formated_msg, logStream);
   }
   mutex.Unlock();
}
//---------------------------------------------------------------------------
