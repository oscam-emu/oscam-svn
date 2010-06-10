/*
 * config.c
 *
 *      Author: aston
 */

#include "config.h"
#include "main.h"

//---------------------------------------------------------------------------
t_config::t_config()
{
   oscamConf = new s_config;
   oscamConfFile = NULL;

#ifdef CS_ANTICASC
   oscamConf->ac_cs = "oscam.ac";
#endif
}

//---------------------------------------------------------------------------
t_config::~t_config()
{
   if (oscamConfFile) delete oscamConfFile;
   delete_s_ip(oscamConf->mon_allowed);
   delete_s_ip(oscamConf->c33_plain);
   delete_s_ip(oscamConf->ncd_allowed);
   delete_s_ip(oscamConf->rad_allowed);
#ifdef WEBIF
   delete_s_ip(oscamConf->http_allowed);
#endif
   if (oscamConf->provid) delete oscamConf->provid;
   if (oscamConf->sidtab) delete oscamConf->sidtab;
#ifdef CS_ANTICASC
   if (oscamConf->cpmap) delete oscamConf->cpmap;
#endif
   delete oscamConf;
}

//---------------------------------------------------------------------------
void t_config::delete_s_ip(s_ip *current)
{
   s_ip *next = current;
   while(next) {
	  current = next;
	  next = current->next;
	  delete current;
   }
}

//---------------------------------------------------------------------------
void t_config::chk_port_tab(char *portasc, PTAB *ptab)
{
	int i, j, nfilts, ifilt, iport;
	char *ptr1, *ptr2, *ptr3;
	char *ptr[CS_MAXPORTS] = {0};
	int  port[CS_MAXPORTS] = {0};
	int previous_nports = ptab->nports;

	for (nfilts = i = previous_nports, ptr1 = strtok(portasc, ";"); (i < CS_MAXCAIDTAB) && (ptr1); ptr1 = strtok(NULL, ";"), i++) {
		ptr[i] = ptr1;
		if( (ptr2 = strchr(mainClass->trim(ptr1), '@')) ) {
			*ptr2++ ='\0';
			ptab->ports[i].s_port = atoi(ptr1);
			ptr[i] = ptr2;
			port[i] = ptab->ports[i].s_port;
			ptab->nports++;
		}
		nfilts++;
	}

	if( nfilts == 1 && strlen(portasc) < 6 && ptab->ports[0].s_port == 0 ) {
		ptab->ports[0].s_port = atoi(portasc);
		ptab->nports = 1;
	}

	iport = ifilt = previous_nports;
	for (i=previous_nports; i<nfilts; i++) {
		if( port[i] != 0 )
			iport = i;
		for (j = 0, ptr3 = strtok(ptr[i], ","); (j < CS_MAXPROV) && (ptr3); ptr3 = strtok(NULL, ","), j++) {
			if( (ptr2 = strchr(mainClass->trim(ptr3), ':')) ) {
				*ptr2++='\0';
				ptab->ports[iport].ftab.nfilts++;
				ifilt = ptab->ports[iport].ftab.nfilts-1;
				ptab->ports[iport].ftab.filts[ifilt].caid = (ushort)mainClass->a2i(ptr3, 4);
				ptab->ports[iport].ftab.filts[ifilt].prids[j] = mainClass->a2i(ptr2, 6);
			} else {
				ptab->ports[iport].ftab.filts[ifilt].prids[j] = mainClass->a2i(ptr3, 6);
			}
			ptab->ports[iport].ftab.filts[ifilt].nprids++;
		}
	}
}

//---------------------------------------------------------------------------
void t_config::chk_iprange(char *value, struct s_ip **base)
{
	int i = 0;
	char *ptr1, *ptr2;
	struct s_ip *lip, *cip;

	for (cip = lip = *base; cip; cip = cip->next)
		lip = cip;

	if (!(cip = new s_ip))
	   throw StandardException("chk_iprange() Error allocating memory!");

	if (*base) lip->next = cip;
	else	  *base = cip;

	memset(cip, 0, sizeof(struct s_ip));
	for (ptr1 = strtok(value, ","); ptr1; ptr1 = strtok(NULL, ",")) {
		if (i == 0)
		   ++i;
		else {
		   if (!(cip = new s_ip))
			  throw StandardException("chk_iprange() Error allocating memory!");
		   lip->next = cip;
		   memset(cip, 0, sizeof(struct s_ip));
		}

		if( (ptr2 = strchr(mainClass->trim(ptr1), '-')) ) {
			*ptr2++ ='\0';
			cip->ip[0] = mainClass->cs_inet_addr(mainClass->trim(ptr1));
			cip->ip[1] = mainClass->cs_inet_addr(mainClass->trim(ptr2));
		} else {
			cip->ip[0]=cip->ip[1] = mainClass->cs_inet_addr(ptr1);
		}
		lip = cip;
	}
}

//---------------------------------------------------------------------------
void t_config::chk_token(string* token, string* value, int tag)
{
   switch(tag) {
       case TAG_GLOBAL   : chk_t_global(token, value); break;
       case TAG_MONITOR  : chk_t_monitor(token, value); break;
	   case TAG_CAMD33   : chk_t_camd33(token, value); break;
	   case TAG_CAMD35   :
	   case TAG_CS357X   : chk_t_camd35(token, value); break;
	   case TAG_NEWCAMD  : chk_t_newcamd(token, value); break;
	   case TAG_RADEGAST : chk_t_radegast(token, value); break;
	   case TAG_SERIAL   : chk_t_serial(token, value); break;
	   case TAG_CS378X   : chk_t_camd35_tcp(token, value); break;
	   case TAG_CCCAM    : chk_t_cccam(token, value); break;
#ifdef CS_WITH_GBOX
		case TAG_GBOX    : chk_t_gbox(token, value); break;
#else
		case TAG_GBOX    : cout << "Warning: OSCam compiled without gbox support!" << endl; break;
#endif
//------------------
#ifdef HAVE_DVBAPI
		case TAG_DVBAPI  : chk_t_dvbapi(token, value); break;
#else
		case TAG_DVBAPI  : cout << "Warning: OSCam compiled without DVB API support!" << endl; break;
#endif
//------------------
#ifdef WEBIF
		case TAG_WEBIF   : chk_t_webif(token, value); break;
#else
		case TAG_WEBIF   : cout << "Warning: OSCam compiled without Webinterface support!" << endl; break;
#endif
//------------------
#ifdef CS_ANTICASC
		case TAG_ANTICASC: chk_t_ac(token, value); break;
#else
		case TAG_ANTICASC: cout << "Warning: OSCam compiled without anticascading support!" << endl; break;
#endif
   }
}

//---------------------------------------------------------------------------
void t_config::chk_t_global(string* token, string* value)
{
	if (token->compare("disablelog") == 0) {
	   oscamConf->disablelog = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("disableuserfile") == 0) {
	   oscamConf->disableuserfile = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("serverip") == 0) {
	   oscamConf->srvip = value->length() ? inet_addr(value->c_str()) : 0;
	   return;
	}

	if (token->compare("logfile") == 0) {
	   oscamConf->logfile = value->length() ? *value : "";
	   return;
	}

	if (token->compare("pidfile") == 0) {
	   oscamConf->pidfile = value->length() ? *value : "";
	   return;
	}

	if (token->compare("usrfile") == 0) {
	   oscamConf->usrfile = value->length() ? *value : "";
	   return;
	}

	if (token->compare("cwlogdir") == 0) {
	   oscamConf->cwlogdir = value->length() ? *value : "";
	   return;
	}

	if (token->compare("usrfileflag") == 0) {
	   oscamConf->usrfileflag = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("clienttimeout") == 0) {
	   oscamConf->ctimeout = value->length() ? atoi(value->c_str()) : CS_CLIENT_TIMEOUT;
	   if (oscamConf->ctimeout < 100) oscamConf->ctimeout *= 1000;
	   return;
	}

	if (token->compare("fallbacktimeout") == 0) {
	   oscamConf->ftimeout = value->length() ? atoi(value->c_str()) : CS_CLIENT_TIMEOUT;
	   if (oscamConf->ftimeout < 100) oscamConf->ftimeout *= 1000;
	   return;
	}

	if (token->compare("clientmaxidle") == 0) {
	   oscamConf->cmaxidle = value->length() ? atoi(value->c_str()) : CS_CLIENT_MAXIDLE;
	   return;
	}

	if (token->compare("cachedelay") == 0) {
	   oscamConf->delay = value->length() ? atoi(value->c_str()) : CS_DELAY;
	   return;
	}

	if (token->compare("bindwait") == 0) {
	   oscamConf->bindwait = value->length() ? atoi(value->c_str()) : CS_BIND_TIMEOUT;
	   return;
	}

	if (token->compare("netprio") == 0) {
	   oscamConf->netprio = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("resolvedelay") == 0) {
	   oscamConf->resolvedelay = value->length() ? atoi(value->c_str()) : CS_RESOLVE_DELAY;
	   return;
	}

	if (token->compare("clientdyndns") == 0) {
	   oscamConf->clientdyndns = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("sleep") == 0) {
	   oscamConf->tosleep = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("unlockparental") == 0) {
	   oscamConf->ulparent = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("nice") == 0) {
	   oscamConf->nice = value->length() ? atoi(value->c_str()) : 99;
	   if ((oscamConf->nice < -20) || (oscamConf->nice > 20))
		   oscamConf->nice = 99;
/*     if (oscamConf->nice != 99) // TODO
		  cs_setpriority(oscamConf->nice);  // ignore errors*/
	   return;
	}

	if (token->compare("unlockparental") == 0) {
	   oscamConf->ulparent = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("serialreadertimeout") == 0) {
	   oscamConf->srtimeout = value->length() ? atoi(value->c_str()) : 1500;
	   if (oscamConf->srtimeout <= 0)
		  oscamConf->srtimeout = 1500;
	   return;
	}

	if (token->compare("maxlogsize") == 0) {
	   oscamConf-> max_log_size = value->length() ? atoi(value->c_str()) : 10;
	   if (oscamConf->max_log_size <= 10)
		  oscamConf->max_log_size = 10;
	   return;
	}

	if (token->compare("waitforcards") == 0) {
	   oscamConf->waitforcards = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	if (token->compare("preferlocalcards") == 0) {
	   oscamConf->preferlocalcards = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	if (token->compare("saveinithistory") == 0) {
	   oscamConf->saveinithistory = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	if (token->compare("readerrestartseconds") == 0) {
	   oscamConf->reader_restart_seconds = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("readerautoloadbalance") == 0) {
	   oscamConf->reader_auto_loadbalance = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	if (token->compare("readerautoloadbalance_save") == 0) {
	   oscamConf->reader_auto_loadbalance_save = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
  	   cerr << "Warning: keyword '" << token->c_str() << "' in global section not recognized!" << endl;
}

//---------------------------------------------------------------------------
void t_config::chk_t_monitor(string* token, string* value)
{
	if (token->compare("port") == 0) {
	   oscamConf->mon_port = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("serverip") == 0) {
	   oscamConf->mon_srvip = value->length() ? inet_addr(value->c_str()) : 0;
	   return;
	}

	if (token->compare("nocrypt") == 0) {
	   if (value->length()) chk_iprange((char*)value->c_str(), &oscamConf->mon_allowed);
	   return;
	}

	if (token->compare("aulow") == 0) {
	   oscamConf->mon_aulow = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("monlevel") == 0) {
	   oscamConf->mon_level = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("hideclient_to") == 0) {
	   oscamConf->mon_hideclient_to = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("appendchaninfo") == 0) {
	   oscamConf->mon_appendchaninfo = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in monitor section not recognized!" << endl;
}

//---------------------------------------------------------------------------
void t_config::chk_t_camd33(string* token, string* value)
{
	if (token->compare("port") == 0) {
	   oscamConf->c33_port = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("serverip") == 0) {
	   oscamConf->c33_srvip = value->length() ? inet_addr(value->c_str()) : 0;
	   return;
	}

	if (token->compare("nocrypt") == 0) {
	   if (value->length()) chk_iprange((char*)value->c_str(), &oscamConf->c33_plain);
	   return;
	}

	if (token->compare("passive") == 0) {
	   oscamConf->c33_passive = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("key") == 0) {
	   if (!value->length()) {
		  oscamConf->c33_crypted = 0;
	      return;
	   }
	   if (mainClass->key_atob((char*)value->c_str(), oscamConf->c33_key))
          throw StandardException("Configuration camd3.3x: Error in Key");
	   oscamConf->c33_crypted = 1;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in camd33 section not recognized!" << endl;
}

//---------------------------------------------------------------------------
void t_config::chk_t_camd35(string* token, string* value)
{
	if (token->compare("port") == 0) {
	   oscamConf->c35_port = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("serverip") == 0) {
	   oscamConf->c35_tcp_srvip = value->length() ? inet_addr(value->c_str()) : 0;
	   return;
	}

	if (token->compare("suppresscmd08") == 0) {
	   oscamConf->c35_suppresscmd08 = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in camd35 section not recognized!" << endl;
}

//---------------------------------------------------------------------------
void t_config::chk_t_newcamd(string *token, string *value)
{
	if (token->compare("key") == 0) {
	   if (value->length()) chk_port_tab((char*)value->c_str(), &oscamConf->ncd_ptab);
	   return;
	}

	if (token->compare("serverip") == 0) {
	   oscamConf->ncd_srvip = value->length() ? inet_addr(value->c_str()) : 0;
	   return;
	}

	if (token->compare("allowed") == 0) {
	   if (value->length()) chk_iprange((char*)value->c_str(), &oscamConf->ncd_allowed);
	   return;
	}

	if (token->compare("key") == 0) {
	   if (value->length())
	     if (mainClass->key_atob14((char*)value->c_str(), oscamConf->ncd_key))
            throw StandardException("Configuration newcamd: Error in Key!");
	   return;
	}

	if (token->compare("keepalive") == 0) {
	   oscamConf->ncd_keepalive = value->length() ? atoi(value->c_str()) : 1;
	   return;
	}

	if (token->compare("mgclient") == 0) {
	   oscamConf->ncd_mgclient = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in newcamd section not recognized!" << endl;
}

//---------------------------------------------------------------------------
void t_config::chk_t_radegast(string *token, string *value)
{
	if (token->compare("port") == 0) {
	   oscamConf->rad_port = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("serverip") == 0) {
	   oscamConf->rad_srvip = value->length() ? inet_addr(value->c_str()) : 0;
	   return;
	}

	if (token->compare("allowed") == 0) {
	   if (value->length()) chk_iprange((char*)value->c_str(), &oscamConf->rad_allowed);
	   return;
	}

	if (token->compare("user") == 0) {
	   oscamConf->rad_usr = *value;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in radegast section not recognized!" << endl;
}

//---------------------------------------------------------------------------
void t_config::chk_t_serial(string *token, string *value)
{
	if (token->compare("device") == 0) {
	   int len = oscamConf->ser_device.length();
	   if (len) oscamConf->ser_device += 1; // use ctrl-a as delimiter
	   oscamConf->ser_device += *value;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in serial section not recognized!" << endl;
}

//---------------------------------------------------------------------------
void t_config::chk_t_camd35_tcp(string *token, string *value)
{
	if (token->compare("port") == 0) {
	   if (value->length()) chk_port_tab((char*)value->c_str(), &oscamConf->c35_tcp_ptab);
	   return;
	}

	if (token->compare("serverip") == 0) {
	   oscamConf->c35_tcp_srvip = value->length() ? inet_addr(value->c_str()) : 0;
	   return;
	}

	if (token->compare("suppresscmd08") == 0) {
	   oscamConf->c35_suppresscmd08 = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in camd35 tcp section not recognized!" << endl;
}

//---------------------------------------------------------------------------
void t_config::chk_t_cccam(string *token, string *value)
{
	if (token->compare("port") == 0) {
	   oscamConf->cc_port = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	if (token->compare("reshare") == 0) {
	   oscamConf->cc_reshare = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	// cccam version
	if (token->compare("version") == 0) {
	   if (value->length() > 7)
		  throw StandardException("cccam config: version too long!");
	   oscamConf->cc_version = *value;
	   return;
	}

	// cccam build number
	if (token->compare("cc_build") == 0) {
	   if (value->length() > 5)
		  throw StandardException("cccam config build number too long!");
	   oscamConf->cc_build = *value;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in cccam section not recognized!" << endl;
}

//---------------------------------------------------------------------------
#ifdef CS_WITH_GBOX
void t_config::chk_t_gbox(string* token, string* value)
{
	if (token->compare("password") == 0) {
	   oscamConf->gbox_pwd.resize(5);
	   mainClass->cs_atob((uchar*)oscamConf->gbox_pwd.c_str(), (char*)value->c_str(), 4); // why pass is just 4 chars long ???
	   return;
	}

	if (token->compare("maxdist") == 0) {
	   oscamConf->gbox_maxdist = atoi(value->c_str());
	   return;
	}

	if (token->compare("ignorelist") == 0) {
	   oscamConf->gbox_ignorefile = *value;
	   return;
	}

	if (token->compare("onlineinfos") == 0) {
	   oscamConf->gbox_gbxShareOnl = *value;
	   return;
	}

	if (token->compare("cardinfos") == 0) {
	   oscamConf->gbox_cardfile = *value;
	   return;
	}

	if (token->compare("locals") == 0) {
	   char *ptr1;
	   int n = 0, i;
	   for (i = 0, ptr1 = strtok((char*)value->c_str(), ","); (i < CS_MAXLOCALS) && (ptr1); ptr1 = strtok(NULL, ",")) {
		   oscamConf->gbox_locals[n++] = mainClass->a2i(ptr1, 8);
		   //printf("%i %08X",n,oscamConf->locals[n-1]);
	   }
	   oscamConf->gbox_num_locals = n;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in gbox section not recognized!" << endl;
}
#endif

//---------------------------------------------------------------------------
#ifdef HAVE_DVBAPI
void t_config::chk_t_dvbapi(string* token, string* value)
{
	if (token->compare("enabled") == 0) {
	   oscamConf->dvbapi_enabled = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	if (token->compare("au") == 0) {
	   oscamConf->dvbapi_au = value->length() ? atoi(value->c_str()) : -1;
	   return;
	}

	if (token->compare("boxtype") == 0) {
	   char *boxdesc[] = { BOXDESC };
	   if (value->length())
	      for (int i = 1; i <= BOXTYPES; i++) {
		     if (value->compare(boxdesc[i]) == 0) {
		        oscamConf->dvbapi_boxtype = i;
		        oscamConf->dvbapi_boxdesc = boxdesc[i];
		        return;
		     }
	      }
	   oscamConf->dvbapi_boxtype = 0;
	   return;
	}

	if (token->compare("user") == 0) {
	   oscamConf->dvbapi_usr = *value;
	   return;
	}

	if (token->compare("priority") == 0) {
	   dvbapi_chk_caidtab((char*)value->c_str(), &oscamConf->dvbapi_prioritytab);
	   return;
	}

	if (token->compare("ignore") == 0) {
	   dvbapi_chk_caidtab((char*)value->c_str(), &oscamConf->dvbapi_ignoretab);
	   return;
	}

	if (token->compare("cw_delay") == 0) {
	   dvbapi_chk_caidtab((char*)value->c_str(), &oscamConf->dvbapi_delaytab);
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in dvbapi section not recognized!" << endl;
}

//---------------------------------------------------------------------------
void t_config::dvbapi_chk_caidtab(char *caidasc, CAIDTAB *ctab)
{
    int i;
	char *ptr1, *ptr3;

	for (i = 0, ptr1 = strtok(caidasc, ","); (i < CS_MAXCAIDTAB) && (ptr1); ptr1 = strtok(NULL, ",")) {
		unsigned long caid, prov;
		if( (ptr3 = strchr(mainClass->trim(ptr1), ':')) )
			*ptr3++ = '\0';
		else
			 ptr3 = "";

		if (((caid = mainClass->a2i(ptr1, 2))|(prov = mainClass->a2i(ptr3, 3))))
		{
			ctab->caid[i] = caid;
			ctab->cmap[i] = prov >> 8;
			ctab->mask[i++] = prov;
		}
	}
}
#endif

//---------------------------------------------------------------------------
#ifdef WEBIF
void t_config::chk_t_webif(string *token, string *value)
{
	if (token->compare("httpport") == 0) {
	   oscamConf->http_port = value->length() ? atoi(value->c_str()) : 0;
	   return;
	}

	if (token->compare("httpuser") == 0) {
	   oscamConf->http_user = *value;
	   return;
	}

	if (token->compare("httppwd") == 0) {
	   oscamConf->http_pwd = *value;
	   return;
	}

	if (token->compare("httpcss") == 0) {
	   oscamConf->http_css = *value;
	   return;
	}

	if (token->compare("httpscript") == 0) {
	   oscamConf->http_script = *value;
	   return;
	}

	if (token->compare("httptpl") == 0) {
	   oscamConf->http_tpl = *value;
	   int len = oscamConf->http_tpl.length();
	   if (len)
		  if (oscamConf->http_tpl[len - 1] != '/')
			 oscamConf->http_tpl[len] = '\0';
	   return;
	}

	if (token->compare("httprefresh") == 0) {
	   oscamConf->http_refresh = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	if (token->compare("httphideidleclients") == 0) {
	   oscamConf->http_hide_idle_clients = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	if (token->compare("httpallowed") == 0) {
	   if (value->length()) chk_iprange((char*)value->c_str(), &oscamConf->http_allowed);
	   return;
	}

	if (token->compare("httpreadonly") == 0) {
	   oscamConf->http_readonly = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	if (token->compare("httpdyndns") == 0) {
	   oscamConf->http_dyndns = *value;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in webif section not recognized!" << endl;
}
#endif

//---------------------------------------------------------------------------
#ifdef CS_ANTICASC
void t_config::chk_t_ac(string *token, string *value)
{
	if (token->compare("enabled") == 0) {
	   oscamConf->ac_enabled = value->length() ? atoi(value->c_str()) : false;
	   return;
	}

	if (token->compare("numusers") == 0) {
	   oscamConf->ac_users = value->length() ? atoi(value->c_str()) : 0;
	   if (oscamConf->ac_users < 0 ) oscamConf->ac_users = 0;
	   return;
	}

	if (token->compare("sampletime") == 0) {
	   oscamConf->ac_stime = value->length() ? atoi(value->c_str()) : 0;
	   if (oscamConf->ac_stime < 0 ) oscamConf->ac_stime = 2;
	   return;
	}

	if (token->compare("samples") == 0) {
	   oscamConf->ac_samples = value->length() ? atoi(value->c_str()) : 0;
	   if (oscamConf->ac_samples < 2 || oscamConf->ac_samples > 10)
		  oscamConf->ac_samples = 10;
	   return;
	}

	if (token->compare("penalty") == 0) {
	   oscamConf->ac_penalty = value->length() ? atoi(value->c_str()) : 0;
	   if (oscamConf->ac_penalty < 0)
		  oscamConf->ac_penalty = 0;
	   return;
	}

	if (token->compare("penalty") == 0) {
	   oscamConf->ac_logfile = *value;
	   return;
	}

	if (token->compare("fakedelay") == 0) {
	   oscamConf->ac_fakedelay = value->length() ? atoi(value->c_str()) : 0;
	   if (oscamConf->ac_fakedelay < 100 || oscamConf->ac_fakedelay > 1000)
		  oscamConf->ac_fakedelay = 1000;
	   return;
	}

	if (token->compare("denysamples") == 0) {
	   oscamConf->ac_denysamples = value->length() ? atoi(value->c_str()) : 0;
	   if (oscamConf->ac_denysamples < 2 || oscamConf->ac_denysamples > oscamConf->ac_samples - 1)
		  oscamConf->ac_denysamples = oscamConf->ac_samples - 1;
	   return;
	}

	char *pointer = (char*)token->c_str();
	if (*pointer != '#')
	   cerr << "Warning: keyword '" << token->c_str() << "' in anticascading section not recognized!" << endl;
}
#endif

//---------------------------------------------------------------------------
void t_config::load_oscamConf()
{
	char *cctag[] = { "global", "monitor", "camd33", "camd35", "newcamd", "radegast", "serial",
			          "cs357x", "cs378x", "gbox", "cccam", "dvbapi", "webif", "anticasc", NULL };

   string fileName = mainClass->GetConfingDir() + CS_CONF;
   oscamConfFile = new fstream();
   oscamConfFile->open(fileName.c_str(), ios::out | ios::in);
   if (!oscamConfFile->is_open())
      throw StandardException("Cant open config file %s", fileName.c_str());

   int tag = TAG_GLOBAL;
   while(!oscamConfFile->eof()) {
	  string line;
	  getline(*oscamConfFile, line);
	  mainClass->trim(&line); mainClass->strToLower(&line);
	  int len = line.length();
	  if (len < 3)
		 continue;
	  if (line[0] == '[' && line[len-1] == ']') {
	     for (int i = 0; cctag[i]; i++) {
		    string token = "[]";
	        token.insert(1, cctag[i]);
	        if (line.compare(token) == 0)
	          { tag = i; break; }
	     }
	     continue;
	  }
	  size_t pos;
	  if ((pos = line.find('=', 0)) == string::npos)
		 continue;
	  string value = line.substr(pos + 1, len - pos);
	  line.erase(pos - 1);
	  mainClass->trim(&value); mainClass->trim(&line);
	  chk_token(&line, &value, tag);
   }
   oscamConfFile->close();

#ifndef CS_LOGFILE
	if (!oscamConf->logfile.length()) {
	   oscamConf->logfile =  CS_LOGFILE;
	}
#endif
	if (oscamConf->ftimeout >= oscamConf->ctimeout) {
		oscamConf->ftimeout = oscamConf->ctimeout - 100;
		fprintf(stderr, "WARNING: fallbacktimeout adjusted to %lu ms (must be smaller than clienttimeout (%lu ms))\n",
				        oscamConf->ftimeout, oscamConf->ctimeout);
	}
	if(oscamConf->ftimeout < oscamConf->srtimeout) {
		oscamConf->ftimeout = oscamConf->srtimeout + 100;
		fprintf(stderr, "WARNING: fallbacktimeout adjusted to %lu ms (must be greater than serialreadertimeout (%lu ms))\n",
				        oscamConf->ftimeout, oscamConf->srtimeout);
	}
	if(oscamConf->ctimeout < oscamConf->srtimeout) {
		oscamConf->ctimeout = oscamConf->srtimeout + 100;
		fprintf(stderr, "WARNING: clienttimeout adjusted to %lu ms (must be greater than serialreadertimeout (%lu ms))\n",
				         oscamConf->ctimeout, oscamConf->srtimeout);
	}
#ifdef CS_ANTICASC
	if( oscamConf->ac_denysamples + 1 > oscamConf->ac_samples ) {
		oscamConf->ac_denysamples = oscamConf->ac_samples - 1;
		fprintf(stderr, "WARNING: DenySamples adjusted to %d\n", oscamConf->ac_denysamples);
	}
#endif
}

//---------------------------------------------------------------------------
void t_config::load_oscamUser()
{

}
//---------------------------------------------------------------------------
void t_config::load_oscamServer()
{

}
//---------------------------------------------------------------------------
void t_config::load_oscamSrvid()
{

}
//---------------------------------------------------------------------------
void t_config::load_oscamGuess()
{

}
//---------------------------------------------------------------------------
void t_config::load_oscamCert()
{

}
//---------------------------------------------------------------------------
void t_config::load_oscamServices()
{

}
//---------------------------------------------------------------------------
