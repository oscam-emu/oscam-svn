/*
 * main.h
 *
 *      Author: aston
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "global.h"
#include "simples.h"
#include "reader.h"
#include "client.h"
#include "config.h"
#include "logger.h"

//--------------------------------------------------------------------
// CLASS --- main ---
//--------------------------------------------------------------------
class t_main : public t_simples
{
private:
	string confingDir;
	int    csDblevel;
	bool terminated;
    bool runINbg;
	static void exitSignalHandler(int);
	void oscamUsage();
public:
	 t_main();
	~t_main();
	void run(int argc, char *argv[]);
	void terminate() { terminated = true; }

	string GetConfingDir() { return confingDir; }
	int    GetCsDblevel()  { return csDblevel; }

	t_reader *reader[MAXREADER];
	t_client *client[MAXCLIENT];
	t_config *config;
	t_logger *logger;
};
//---------------------------------------------------------------------------
extern t_main *mainClass;

#endif /* MAIN_H_ */
