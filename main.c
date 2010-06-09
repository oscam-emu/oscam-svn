/*
 * main.h
 *
 *      Author: aston
 */

#include "main.h"

//---------------------------------------------------------------------------
t_main::t_main()
{
   for(int i = 0; i < MAXREADER; i++)
	 reader[i] = NULL;
   for(int i = 0; i < MAXCLIENT; i++)
	 client[i] = NULL;
   config = NULL;
}

//---------------------------------------------------------------------------
t_main::~t_main()
{
   for(int i = 0; i < MAXREADER; i++)
	 if (reader[i]) delete reader[i];
   for(int i = 0; i < MAXCLIENT; i++)
	 if (client[i]) delete client[i];
   if (config) delete config;
}

//---------------------------------------------------------------------------
/* Signal handler for SIGQUIT and SIGINT. */
void t_main::exitSignalHandler(int)
{
   mainClass->terminate();
}

//---------------------------------------------------------------------------
void t_main::run()
{
	if (signal(SIGINT, exitSignalHandler) == SIG_ERR)
	   throw StandardException("Error setting up signal SIGINT handler!");
	if (signal(SIGQUIT, exitSignalHandler) == SIG_ERR)
	   throw StandardException("Error setting up signal SIGQUIT handler!");

	confingDir = "E:\\Linux\\Virtual-Machines\\Work\\eclipse\\oscam++\\"; // for test

	config = new t_config();
	config->load_oscamConf();
	// start logger here ?!?
	// switch to background
	try {
	   config->load_oscamUser();
	   config->load_oscamServer();

	   terminated = false;
       while(!terminated) {
          kill(getpid(), SIGQUIT);
       }
	}
	catch (StandardException& e) {
	   cerr << e.descrptChar() << endl; // this should go into log
	}
}
//---------------------------------------------------------------------------
