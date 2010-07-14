/*
 * logger.h
 *
 *      Author: aston
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "global.h"

//--------------------------------------------------------------------
// CLASS --- moded fstream --- i need file name
//--------------------------------------------------------------------
class oscamFstream : public fstream
{
private:
	string fileName;
public:
	 oscamFstream() { logOnDisplay = false; };
    ~oscamFstream() { };
    string getFileName() { return fileName; }
	void open(string FileName)
	   { fileName = FileName; fstream::open(FileName.c_str()); }
	void open(string FileName, ios_base::openmode mode)
	   { fileName = FileName; fstream::open(FileName.c_str(), mode); }
	bool logOnDisplay;
};

//--------------------------------------------------------------------
// CLASS --- logger ---
//--------------------------------------------------------------------
class t_logger
{
private:
	oscamFstream *statisticStream;
	oscamFstream *logStream;

	bool isFileExists(string *fileName);
	int  checkFileSize(oscamFstream *fileStream);
	void execWrite(string *data, oscamFstream *stream);

	cMutex mutex;
public:
	 t_logger();
	~t_logger();
	void openLogFile();
	void writeLogNormal(string msg, ...);
	void writeLogDebug(string msg, ...);
	void writeLogDump(uchar *buf, int n, string msg, ...);
};
//---------------------------------------------------------------------------

#endif /* LOGGER_H_ */
