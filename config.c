/*
 * config.c
 *
 *      Author: aston
 */

#include "config.h"

//---------------------------------------------------------------------------
t_config::t_config()
{
   oscamConf = new s_config;
   oscamConfFile = NULL;
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
void t_config::init_config()
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

   //TODO .....
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
   }
   //TODO .....
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
