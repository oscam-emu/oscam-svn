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

   runINbg = false;
   csDblevel = 0;
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
void t_main::run(int argc, char *argv[])
{
    int i;
	while ((i = getopt(argc, argv, "bc:d:hm:")) != EOF) {
	   switch(i) {
		   case 'b': runINbg = true;
		             break;
		   case 'c': confingDir = optarg;
		             break;
		   case 'd': csDblevel = atoi(optarg);
		             break;
		   case 'h':
		   default : oscamUsage();
		             return;
	   }
	}

    if (signal(SIGINT, exitSignalHandler) == SIG_ERR)
	   throw StandardException("Error setting up signal SIGINT handler!");
	if (signal(SIGQUIT, exitSignalHandler) == SIG_ERR)
	   throw StandardException("Error setting up signal SIGQUIT handler!");

	config = new t_config();
	config->load_oscamConf();

    try {
	   // start logger here
#ifdef OS_MACOSX
       if (runINbg && daemon_compat(1, 0))
#else
       if (runINbg && daemon(1, 0))
#endif
       {
          throw StandardException("Error starting in background (errno=%d)", errno);
       }
       config->load_oscamUser();
	   config->load_oscamServer();

	   terminated = false;
       while(!terminated) {
          sleep(1);
    	  kill(getpid(), SIGQUIT);
       }
	}
	catch (StandardException& e) {
	   cerr << e.descrptChar() << endl; // this should go into log
	}
}

void t_main::oscamUsage()
{
  char logo[] = "  ___  ____   ___                \n / _ \\/ ___| / __|__ _ _ __ ___  \n| | | \\___ \\| |  / _` | '_ ` _ \\ \n| |_| |___) | |_| (_| | | | | | |\n \\___/|____/ \\___\\__,_|_| |_| |_|\n";
  fprintf(stderr, "%s\n\n", logo);
  fprintf(stderr, "OSCam cardserver v%s, build #%s (%s) - (w) 2010 streamboard SVN\n", CS_VERSION_X, CS_SVN_VERSION, CS_OSTYPE);
  fprintf(stderr, "\tsee http://streamboard.gmc.to:8001/wiki/ for more details\n");
  fprintf(stderr, "\tbased on streamboard mp-cardserver v0.9d - (w) 2004-2007 by dukat\n");
  fprintf(stderr, "\tinbuilt modules: ");
#ifdef HAVE_DVBAPI
#ifdef WITH_STAPI
  fprintf(stderr, "dvbapi with stapi");
#else
  fprintf(stderr, "dvbapi ");
#endif
#endif
#ifdef WEBIF
  fprintf(stderr, "webinterface ");
#endif
#ifdef CS_ANTICASC
  fprintf(stderr, "anticascading ");
#endif
#ifdef LIBUSB
  fprintf(stderr, "smartreader ");
#endif
#ifdef HAVE_PCSC
  fprintf(stderr, "pcsc ");
#endif
#ifdef CS_WITH_GBOX
  fprintf(stderr, "gbox ");
#endif
#ifdef IRDETO_GUESSING
  fprintf(stderr, "irdeto-guessing ");
#endif
#ifdef CS_LED
  fprintf(stderr, "led-trigger ");
#endif
  fprintf(stderr, "\n\n");
  fprintf(stderr, "oscam [-b] [-c config-dir] [-d]");
#ifdef CS_NOSHM
  fprintf(stderr, " [-m memory-file]");
#endif
  fprintf(stderr, " [-h]");
  fprintf(stderr, "\n\n\t-b         : start in background\n");
  fprintf(stderr, "\t-c <dir>   : read configuration from <dir>\n");
  fprintf(stderr, "\t             default = %s\n", CS_CONFDIR);
  fprintf(stderr, "\t-d <level> : debug level mask\n");
  fprintf(stderr, "\t               0 = no debugging (default)\n");
  fprintf(stderr, "\t               1 = detailed error messages\n");
  fprintf(stderr, "\t               2 = ATR parsing info, ECM, EMM and CW dumps\n");
  fprintf(stderr, "\t               4 = traffic from/to the reader\n");
  fprintf(stderr, "\t               8 = traffic from/to the clients\n");
  fprintf(stderr, "\t              16 = traffic to the reader-device on IFD layer\n");
  fprintf(stderr, "\t              32 = traffic to the reader-device on I/O layer\n");
  fprintf(stderr, "\t              64 = EMM logging\n");
  fprintf(stderr, "\t             255 = debug all\n");
#ifdef CS_NOSHM
  fprintf(stderr, "\t-m <file>  : use <file> as mmaped memory file\n");
  fprintf(stderr, "\t             default = %s\n", CS_MMAPFILE);
#endif
  fprintf(stderr, "\t-h         : show this help\n");
  fprintf(stderr, "\n");
}
//---------------------------------------------------------------------------
