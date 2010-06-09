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

//---------------------------------------------------------------------------
class t_main : public t_simples
{
private:
	string confingDir;
	bool terminated;

	static void exitSignalHandler(int);
public:
	 t_main();
	~t_main();
	void run();
	void terminate() { terminated = true; }
	string GetConfingDir() { return confingDir; }

	t_reader *reader[MAXREADER];
	t_client *client[MAXCLIENT];
	t_config *config;
};
//---------------------------------------------------------------------------
extern t_main *mainClass;

#endif /* MAIN_H_ */
