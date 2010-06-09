/*
 * main.h
 *
 *      Author: aston
 */



#include "main.h"
#include "config.h"

//---------------------------------------------------------------------------
t_main::t_main()
{
   for(int i = 0; i < MAXREADER; i++)
	 reader[i] = NULL;
   for(int i = 0; i < MAXCLIENT; i++)
	 client[i] = NULL;
}

//---------------------------------------------------------------------------
t_main::~t_main()
{
   for(int i = 0; i < MAXREADER; i++)
	 if (reader[i]) delete reader[i];
   for(int i = 0; i < MAXCLIENT; i++)
	 if (client[i]) delete client[i];
}

//---------------------------------------------------------------------------
//t_main *mainClass = NULL;
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

	confingDir = "E:\\Linux\\Virtual-Machines\\Work\\eclipse\\oscam++\\";
	t_config *config = new t_config();
	config->init_config();
	delete config;

    terminated = false;
    while(!terminated) {
       kill(getpid(), SIGQUIT);
    }
}
//---------------------------------------------------------------------------
