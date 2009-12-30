//
// OSCam HTTP server module
//
#include "oscam-http-helpers.c"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

static char* monlevel[] = {
			"0 = no access to monitor",
			"1 = only server and own procs",
			"2 = all procs, but viewing only, default",
			"3 = all procs, reload of oscam.user possible",
			"4 = complete access"};

static char* uniq[] = {
			"0 = none",
			"1 = strict",
			"2 = per IP"};


void refresh_oscam(enum refreshtypes refreshtype){
int i;
	switch (refreshtype){
		case REFR_ACCOUNTS:
				cs_log("Refresh Accounts requested by WebIF");
			  init_userdb();
				cs_reinit_clients();
#ifdef CS_ANTICASC
				for (i=0; i<CS_MAXPID; i++)
					if (client[i].typ=='a') {
						kill(client[i].pid, SIGHUP);
						break;
					}
#endif
			break;

		case REFR_READERS:
			cs_log("Refresh Reader");
			//todo how I can refresh the readers
			break;

		case REFR_SERVER:
			cs_log("Refresh Server requested by WebIF");
			//kill(client[0].pid, SIGHUP);
			//todo how I can refresh the server after global settings
			break;

#ifdef CS_ANTICASC
		case REFR_ANTICASC:
			cs_log("Refresh Anticascading requested by WebIF");
			for (i=0; i<CS_MAXPID; i++)
				if (client[i].typ=='a') {
					kill(client[i].pid, SIGHUP);
					break;
				}
			break;
#endif
	}
}

void send_oscam_config_global(FILE *f, struct uriparams *params) {
	int i;

	fprintf(f,"<BR><BR>");
	
	if (strcmp(getParam(params, "action"), "execute") == 0){
		//we found the execute flag
		for(i=0;i<(*params).paramcount;i++){
			if ((strcmp((*params).params[i], "part")) && (strcmp((*params).params[i], "action"))){
				fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", (*params).params[i], (*params).values[i]);
				//we use the same function as used for parsing the config tokens
				chk_t_global((*params).params[i], (*params).values[i]);
			}
		}

		//Disclaimer
		fprintf(f,"<BR><BR><B>Configuration Global *DONE*</B><BR><BR>");
		refresh_oscam(REFR_SERVER);
	} else {

		//if nothing above matches we show the form
		fprintf(f,"<form action=\"/config.html\" method=\"get\">\r\n");
		fprintf(f,"<input name=\"part\" type=\"hidden\" value=\"global\">\r\n");
		fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"execute\">\r\n");
		fprintf(f,"<TABLE cellspacing=\"0\">");
		fprintf(f,"\t<TH>&nbsp;</TH><TH>Edit Global Config </TH>");
	
		//ServerIP
		fprintf(f,"\t<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", inet_ntoa(*(struct in_addr *)&cfg->srvip));
		//Logfile
		fprintf(f,"\t<TR><TD>Logfile:</TD><TD><input name=\"logfile\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", logfile);
		//PID File
		fprintf(f,"\t<TR><TD>PID File:</TD><TD><input name=\"pidfile\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", cfg->pidfile);
		//Userfile
		fprintf(f,"\t<TR><TD>Usrfile:</TD><TD><input name=\"usrfile\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", cfg->usrfile);
		//Logfile
		fprintf(f,"\t<TR><TD>CWlogdir:</TD><TD><input name=\"cwlogdir\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", cfg->cwlogdir);
		//Clienttimeout
		fprintf(f,"\t<TR><TD>Clienttimeout:</TD><TD><input name=\"clienttimeout\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%ld\"></TD></TR>\r\n", cfg->ctimeout);
		//fallbacktimeout
		fprintf(f,"\t<TR><TD>Fallbacktimeout:</TD><TD><input name=\"fallbacktimeout\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%ld\"></TD></TR>\r\n", cfg->ftimeout);
		//clientmaxidle
		fprintf(f,"\t<TR><TD>Clientmaxidle:</TD><TD><input name=\"clientmaxidle\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->cmaxidle);
		//cachedelay
		fprintf(f,"\t<TR><TD>Cachedelay:</TD><TD><input name=\"cachedelay\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%ld\"></TD></TR>\r\n", cfg->delay);
		//bindwait
		fprintf(f,"\t<TR><TD>Bindwait:</TD><TD><input name=\"bindwait\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->bindwait);
		//netprio
		fprintf(f,"\t<TR><TD>Netprio:</TD><TD><input name=\"netprio\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%ld\"></TD></TR>\r\n", cfg->netprio);
		//resolvedelay
		fprintf(f,"\t<TR><TD>Resolvedelay:</TD><TD><input name=\"resolvedelay\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->resolvedelay);
		//sleep
		fprintf(f,"\t<TR><TD>Sleep:</TD><TD><input name=\"sleep\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->tosleep);
		//unlockparental
		fprintf(f,"\t<TR><TD>Unlockparental:</TD><TD><input name=\"unlockparental\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->ulparent);
		//nice
		fprintf(f,"\t<TR><TD>Nice:</TD><TD><input name=\"nice\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->nice);
		//serialreadertimeout
		fprintf(f,"\t<TR><TD>Serialreadertimeout:</TD><TD><input name=\"serialreadertimeout\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->srtimeout);
		//maxlogsize
		fprintf(f,"\t<TR><TD>Maxlogsize:</TD><TD><input name=\"maxlogsize\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->max_log_size);
		//showecmdw
		fprintf(f,"\t<TR><TD>Showecmdw:</TD><TD><input name=\"showecmdw\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->show_ecm_dw);
		//waitforcards
		fprintf(f,"\t<TR><TD>Waitforcards:</TD><TD><input name=\"waitforcards\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->waitforcards);
		//preferlocalcards
		fprintf(f,"\t<TR><TD>Preferlocalcards:</TD><TD><input name=\"preferlocalcards\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->preferlocalcards);
	
		//Tablefoot and finish form
		fprintf(f,"</TABLE>\r\n");
		fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
	
		//Disclaimer
		fprintf(f,"<BR><BR>Configuration Global not yet implemented chengings havn't any effect<BR><BR>");
	}
}

void send_oscam_config_camd33(FILE *f, struct uriparams *params) {
	int i;

	fprintf(f,"<BR><BR>");

	if (strcmp(getParam(params, "action"), "execute") == 0){
		//we found the execute flag
		for(i=0;i<(*params).paramcount;i++){
			if ((strcmp((*params).params[i], "part")) && (strcmp((*params).params[i], "action"))){
				fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", (*params).params[i], (*params).values[i]);
				//we use the same function as used for parsing the config tokens
				chk_t_camd33((*params).params[i], (*params).values[i]);
			}
		}

		//Disclaimer
		fprintf(f,"<BR><BR><B>Configuration camd33 *DONE*</B><BR><BR>");
		refresh_oscam(REFR_SERVER);
	} else {

		//if nothing above matches we show the form
		fprintf(f,"<form action=\"/config.html\" method=\"get\">\r\n");
		fprintf(f,"<input name=\"part\" type=\"hidden\" value=\"camd33\">\r\n");
		fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"execute\">\r\n");
		fprintf(f,"<TABLE cellspacing=\"0\">");
		fprintf(f,"\t<TH>&nbsp;</TH><TH>Edit Camd33 Config </TH>");
	
		//Port
		fprintf(f,"\t<TR><TD>Port:</TD><TD><input name=\"port\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->c33_port);
		//ServerIP
		fprintf(f,"\t<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", inet_ntoa(*(struct in_addr *)&cfg->c33_srvip));
		//Key
		int keysize =sizeof(cfg->c33_key);
		fprintf(f,"\t<TR><TD>Key:</TD><TD><input name=\"key\" type=\"text\" size=\"35\" maxlength=\"28\" value=\"");
		for (i=0;i<keysize;i++)
			fprintf(f,"%02X",cfg->c33_key[i]);
		fprintf(f,"\"></TD></TR>\r\n");
		//Passive
		fprintf(f,"\t<TR><TD>Passive:</TD><TD><input name=\"passive\" type=\"text\" size=\"3\" maxlength=\"1\" value=\"%d\"></TD></TR>\r\n", cfg->c33_passive);
		//Nocrypt
		fprintf(f,"\t<TR><TD>Nocrypt:</TD><TD><input name=\"nocrypt\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");
	  struct s_ip *cip;
	  char *dot="";
	  for (cip=cfg->c33_plain; cip; cip=cip->next){
	  	if (!(cip->ip[0] == cip->ip[1]))
				fprintf(f,"%s%s-%s", dot, inet_ntoa(*(struct in_addr *)&cip->ip[0]), inet_ntoa(*(struct in_addr *)&cip->ip[1]));
	  	else
				fprintf(f,"%s%s", dot, inet_ntoa(*(struct in_addr *)&cip->ip[0]));
	  }
		fprintf(f,"\">wrong, see Ticket #265</TD></TR>\r\n");
	
		//Tablefoot and finish form
		fprintf(f,"</TABLE>\r\n");
		fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
	
		//Disclaimer
		fprintf(f,"<BR><BR>Configuration camd33 not yet implemented<BR><BR>");
	}
}

void send_oscam_config_camd35(FILE *f, struct uriparams *params) {
	int i;

	fprintf(f,"<BR><BR>");

	if (strcmp(getParam(params, "action"),"execute") == 0){
			//we found the execute flag
			for(i=0;i<(*params).paramcount;i++){
				if ((strcmp((*params).params[i], "part")) && (strcmp((*params).params[i], "action"))){
					fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", (*params).params[i], (*params).values[i]);
					//we use the same function as used for parsing the config tokens
					chk_t_camd35((*params).params[i], (*params).values[i]);
				}
			}

			//Disclaimer
			fprintf(f,"<BR><BR><B>Configuration camd35 *DONE*</B><BR><BR>");
			refresh_oscam(REFR_SERVER);
	} else {
		//if nothing above matches we show the form
		fprintf(f,"<form action=\"/config.html\" method=\"get\">\r\n");
		fprintf(f,"<input name=\"part\" type=\"hidden\" value=\"camd35\">\r\n");
		fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"execute\">\r\n");
		fprintf(f,"<TABLE cellspacing=\"0\">");
		fprintf(f,"\t<TH>&nbsp;</TH><TH>Edit Camd35 Config </TH>");
	
		//Port
		fprintf(f,"\t<TR><TD>Port:</TD><TD><input name=\"port\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->c35_port);
		//ServerIP
		fprintf(f,"\t<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", inet_ntoa(*(struct in_addr *)&cfg->c35_tcp_srvip));
	
		//Tablefoot and finish form
		fprintf(f,"</TABLE>\r\n");
		fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
	
		//Disclaimer
		fprintf(f,"<BR><BR>Configuration camd35 not yet implemented<BR><BR>");
	}
}

void send_oscam_config_newcamd(FILE *f, struct uriparams *params) {
	int i;

	fprintf(f,"<BR><BR>");

	if (strcmp(getParam(params, "action"),"execute") == 0){
		//we found the execute flag
		for(i=0;i<(*params).paramcount;i++){
			if ((strcmp((*params).params[i], "part")) && (strcmp((*params).params[i], "action"))){
				fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", (*params).params[i], (*params).values[i]);
				//we use the same function as used for parsing the config tokens
				chk_t_newcamd((*params).params[i], (*params).values[i]);
			}
		}
	
		//Disclaimer
		fprintf(f,"<BR><BR><B>Configuration newcamd *DONE*</B><BR><BR>");
		refresh_oscam(REFR_SERVER);
	} else {
		//if nothing above matches we show the form
		fprintf(f,"<form action=\"/config.html\" method=\"get\">\r\n");
		fprintf(f,"<input name=\"part\" type=\"hidden\" value=\"newcamd\">\r\n");
		fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"execute\">\r\n");
		fprintf(f,"<TABLE cellspacing=\"0\">");
		fprintf(f,"\t<TH>&nbsp;</TH><TH>Edit Newcamd Config </TH>");
	
		//Port
		fprintf(f,"\t<TR><TD>Port:</TD><TD><input name=\"port\" type=\"text\" size=\"80\" maxlength=\"200\" value=\"");
		int j;
		char *dot1, *dot2;
		if (cfg->ncd_ptab.nports>0){
			dot1 = "";
			for(i=0;i<cfg->ncd_ptab.nports;i++){
				dot2 = "";
				fprintf(f, "%s%d", dot1, cfg->ncd_ptab.ports[i].s_port);
				fprintf(f, "@%04X", cfg->ncd_ptab.ports[i].ftab.filts[0].caid);
				if (cfg->ncd_ptab.ports[i].ftab.filts[0].nprids > 0){
					fprintf(f, ":");
					for (j = 0; j < cfg->ncd_ptab.ports[i].ftab.filts[0].nprids; j++){
						fprintf(f,"%s%lX", dot2, cfg->ncd_ptab.ports[i].ftab.filts[0].prids[j]);
						dot2 = ",";
					}
				}
				dot1=";";
			}
		}
	
		fprintf(f,"\"></TD></TR>\r\n");
	
		//ServerIP
		fprintf(f,"\t<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", inet_ntoa(*(struct in_addr *)&cfg->ncd_srvip));
		//Key
		fprintf(f,"\t<TR><TD>Key:</TD><TD><input name=\"key\" type=\"text\" size=\"35\" maxlength=\"28\" value=\"");
	
		for (i=0;i<14;i++)
			fprintf(f,"%02X",cfg->ncd_key[i]);
	
		fprintf(f,"\"></TD></TR>\r\n");
		//Tablefoot and finish form
		fprintf(f,"</TABLE>\r\n");
		fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
	}
}

void send_oscam_config_radegast(FILE *f, struct uriparams *params) {
	int i;

	fprintf(f,"<BR><BR>");
	
	if (strcmp(getParam(params, "action"),"execute") == 0){
		//we found the execute flag
		for(i=0;i<(*params).paramcount;i++){
			if ((strcmp((*params).params[i], "part")) && (strcmp((*params).params[i], "action"))){
				fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", (*params).params[i], (*params).values[i]);
				//we use the same function as used for parsing the config tokens
				chk_t_radegast((*params).params[i], (*params).values[i]);
			}
		}

		//Disclaimer
		fprintf(f,"<BR><BR><B>Configuration Radegast *DONE*</B><BR><BR>");
		refresh_oscam(REFR_SERVER);
	} else {

		//if nothing above matches we show the form
		fprintf(f,"<form action=\"/config.html\" method=\"get\">\r\n");
		fprintf(f,"<input name=\"part\" type=\"hidden\" value=\"radegast\">\r\n");
		fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"execute\">\r\n");
		fprintf(f,"<TABLE cellspacing=\"0\">");
		fprintf(f,"\t<TH>&nbsp;</TH><TH>Edit Radegast Config </TH>");
	
		//Port
		fprintf(f,"\t<TR><TD>Port:</TD><TD><input name=\"port\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->rad_port);
		//ServerIP
		fprintf(f,"\t<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", inet_ntoa(*(struct in_addr *)&cfg->rad_srvip));
		//Allowed
		fprintf(f,"\t<TR><TD>Allowed:</TD><TD><input name=\"allowed\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");
	  struct s_ip *cip;
	  char *dot="";
	  for (cip=cfg->rad_allowed; cip; cip=cip->next){
	  	if (!(cip->ip[0] == cip->ip[1]))
				fprintf(f,"%s%s-%s", dot, inet_ntoa(*(struct in_addr *)&cip->ip[0]), inet_ntoa(*(struct in_addr *)&cip->ip[1]));
	  	else
				fprintf(f,"%s%s", dot, inet_ntoa(*(struct in_addr *)&cip->ip[0]));
	  }
		fprintf(f,"\">wrong, see Ticket #265</TD></TR>\r\n");
		//Port
		fprintf(f,"\t<TR><TD>User:</TD><TD><input name=\"user\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", cfg->rad_usr);
	
		//Tablefoot and finish form
		fprintf(f,"</TABLE>\r\n");
		fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
	}
}

void send_oscam_config_cccam(FILE *f, struct uriparams *params) {
	int i;

	fprintf(f,"<BR><BR>");

	if (strcmp(getParam(params, "action"),"execute") == 0){
		//we found the execute flag
		for(i=0;i<(*params).paramcount;i++){
			if ((strcmp((*params).params[i], "part")) && (strcmp((*params).params[i], "action"))){
				fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", (*params).params[i], (*params).values[i]);
				//we use the same function as used for parsing the config tokens
				chk_t_cccam((*params).params[i], (*params).values[i]);
			}
		}

		//Disclaimer
		fprintf(f,"<BR><BR>Configuration Cccam Do not yet implemented<BR><BR>");
		refresh_oscam(REFR_SERVER);
	} else {

		//if nothing above matches we show the form
		fprintf(f,"<form action=\"/config.html\" method=\"get\">\r\n");
		fprintf(f,"<input name=\"part\" type=\"hidden\" value=\"cccam\">\r\n");
		fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"execute\">\r\n");
		fprintf(f,"<TABLE cellspacing=\"0\">");
		fprintf(f,"\t<TH>&nbsp;</TH><TH>Edit Cccam Config </TH>");
	
	
		//Tablefoot and finish form
		fprintf(f,"</TABLE>\r\n");
		fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
	
		//Disclaimer
		fprintf(f,"<BR><BR>Configuration Cccam not yet implemented<BR><BR>");
	}
}

void send_oscam_config_gbox(FILE *f, struct uriparams *params) {
	int i;

	fprintf(f,"<BR><BR>");

	if (strcmp(getParam(params, "action"),"execute") == 0){
		//we found the execute flag
		for(i=0;i<(*params).paramcount;i++){
			if ((strcmp((*params).params[i], "part")) && (strcmp((*params).params[i], "action"))){
				fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", (*params).params[i], (*params).values[i]);
				//we use the same function as used for parsing the config tokens
				chk_t_gbox((*params).params[i], (*params).values[i]);
			}
		}

		//Disclaimer
		fprintf(f,"<BR><BR><B>Configuration Gbox *DONE*</B><BR><BR>");
		refresh_oscam(REFR_SERVER);
	} else {

		//if nothing above matches we show the form
		fprintf(f,"<form action=\"/config.html\" method=\"get\">\r\n");
		fprintf(f,"<input name=\"part\" type=\"hidden\" value=\"gbox\">\r\n");
		fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"execute\">\r\n");
		fprintf(f,"<TABLE cellspacing=\"0\">");
		fprintf(f,"\t<TH>&nbsp;</TH><TH>Edit Gbox Config </TH>");
	
		//Password
		fprintf(f,"\t<TR><TD>Password:</TD><TD><input name=\"password\" type=\"text\" size=\"10\" maxlength=\"8\" value=\"");
		for (i=0;i<4;i++)
			fprintf(f,"%02X",cfg->gbox_pwd[i]);
		fprintf(f,"\"></TD></TR>\r\n");
		//Maxdist
		fprintf(f,"\t<TR><TD>Maxdist:</TD><TD><input name=\"maxdist\" type=\"text\" size=\"5\" maxlength=\"2\" value=\"%d\"></TD></TR>\r\n", cfg->maxdist);
		//ignorelist
		fprintf(f,"\t<TR><TD>Ignorelist:</TD><TD><input name=\"ignorelist\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", cfg->ignorefile);
		//onlineinfos
		fprintf(f,"\t<TR><TD>Onlineinfos:</TD><TD><input name=\"onlineinfos\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", cfg->gbxShareOnl);
		//cardinfos
		fprintf(f,"\t<TR><TD>Cardinfos:</TD><TD><input name=\"cardinfos\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", cfg->cardfile);
		//locals
		fprintf(f,"\t<TR><TD>Locals:</TD><TD><input name=\"locals\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");
		char *dot = "";
		for (i = 0; i < cfg->num_locals; i++){
			fprintf(f,"%s%06lX", dot, cfg->locals[i]);
			dot=";";
		}
		fprintf(f,"\"></TD></TR>\r\n");
	
		//Tablefoot and finish form
		fprintf(f,"</TABLE>\r\n");
		fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
	}
}

void send_oscam_config_monitor(FILE *f, struct uriparams *params) {
	int i;

	fprintf(f,"<BR><BR>");

	if (strcmp(getParam(params, "action"),"execute") == 0){
		//we found the execute flag
		for(i=0;i<(*params).paramcount;i++){
			if ((strcmp((*params).params[i], "part")) && (strcmp((*params).params[i], "action"))){
				fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", (*params).params[i], (*params).values[i]);
				//we use the same function as used for parsing the config tokens
				chk_t_monitor((*params).params[i], (*params).values[i]);
			}
		}

		//Disclaimer
		fprintf(f,"<BR><BR><B>Configuration Monitor *DONE*</B><BR><BR>");
		refresh_oscam(REFR_SERVER);
	} else {

		//if nothing above matches we show the form
		fprintf(f,"<form action=\"/config.html\" method=\"get\">\r\n");
		fprintf(f,"<input name=\"part\" type=\"hidden\" value=\"monitor\">\r\n");
		fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"execute\">\r\n");
		fprintf(f,"<TABLE cellspacing=\"0\">");
		fprintf(f,"\t<TH>&nbsp;</TH><TH>Edit Monitor Config </TH>");
	
		//Port
		fprintf(f,"\t<TR><TD>Port:</TD><TD><input name=\"port\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->mon_port);
		//ServerIP
		fprintf(f,"\t<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", inet_ntoa(*(struct in_addr *)&cfg->mon_srvip));
		//Nocrypt
		fprintf(f,"\t<TR><TD>Nocrypt:</TD><TD><input name=\"nocrypt\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");
	  struct s_ip *cip;
	  char *dot="";
	  for (cip=cfg->mon_allowed; cip; cip=cip->next){
	  	if (!(cip->ip[0] == cip->ip[1]))
				fprintf(f,"%s%s-%s", dot, inet_ntoa(*(struct in_addr *)&cip->ip[0]), inet_ntoa(*(struct in_addr *)&cip->ip[1]));
	  	else
				fprintf(f,"%s%s", dot, inet_ntoa(*(struct in_addr *)&cip->ip[0]));
	  }
		fprintf(f,"\">wrong, see Ticket #265</TD></TR>\r\n");
		//aulow
		fprintf(f,"\t<TR><TD>Aulow:</TD><TD><input name=\"aulow\" type=\"text\" size=\"5\" maxlength=\"1\" value=\"%d\"></TD></TR>\r\n", cfg->mon_aulow);
		//Monlevel
		fprintf(f,"<TR><TD>Monlevel:</TD><TD><select name=\"monlevel\" >\r\n");
			for(i = 0; i < 5; i++){
				if(i == cfg->mon_level)
					fprintf(f,"\t<option value=\"%d\" selected>%s</option>\r\n", i, monlevel[i]);
				else
					fprintf(f,"\t<option value=\"%d\">%s</option>\r\n", i, monlevel[i]);
		}
		fprintf(f,"</select></TD></TR>\r\n");
		//HTTPport
		fprintf(f,"\t<TR><TD>Port:</TD><TD><input name=\"httpport\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->http_port);
		//hideclient_to
		fprintf(f,"\t<TR><TD>Hideclientto:</TD><TD><input name=\"hideclient_to\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->mon_hideclient_to);
		//HTTPuser
		fprintf(f,"<TR><TD>Httpuser:</TD><TD><input name=\"httpuser\" type=\"text\" size=\"20\" maxlength=\"20\" value=\"%s\"></TD></TR>\r\n", cfg->http_user);
		//HTTPpassword
		fprintf(f,"<TR><TD>Httppwd:</TD><TD><input name=\"httppwd\" type=\"text\" size=\"20\" maxlength=\"20\" value=\"%s\"></TD></TR>\r\n", cfg->http_pwd);
		//HTTPcss
		fprintf(f,"<TR><TD>Httpcss:</TD><TD><input name=\"httpcss\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", cfg->http_css);
		//HTTPrefresh
		fprintf(f,"\t<TR><TD>Httprefresh:</TD><TD><input name=\"httprefresh\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->http_refresh);
	
		//Tablefoot and finish form
		fprintf(f,"</TABLE>\r\n");
		fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
	}
}

#ifdef CS_ANTICASC

void send_oscam_config_anticasc(FILE *f, struct uriparams *params) {
	int i;

	fprintf(f,"<BR><BR>");

	if (strcmp(getParam(params, "action"),"execute") == 0){
		//we found the execute flag
		for(i=0;i<(*params).paramcount;i++){
			if ((strcmp((*params).params[i], "part")) && (strcmp((*params).params[i], "action"))){
				fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", (*params).params[i], (*params).values[i]);
				//we use the same function as used for parsing the config tokens
				chk_t_ac((*params).params[i], (*params).values[i]);
			}
		}

		//Disclaimer
		fprintf(f,"<BR><BR><B>Configuration Anticascading *DONE*</B><BR><BR>");
		refresh_oscam(REFR_ANTICASC);
	} else{

		//if nothing above matches we show the form
		fprintf(f,"<form action=\"/config.html\" method=\"get\">\r\n");
		fprintf(f,"<input name=\"part\" type=\"hidden\" value=\"anticasc\">\r\n");
		fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"execute\">\r\n");
		fprintf(f,"<TABLE cellspacing=\"0\">");
		fprintf(f,"\t<TH>&nbsp;</TH><TH>Edit Anticascading Config </TH>");
	
		//Port
		char *checked="";
		if (cfg->ac_enabled > 0) checked="checked";
		fprintf(f,"\t<TR><TD>Enabled:</TD><TD><input name=\"enabled\" type=\"checkbox\" value=\"1\" %s>\r\n", checked);
		//numusers
		fprintf(f,"\t<TR><TD>Numusers:</TD><TD><input name=\"numusers\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->ac_users);
		//sampletime
		fprintf(f,"\t<TR><TD>Sampletime:</TD><TD><input name=\"sampletime\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->ac_stime);
		//samples
		fprintf(f,"\t<TR><TD>Samples:</TD><TD><input name=\"samples\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->ac_samples);
		//penalty
		fprintf(f,"\t<TR><TD>Penalty:</TD><TD><input name=\"penalty\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->ac_penalty);
		//aclogfile
		fprintf(f,"\t<TR><TD>AClogfile:</TD><TD><input name=\"aclogfile\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", cfg->ac_logfile);
		//fakedelay
		fprintf(f,"\t<TR><TD>Fakedelay:</TD><TD><input name=\"fakedelay\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->ac_fakedelay);
		//denysamples
		fprintf(f,"\t<TR><TD>Denysamples:</TD><TD><input name=\"denysamples\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"%d\"></TD></TR>\r\n", cfg->ac_denysamples);
	
		//Tablefoot and finish form
		fprintf(f,"</TABLE>\r\n");
		fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
	}
}

#endif

void send_oscam_config(FILE *f, struct uriparams *params) {
	fprintf(f,"<BR><BR>");

	/*create submenue*/
	fprintf(f, "<TABLE border=0 class=\"menu\">\n");
	fprintf(f, "	<TR>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"config.html?part=global\">Global</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"config.html?part=camd33\">Camd3.3</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"config.html?part=camd35\">Camd3.5</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"config.html?part=newcamd\">Newcamd</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"config.html?part=radegast\">Radegast</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"config.html?part=cccam\">Cccam</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"config.html?part=gbox\">Gbox</TD>\n");
#ifdef CS_ANTICASC
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"config.html?part=anticasc\">Anticascading</TD>\n");
#endif
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"config.html?part=monitor\">Monitor</TD>\n");
	fprintf(f, "	</TR>\n");
	fprintf(f, "</TABLE>\n");

	char *part = getParam(params, "part");
	if (strlen(part) == 0)
		send_oscam_config_global(f, params);
	else {
		if (!strcmp(part,"global"))
			send_oscam_config_global(f, params);
		else if (!strcmp(part,"camd33"))
			send_oscam_config_camd33(f, params);
		else if (!strcmp(part,"camd35"))
			send_oscam_config_camd35(f, params);
		else if (!strcmp(part,"newcamd"))
			send_oscam_config_newcamd(f, params);
		else if (!strcmp(part,"radegast"))
			send_oscam_config_radegast(f, params);
		else if (!strcmp(part,"cccam"))
			send_oscam_config_cccam(f, params);
		else if (!strcmp(part,"gbox"))
			send_oscam_config_gbox(f, params);
#ifdef CS_ANTICASC
		else if (!strcmp(part,"anticasc"))
			send_oscam_config_anticasc(f, params);
#endif
		else if (!strcmp(part,"monitor"))
			send_oscam_config_monitor(f, params);
		else
			send_oscam_config_global(f, params);
	}
}

void send_oscam_reader(FILE *f) {
	int ridx;
	char *ctyp;

	fprintf(f,"<BR><BR><TABLE cellspacing=\"0\" cellpadding=\"10\">\r\n");
	fprintf(f,"<TR><TH>Label</TH><TH>Protocol</TH><TH>Action</TH></TR>");

	for(ridx=0;ridx<CS_MAXREADER;ridx++){
		if(!reader[ridx].device[0]) break;

		switch(reader[ridx].typ)
			{
				case R_MOUSE   : ctyp="mouse";    break;
				case R_INTERNAL: ctyp="intern";   break;
				case R_SMART   : ctyp="smartreader";    break;
				case R_CAMD35  : ctyp="camd 3.5x";break;
				case R_CAMD33  : ctyp="camd 3.3x";break;
				case R_NEWCAMD : ctyp="newcamd";  break;
				case R_RADEGAST: ctyp="radegast"; break;
				case R_SERIAL  : ctyp="serial";   break;
				case R_GBOX    : ctyp="gbox";     break;
#ifdef HAVE_PCSC
				case R_PCSC    : ctyp="pcsc";     break;
#endif
				case R_CCCAM   : ctyp="cccam";    break;
				case R_CS378X  : ctyp="cs378x";   break;
				default        : ctyp="unknown";  break;
			}

		fprintf(f,"\t<TR><TD>%s</TD><TD>%s</TD><TD><A HREF=\"readerconfig.html?user=%s\">Edit Settings</A></TD></TR>",reader[ridx].label, ctyp, reader[ridx].label);
	}
	fprintf(f,"</TABLE>\r\n");
	//kill(client[0].pid, SIGUSR1);
}

void send_oscam_reader_config(FILE *f, struct uriparams *params) {
	int i,ridx;

	fprintf(f,"<BR><BR>\r\n");
	char *reader_ = getParam(params, "reader");

	for(ridx=0;ridx<CS_MAXREADER;ridx++){
		if (!reader[ridx].device[0]){
			fprintf(f,"Reader %s not found", reader_);
			return ;
		}
		if (!strcmp(reader_,reader[ridx].label)) break;
	}

	/*build form head*/
	fprintf(f,"<form action=\"/readerconfig_do.html\" method=\"get\"><input name=\"reader\" type=\"hidden\" value=\"%s\">\r\n", reader[ridx].label);
	fprintf(f,"<TABLE cellspacing=\"0\">");
	fprintf(f,"<TH>&nbsp;</TH><TH>Edit Reader %s </TH>", reader[ridx].label);

	/*build form fields*/
	fprintf(f,"<TR><TD>Device:</TD><TD><input name=\"device\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s", reader[ridx].device);
	if(reader[ridx].r_port) fprintf(f,",%d",reader[ridx].r_port);
	if(reader[ridx].l_port) fprintf(f,",%d",reader[ridx].l_port);
	fprintf(f,"\"></TD></TR>\r\n");
	//Group
	fprintf(f,"<TR><TD>Group:</TD><TD><input name=\"grp\" type=\"text\" size=\"10\" maxlength=\"10\" value=\"");
	/*restore the settings format of group from long over bitarray*/
	char *dot = ""; //flag for comma
	char grpbit[33];
	long2bitchar(reader[ridx].grp, grpbit);
	for(i = 0; i < 32; i++){
		if (grpbit[i] == '1'){
				fprintf(f, "%s%d", dot, i+1);
				dot = ",";
			}
		}
	fprintf(f,"\"></TD></TR>\r\n");
	fprintf(f,"<TR><TD>Key:</TD><TD><input name=\"key\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].ncd_key);
	//fprintf(f,"<TR><TD>Password:</TD><TD><input name=\"password\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].gbox_pwd);
	fprintf(f,"<TR><TD>Pincode:</TD><TD><input name=\"pincode\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].pincode);
	fprintf(f,"<TR><TD>Readnano:</TD><TD><input name=\"readnano\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].emmfile);

	//services
	char sidok[33];
	long2bitchar(reader[ridx].sidtabok, sidok);
	char sidno[33];
	long2bitchar(reader[ridx].sidtabno,sidno);
	struct s_sidtab *sidtab = cfg->sidtab;
	fprintf(f,"<TR><TD>Services:</TD><TD><TABLE cellspacing=\"0\" class=\"invisible\">");
	int pos=0;
	char *chk;
	//build matrix
	for (; sidtab; sidtab=sidtab->next){
		chk="";
		if(sidok[pos]=='1') chk="checked";
		fprintf(f,"<TR><TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"%s\" %s> %s</TD>", sidtab->label, chk, sidtab->label);
		chk="";
		if(sidno[pos]=='1') chk="checked";
		fprintf(f,"<TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"!%s\" %s> !%s</TD></TR>\r\n", sidtab->label, chk, sidtab->label);
		pos++;
	}
	fprintf(f,"</TD></TR></TABLE>\r\n");


	fprintf(f,"<TR><TD>Inactivitytimeout:</TD><TD><input name=\"inactivitytimeout\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%d\"></TD></TR>\r\n", reader[ridx].tcp_ito);
	fprintf(f,"<TR><TD>Reconnecttimeout:</TD><TD><input name=\"reconnecttimeout\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%d\"></TD></TR>\r\n", reader[ridx].tcp_rto);
	fprintf(f,"<TR><TD>Disableserverfilter:</TD><TD><input name=\"disableserverfilter\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%d\"></TD></TR>\r\n", reader[ridx].ncd_disable_server_filt);
	fprintf(f,"<TR><TD>Fallback:</TD><TD><input name=\"fallback\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"%d\"></TD></TR>\r\n", reader[ridx].fallback);
	//fprintf(f,"<TR><TD>Logport:</TD><TD><input name=\"logport\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%d\"></TD></TR>\r\n", reader[ridx].log_port);
	//fprintf(f,"<TR><TD>Caid:</TD><TD><input name=\"caid\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].log_port);
	/*---------------------CAID-----------------------------
	-------------------------------------------------------*/
	i = 0;
	CAIDTAB *ctab = &reader[ridx].ctab;

	fprintf(f,"<TR><TD>CAID:</TD><TD><input name=\"caid\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");
	/*make string from caidtab*/
	if (ctab->caid[i]){
		while(ctab->caid[i]) {
			if (i==0)	fprintf(f, "%04X", ctab->caid[i]);
			else fprintf(f, ",%04X", ctab->caid[i]);
			if(ctab->mask[i])	fprintf(f, "&%04X", ctab->mask[i]);
			i++;
		}
	}
	fprintf(f,"\"></TD></TR>\r\n");


	fprintf(f,"<TR><TD>Boxid:</TD><TD><input name=\"boxid\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%ld\"></TD></TR>\r\n", reader[ridx].boxid);



	/*build form foot*/
	fprintf(f,"</TABLE>\r\n");
	fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");

	fprintf(f,"<BR><BR>Reader not yet implemented - Nothing changes on click<BR><BR>");
}

void send_oscam_reader_config_do(FILE *f, struct uriparams *params) {
	int ridx;

	fprintf(f,"<BR><BR>\r\n");
	char *reader_ = getParam(params, "reader");

	for(ridx=0;ridx<CS_MAXREADER;ridx++){
		if (!reader[ridx].device[0]){
			fprintf(f,"Reader %s not found", reader_);
			return ;
		}
		if (!strcmp(reader_,reader[ridx].label)) break;
	}

	fprintf(f,"Reader: %s - Nothing changed", reader[ridx].label);
	fprintf(f,"<BR><BR>Reader not yet implemented<BR><BR>");
	refresh_oscam(REFR_READERS);
}

void send_oscam_user_config_add(FILE *f, struct uriparams *params) {
		struct s_auth *ptr;
		struct s_auth *account;
		int i, accidx=0;

		char *newuser = getParam(params, "user");

		if (strlen(newuser) > 0) {

			for (account=cfg->account; (account) ; account=account->next){
				//account already exist - show config for this account
				if(!strcmp(newuser, account->usr)){
					(*params).params[0] = "user";
					(*params).values[0] = newuser;
					(*params).paramcount = 1;
					send_oscam_user_config(f, params);
					return;
				}
				//last account reached
				if(!account->next) break;
				accidx++;
			}

      if (!(ptr=malloc(sizeof(struct s_auth))))
      {
        cs_log("Error allocating memory (errno=%d)", errno);
        return;
      }
      if (account)
        account->next=ptr;
      else
        cfg->account=ptr;
      account=ptr;
      memset(account, 0, sizeof(struct s_auth));
      strncpy((char *)account->usr, newuser, sizeof(account->usr)-1);
      account->au=(-1);
      account->monlvl=cfg->mon_level;
      account->tosleep=cfg->tosleep;
      for (i=1; i<CS_MAXCAIDTAB; account->ctab.mask[i++]=0xffff);
      for (i=1; i<CS_MAXTUNTAB; account->ttab.bt_srvid[i++]=0x0000);
			accidx++;
#ifdef CS_ANTICASC
      account->ac_users=cfg->ac_users;
      account->ac_penalty=cfg->ac_penalty;
      account->ac_idx = accidx;
#endif
    }
	(*params).params[0] = "user";
	(*params).values[0] = newuser;
	(*params).paramcount = 1;
	send_oscam_user_config(f, params);

}

void send_oscam_user_config_delete(FILE *f, struct uriparams *params) {

	int found = 0;
	struct s_auth *account, *account2;

	char *user = getParam(params, "user");
	
	account=cfg->account;
	if(strcmp(account->usr, user) == 0){
		cfg->account = account->next;
		free(account);
		found = 1;
	} else if (account->next != NULL){
		do{
			if(strcmp(account->next->usr, user) == 0){
				account2 = account->next;
				account->next = account2->next;
				free(account2);
				found = 1;
				break;
			}
		} while ((account = account->next) && (account->next != NULL));
	}

	if (found>0){
		if (write_userdb()==0)
			refresh_oscam(REFR_ACCOUNTS);
		else
			fprintf(f,"<B>Write Config failed</B><BR><BR>\r\n");
	}
	else
		fprintf(f,"<B>User not found</B><BR><BR>\r\n");
	(*params).paramcount = 0;
	send_oscam_user_config(f, params);
}

void send_oscam_user_config_do(FILE *f, struct uriparams *params) {

	struct s_auth *account;
	int i;
	char servicelabels[255]="";

	//find username within params
	char *user = getParam(params, "user");

	//identfy useraccount
	for (account=cfg->account; (account) ; account=account->next){
		if(!strcmp(user, account->usr))
			break;
	}

	//clear group
	account->grp = 0;

	//fprintf(f,"<BR><BR>\r\n");

	for(i=0;i<(*params).paramcount;i++){
		if ((strcmp((*params).params[i], "action")) && (strcmp((*params).params[i], "user"))){
			//fprintf(f,"%s = %s<BR>\r\n", (*params).params[i], (*params).values[i]);
			if (!strcmp((*params).params[i], "services"))
				sprintf(servicelabels + strlen(servicelabels), "%s,", (*params).values[i]);
			else
				chk_account((*params).params[i], (*params).values[i], account);
		}
	//	else if (!strcmp((*params).params[i], "user"))
	//		fprintf(f,"<B>User %s is reconfigured as follow</B><BR><BR>\r\n", (*params).values[i]);
	}

	chk_account("services", servicelabels, account);

	if (write_userdb()==0)
		refresh_oscam(REFR_ACCOUNTS);
	else
		fprintf(f,"<B>Write Config failed</B><BR><BR>\r\n");
	(*params).paramcount = 0;
	send_oscam_user_config(f, params);
}

void send_oscam_user_config(FILE *f, struct uriparams *params) {

	int i;
	struct s_auth *account;
	char *action = getParam(params, "action");
	if ((*params).paramcount>0){
		if (!strcmp(action, "execute")) {
			send_oscam_user_config_do(f, params);
			return;
		}	else if (!strcmp(action, "adduser")) {
			send_oscam_user_config_add(f, params);
			return;
		}	else if (!strcmp(action, "delete")) {
			send_oscam_user_config_delete(f, params);
			return;
		}
	}	else {
		/* No Parameters given --> list accounts*/
		char *status = "offline";
		char *expired = "";
		char *classname="offline";
		char *lastchan="&nbsp;";
		time_t now = time((time_t)0);
		int isec=0;

		fprintf(f,"<BR><BR><TABLE cellspacing=\"0\" cellpadding=\"10\">\r\n");
		fprintf(f,"<TR><TH>Label</TH>\r\n\t<TH>Status</TH>\r\n\t<TH>Last Channel</TH>\r\n\t<TH>Idle (Sec)</TH>\r\n\t<TH colspan=\"2\" align=\"center\">Action</TH>\r\n</TR>");
		for (account=cfg->account; (account) ; account=account->next){
			expired = ""; classname="offline";
			if(account->expirationdate && account->expirationdate<time(NULL)){
				expired = " (expired)";
				classname = "expired";
			}
			status="offline";

			//search account in active clients
			for (i=0; i<CS_MAXPID; i++)
				if (!strcmp(client[i].usr, account->usr)){
					//30 secs without ecm is offline
					if ((now - client[i].lastecm) < 30){
						status = "<b>online</b>";classname="online";
						lastchan = monitor_get_srvname(client[i].last_srvid);
						isec = now - client[i].last;
					}
				}
			fprintf(f,"<TR class=\"%s\">\r\n", classname);
			fprintf(f,"\t<TD>%s</TD>\r\n\t<TD>%s%s</TD>\r\n\t<TD>%s</TD>\r\n\t<TD>%d</TD>\r\n\t<TD><A HREF=\"userconfig.html?user=%s\">Edit Settings</A></TD>\r\n",account->usr, status, expired, lastchan, isec, account->usr);
			fprintf(f,"\t<TD><A HREF=\"userconfig.html?user=%s&action=delete\">Delete User</A></TD>\r\n",account->usr);
			fprintf(f,"</TR>\r\n");
			isec = 0;
			lastchan = "&nbsp;";
		}
		fprintf(f,"<TR>\r\n");
		fprintf(f,"\t<FORM action=\"/userconfig.html\" method=\"get\">\r\n");
		fprintf(f,"\t<TD>New User:</TD>\r\n");
		fprintf(f,"\t<TD colspan=\"2\"><input name=\"user\" type=\"text\"></TD>\r\n");
		fprintf(f,"\t<input name=\"action\" type=\"hidden\" value=\"adduser\">\r\n");
		fprintf(f,"\t<TD colspan=\"3\" align=\"center\"><input type=\"submit\" value=\"Add User\"></TD>\r\n");
		fprintf(f,"\t</FORM>\r\n");
		fprintf(f,"<TR>\r\n");
		fprintf(f,"</TABLE>\r\n");

		return;
	}

	fprintf(f,"<BR><BR>\r\n");
	
	char *user = getParam(params, "user");

	/*identfy useraccount*/
	for (account=cfg->account; (account) ; account=account->next){
		if(!strcmp(user, account->usr))
			break;
	}

	fprintf(f,"<form action=\"/userconfig.html\" method=\"get\">");
	fprintf(f,"<input name=\"user\" type=\"hidden\" value=\"%s\">\r\n", account->usr);
	fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"execute\">\r\n");
	fprintf(f,"<TABLE cellspacing=\"0\">");
	fprintf(f,"<TH>&nbsp;</TH><TH>Edit User %s </TH>", account->usr);
	//Password
	fprintf(f,"<TR><TD>Password:</TD><TD><input name=\"pwd\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", account->pwd);
	//Expirationdate
	struct tm * timeinfo = localtime (&account->expirationdate);
	char buf [80];
	strftime (buf,80,"%Y-%m-%d",timeinfo);
	if(strcmp(buf,"1970-01-01"))
		fprintf(f,"<TR><TD>Exp. Date:</TD><TD><input name=\"expdate\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n",buf);
	else
		fprintf(f,"<TR><TD>Exp. Date:</TD><TD><input name=\"expdate\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"\"></TD></TR>\r\n");
	//Group
	fprintf(f,"<TR><TD>Group:</TD><TD><input name=\"group\" type=\"text\" size=\"10\" maxlength=\"10\" value=\"");
	/*restore the settings format of group from long over bitarray*/
	char *dot = ""; //flag for comma
	char grpbit[33];
	long2bitchar(account->grp, grpbit);
	for(i = 0; i < 32; i++){
		if (grpbit[i] == '1'){
				fprintf(f, "%s%d", dot, i+1);
				dot = ",";
			}
	}
	fprintf(f,"\"></TD></TR>\r\n");
	//Hostname
	if (strlen((char *)account->dyndns)>0)
		fprintf(f,"<TR><TD>Hostname:</TD><TD><input name=\"hostname\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", account->dyndns);
	//Uniq
	fprintf(f,"<TR><TD>Uniq:</TD><TD><select name=\"uniq\" >\r\n");
		for(i = 0; i < 3; i++){
		if(i == account->uniq)
			fprintf(f,"\t<option value=\"%d\" selected>%s</option>\r\n", i, uniq[i]);
		else
			fprintf(f,"\t<option value=\"%d\">%s</option>\r\n", i, uniq[i]);
		}
	fprintf(f,"</SELECT></TD></TR>\r\n");
	//Sleep
	if(!account->tosleep)
		fprintf(f,"<TR><TD>Sleep:</TD><TD><input name=\"sleep\" type=\"text\" size=\"4\" maxlength=\"4\" value=\"0\"></TD></TR>\r\n");
	else
		fprintf(f,"<TR><TD>Sleep:</TD><TD><input name=\"sleep\" type=\"text\" size=\"4\" maxlength=\"4\" value=\"%d\"></TD></TR>\r\n", account->tosleep);
	//Monlevel selector
	fprintf(f,"<TR><TD>Monlevel:</TD><TD><select name=\"monlevel\" >\r\n");
	for(i = 0; i < 5; i++){
		if(i == account->monlvl)
			fprintf(f,"\t<option value=\"%d\" selected>%s</option>\r\n", i, monlevel[i]);
		else
			fprintf(f,"\t<option value=\"%d\">%s</option>\r\n", i, monlevel[i]);
	}
	fprintf(f,"</select></TD></TR>\r\n");
	//AU Selector
	fprintf(f,"<TR><TD>AU:</TD><TD><select name=\"au\" >\r\n");
	if (!account->au)
		fprintf(f,"\t<option value=\" \" selected>none</option>\r\n");
	else
		fprintf(f,"\t<option value=\" \">none</option>\r\n");

	if (account->autoau == 1)
		fprintf(f,"\t<option value=\"1\" selected>auto</option>\r\n");
	else
		fprintf(f,"\t<option value=\"1\">auto</option>\r\n");

	int ridx;
	for (ridx=0; ridx<CS_MAXREADER; ridx++){
		if(!reader[ridx].device[0]) break;

		if (account->au == ridx)
			fprintf(f,"\t<option value=\"%s\" selected>%s</option>\r\n", reader[ridx].label, reader[ridx].label);
		else
			fprintf(f,"\t<option value=\"%s\">%s</option>\r\n", reader[ridx].label, reader[ridx].label);
	}
	fprintf(f,"</select></TD></TR>\r\n");

	/*---------------------SERVICES-------------------------
	-------------------------------------------------------*/
	//services - first we have to move the long sidtabok/sidtabno to a binary array
	char sidok[33];
	long2bitchar(account->sidtabok,sidok);
	char sidno[33];
	long2bitchar(account->sidtabno,sidno);
	struct s_sidtab *sidtab = cfg->sidtab;
	fprintf(f,"<TR><TD>Services:</TD><TD><TABLE cellspacing=\"0\" class=\"invisible\">");
	int pos=0;
	char *chk;
	//build matrix
	for (; sidtab; sidtab=sidtab->next){
		chk="";
		if(sidok[pos]=='1') chk="checked";
		fprintf(f,"<TR><TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"%s\" %s> %s</TD>", sidtab->label, chk, sidtab->label);
		chk="";
		if(sidno[pos]=='1') chk="checked";
		fprintf(f,"<TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"!%s\" %s> !%s</TD></TR>\r\n", sidtab->label, chk, sidtab->label);
		pos++;
	}
	fprintf(f,"</TD></TR></TABLE>\r\n");

	/*---------------------CAID-----------------------------
	-------------------------------------------------------*/
	i = 0;
	CAIDTAB *ctab = &account->ctab;

	fprintf(f,"<TR><TD>CAID:</TD><TD><input name=\"caid\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");
	/*make string from caidtab*/
	if (ctab->caid[i]){
		while(ctab->caid[i]) {
			if (i==0)	fprintf(f, "%04X", ctab->caid[i]);
			else fprintf(f, ",%04X", ctab->caid[i]);
			if(ctab->mask[i])	fprintf(f, "&%04X", ctab->mask[i]);
			i++;
		}
	}
	fprintf(f,"\"></TD></TR>\r\n");

	/*IDENT*/
	fprintf(f,"<TR><TD>Ident:</TD><TD><input name=\"ident\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");

	int j;
	dot="";
	FTAB *ftab = &account->ftab;
	for (i=0;i<ftab->nfilts;i++){
		fprintf(f, "%s%04X", dot, ftab->filts[i].caid);

		for (j=0;j<ftab->filts[i].nprids;j++)
			fprintf(f, ":%06lX", ftab->filts[i].prids[j]);

		dot=";";
	}
	fprintf(f,"\"></TD></TR>\r\n");

	/*Betatunnel*/
	i = 0;
	TUNTAB *ttab = &account->ttab;

	fprintf(f,"<TR><TD>Betatunnel:</TD><TD><input name=\"betatunnel\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");
	/*make string from Betatunneltab*/
	if (ttab->bt_caidfrom[i]) {
		while(ttab->bt_caidfrom[i]) {
			if (i==0)	fprintf(f, "%04X", ttab->bt_caidfrom[i]);
			else fprintf(f, ",%04X", ttab->bt_caidfrom[i]);
			if(ttab->bt_caidto[i]) fprintf(f, ".%04X", ttab->bt_caidto[i]);
			if(ttab->bt_srvid[i])	fprintf(f, ":%04X", ttab->bt_srvid[i]);
			i++;
		}
	}
	fprintf(f,"\"></TD></TR>\r\n");

#ifdef CS_ANTICASC
	fprintf(f,"<TR><TD>Anticascading numusers:</TD><TD><input name=\"numusers\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"%d\"></TD></TR>\r\n", account->ac_users);
	fprintf(f,"<TR><TD>Anticascading penalty:</TD><TD><input name=\"penalty\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"%d\"></TD></TR>\r\n", account->ac_penalty);
#endif
	fprintf(f,"<TR><TD>&nbsp;</TD><TD align=\"right\"><input type=\"submit\" value=\"Save Settings\" title=\"Save settings and reload users\"></TD></TR>\r\n");
	fprintf(f,"</TABLE>\r\n");
	fprintf(f,"</form>\r\n");
}

void send_oscam_entitlement(FILE *f, struct uriparams *params) {

  /*build entitlements from reader init history*/
	int ridx;
	char *p, *ctyp;

	if((*params).paramcount==0){
		fprintf(f, "<BR><BR>\r\n");
		fprintf(f,"<TABLE cellspacing=\"0\">");
		fprintf(f,"<TR><TH>Reader</TH><TH>Protocol</TH><TH>&nbsp;</TH></TR>");

		for (ridx=0; ridx<CS_MAXREADER; ridx++){

			switch(reader[ridx].typ)
			{
				case R_MOUSE   : ctyp="mouse";    break;
				case R_INTERNAL: ctyp="intern";   break;
				case R_SMART   : ctyp="smartreader";    break;
				case R_CAMD35  : ctyp="camd 3.5x";break;
				case R_CAMD33  : ctyp="camd 3.3x";break;
				case R_NEWCAMD : ctyp="newcamd";  break;
				case R_RADEGAST: ctyp="radegast"; break;
				case R_SERIAL  : ctyp="serial";   break;
				case R_GBOX    : ctyp="gbox";     break;
#ifdef HAVE_PCSC
				case R_PCSC    : ctyp="pcsc";     break;
#endif
				case R_CCCAM   : ctyp="cccam";    break;
				case R_CS378X  : ctyp="cs378x";   break;
				default        : ctyp="unknown";  break;
			}

			if(!reader[ridx].device[0]) break;
			fprintf(f, "\t<TR><TD>%s</TD><TD>%s</TD><TD><A HREF=\"entitlements.html?user=%s\">Check</A></TD></TR>\r\n", reader[ridx].label, ctyp, reader[ridx].label);
		}
		fprintf(f, "</TABLE>\r\n");
	}
	else {

		char *user = getParam(params, "user");

		fprintf(f, "<BR><BR>Entitlement for %s<BR><BR>\r\n", user);

		for (ridx=0; ridx<CS_MAXREADER; ridx++)
			if (!strcmp(user, reader[ridx].label)) break;

		fprintf(f, "<DIV class=\"log\">");
#ifdef CS_RDR_INIT_HIST

		for (p=(char *)reader[ridx].init_history; *p; p+=strlen(p)+1)
			fprintf(f, "%s<BR>\n",p);

#else

		fprintf(f, "the flag CS_RDR_INIT_HIST is not set in your binary<BR>\n");

#endif

		fprintf(f, "</DIV>");
	}

}

void monitor_client_status(struct templatevars *vars, char id, int i){
	if (client[i].pid) {
		char *usr;
		int lsec, isec, cnr, con, cau;
		time_t now = time((time_t)0);
		struct tm *lt;
		
		if ((cfg->mon_hideclient_to <= 0) ||	(((now-client[i].lastecm)/60)<cfg->mon_hideclient_to) ||
		(((now-client[i].lastemm)/60)<cfg->mon_hideclient_to) || (client[i].typ!='c')){
			lsec=now-client[i].login;
			isec=now-client[i].last;
			usr=client[i].usr;
			
			if (((client[i].typ=='r') || (client[i].typ=='p')) && (con=cs_idx2ridx(i))>=0) usr=reader[con].label;
			
			if (client[i].dup) con=2;
			else {
				if ((client[i].tosleep) && (now-client[i].lastswitch>client[i].tosleep)) con=1;
				else con=0;
			}
			
			if (i-cdiff>0) cnr=i-cdiff;
			else cnr=(i>1) ? i-1 : 0;
			
			if( (cau=client[i].au+1) && (now-client[i].lastemm)/60 > cfg->mon_aulow) cau=-cau;
			
			lt=localtime(&client[i].login);
			
			tpl_printf(vars, 0, "CLIENTPID", "%d", client[i].pid); 
			tpl_printf(vars, 0, "CLIENTTYPE", "%c", client[i].typ);
			tpl_printf(vars, 0, "CLIENTCNR", "%d", cnr);
			tpl_addVar(vars, 0, "CLIENTUSER", usr);
			tpl_printf(vars, 0, "CLIENTCAU", "%d", cau);
			tpl_printf(vars, 0, "CLIENTCRYPTED", "%d", client[i].crypted);
			tpl_printf(vars, 0, "CLIENTIP", "%s", cs_inet_ntoa(client[i].ip));
			tpl_printf(vars, 0, "CLIENTPORT", "%d", client[i].port);
			tpl_addVar(vars, 0, "CLIENTPROTO", monitor_get_proto(i));
			tpl_printf(vars, 0, "CLIENTLOGINDATE", "%02d.%02d.%02d", lt->tm_mday, lt->tm_mon+1, lt->tm_year%100);
			tpl_printf(vars, 0, "CLIENTLOGINTIME", "%02d:%02d:%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
			tpl_printf(vars, 0, "CLIENTLOGINSECS", "%d", lsec);
			tpl_printf(vars, 0, "CLIENTCAID", "%04X", client[i].last_caid);
			tpl_printf(vars, 0, "CLIENTSRVID", "%04X", client[i].last_srvid);
			tpl_addVar(vars, 0, "CLIENTSRVNAME", monitor_get_srvname(client[i].last_srvid));
			tpl_printf(vars, 0, "CLIENTIDLESECS", "%d", isec);
			tpl_printf(vars, 0, "CLIENTCON", "%d", con); 
			tpl_addVar(vars, 1, "CLIENTSTATUS", tpl_getTpl(vars, "CLIENTSTATUSBIT"));
		}
	}
}

void send_oscam_status(struct templatevars *vars, FILE *f) {
	int i;	
	for (i=0; i<CS_MAXPID; i++)	monitor_client_status(vars, client[i].pid, i);
	
#ifdef CS_LOGHISTORY
	for (i=(*loghistidx+3) % CS_MAXLOGHIST; i!=*loghistidx; i=(i+1) % CS_MAXLOGHIST){
		char *p_usr, *p_txt;
		p_usr=(char *)(loghist+(i*CS_LOGHISTSIZE));
		p_txt=p_usr+32;
		if (p_txt[0]) tpl_printf(vars, 1, "LOGHISTORY", "%s<BR>\n", p_txt+8);
	}
#else
	tpl_addVar(vars, 0, "LOGHISTORY", "the flag CS_LOGHISTORY is not set in your binary<BR>\n");
#endif

	fputs(tpl_getTpl(vars, "STATUS"), f);
}

void send_oscam_sidtab(FILE *f) {
  struct s_sidtab *sidtab = cfg->sidtab;

	fprintf(f,"<BR><BR><DIV class=\"log\">");

  for (; sidtab; sidtab=sidtab->next)
  {
    int i, comma;
    char buf[1024];
    fprintf(f,"label=%s<BR>\r\n", sidtab->label);
    sprintf(buf, "caid(%d)=", sidtab->num_caid);
    comma=0;
    for (i=0; i<sidtab->num_caid; i++){
			if (comma==0){
				sprintf(buf+strlen(buf), "%04X",sidtab->caid[i]);
				comma++;
			}
			else
				sprintf(buf+strlen(buf), ",%04X",sidtab->caid[i]);
    }

    fprintf(f,"%s<BR>\r\n", buf);
    sprintf(buf, "provider(%d)=", sidtab->num_provid);
    comma=0;
    for (i=0; i<sidtab->num_provid; i++){
			if (comma==0){
				sprintf(buf+strlen(buf), "%ld08X", sidtab->provid[i]);
				comma++;
			}
			else
				sprintf(buf+strlen(buf), ",%ld08X", sidtab->provid[i]);
    }

    fprintf(f,"%s<BR>\r\n", buf);
    sprintf(buf, "services(%d)=", sidtab->num_srvid);
    comma=0;
    for (i=0; i<sidtab->num_srvid; i++){
    	if (comma==0){
				sprintf(buf+strlen(buf), "%04X", sidtab->srvid[i]);
				comma++;
			}
			else
				sprintf(buf+strlen(buf), ",%04X", sidtab->srvid[i]);
		}
    fprintf(f,"%s<BR><BR>\r\n", buf);
  }
  fprintf(f,"</DIV>");
}

int process_request(FILE *f) {
  char buf[4096];
  char tmp[4096];

  int authok = 0;
  char expectednonce[64];

  char *method;
  char *path;
  char *protocol;
  char *pch;
  char *pch2;
  /*list of possible pages*/
  char *pages[]={	"/config.html",
			"/readers.html",
			"/entitlements.html",
			"/status.html",
			"/userconfig.html",
			"/readerconfig.html",
			"/readerconfig_do.html",
			"/services.html",
			"/site.css"};
  int pagescnt = sizeof(pages)/sizeof(char *);  // Calculate the amount of items in array
  
  int pgidx = -1;
  int i;
  int parsemode = 1;
  struct uriparams params;
  params.paramcount = 0;  

  /* First line always includes the GET/POST request */
  if (!fgets(buf, sizeof(buf), f)) return -1;
  method = strtok(buf, " ");
  path = strtok(NULL, " ");
  protocol = strtok(NULL, "\r");
  /* Throw away references to anchors (these are dealt with by the browser and not the server!) */
  path = strtok(path, "#");
  if(method == NULL || path == NULL || protocol == NULL) return -1;

  pch=path;
  /* advance pointer to beginning of query string */
  while(pch[0] != '?' && pch[0] != '\0') ++pch;
  if(pch[0] == '?'){
    pch[0] = '\0';
    ++pch;
  }

  /* Map page to our static page definitions */
  for (i=0; i<pagescnt; i++){
    if (!strcmp(path, pages[i])) pgidx = i;
  }

  /* Parse url parameters; parsemode = 1 means parsing next param, parsemode = -1 parsing next
     value; pch2 points to the beginning of the currently parsed string, pch is the current position */
  pch2=pch;
  while(pch[0] != '\0'){
    if((parsemode == 1 && pch[0] == '=') || (parsemode == -1 && pch[0] == '&')){
      pch[0] = '\0';
      urldecode(pch2);
      if(parsemode == 1) {
        if(params.paramcount >= MAXGETPARAMS) break;
        ++params.paramcount;
        params.params[params.paramcount-1] = pch2;
      } else {
        params.values[params.paramcount-1] = pch2;
      }
      parsemode = -parsemode;
      pch2 = pch + 1;
    }
    ++pch;
  }
  /* last value wasn't processed in the loop yet... */
  if(parsemode == -1 && params.paramcount <= MAXGETPARAMS){
      params.values[params.paramcount-1] = pch2;
  }

	if(strlen(cfg->http_user) == 0 || strlen(cfg->http_pwd) == 0) authok = 1;
	else calculate_nonce(expectednonce, sizeof(expectednonce)/sizeof(char));

	/* Read remaining request (we're only interested in auth header) */
  while (fgets(tmp, sizeof(tmp), f))  {
    if (tmp[0] == '\r' && tmp[1] == '\n') break;
    else if(authok == 0 && strlen(tmp) > 50 && strncmp(tmp, "Authorization:", 14) == 0 && strstr(tmp, "Digest") != NULL) {
    	authok = check_auth(tmp, method, path, expectednonce);
    }
  }

  //cs_debug("%s %d\n", path, pgidx);
  //for(i=0; i < params.paramcount; ++i) cs_debug("%s : %s\n", params.params[i], params.values[i]);

  fseek(f, 0, SEEK_CUR); // Force change of stream direction

  if(authok != 1){
  	strcpy(tmp, "WWW-Authenticate: Digest algorithm=\"MD5\", realm=\"");
  	strcat(tmp, AUTHREALM);
  	strcat(tmp, "\", qop=\"auth\", opaque=\"\", nonce=\"");
  	strcat(tmp, expectednonce);
  	strcat(tmp, "\"");
  	if(authok == 2) strcat(tmp, ", stale=true");
	  send_headers(f, 401, "Unauthorized", tmp, "text/html");
	  return -1;
	}

	/*build page*/
  send_headers(f, 200, "OK", NULL, "text/html");
	if(pgidx == -1 || pgidx == 3){
		time_t t;
		struct templatevars *vars = tpl_create();
		struct tm *lt;
		time(&t);
	
		lt=localtime(&t);	
		tpl_addVar(vars, 0, "CS_VERSION", CS_VERSION);
		tpl_addVar(vars, 0, "CS_SVN_VERSION", CS_SVN_VERSION);
		tpl_printf(vars, 0, "REFRESHTIME", "%d", cfg->http_refresh);
		tpl_printf(vars, 0, "CURDATE", "%02d.%02d.%02d", lt->tm_mday, lt->tm_mon+1, lt->tm_year%100);
		tpl_printf(vars, 0, "CURTIME", "%02d:%02d:%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
		send_oscam_status(vars, f);
		tpl_clear(vars);
	} else {
  if(pgidx == 8) send_css(f);
	else {

		if (pgidx == 3)	send_htmlhead(f, cfg->http_refresh); //status
		else send_htmlhead(f,0);

  send_oscam_menu(f);
  switch(pgidx){
    case  0: send_oscam_config(f, &params); break;
    case  1: send_oscam_reader(f); break;
    case  2: send_oscam_entitlement(f, &params); break;
    case  3: break;
    case  4: send_oscam_user_config(f, &params); break;
    case  5: send_oscam_reader_config(f, &params); break;
    case  6: send_oscam_reader_config_do(f, &params); break;
    case	7: send_oscam_sidtab(f); break;
    default: break;
  }

  send_footer(f);
  fprintf(f, "</BODY></HTML>\r\n");
		}
	}
  return 0;
}

void http_srv() {
	int i,sock;
	struct sockaddr_in sin;
	char *tmp;

	/* Prepare lookup array for conversion between ascii and hex */
	tmp = malloc(3*sizeof(char));
	for(i=0; i<256; i++) {
		snprintf(tmp, 3,"%02x", i);
		memcpy(hex2ascii[i], tmp, 2);
	}
	free(tmp);
	/* Create random string for nonce value generation */
	srand(time(NULL));
	create_rand_str(noncekey,32);

	/* Startup server */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cs_log("HTTP Server: Creating socket failed! (errno=%d)", errno);
		return;
	}
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(cfg->http_port);
	if((bind(sock, (struct sockaddr *) &sin, sizeof(sin))) < 0) {
		cs_log("HTTP Server couldn't bind on port %d (errno=%d). Not starting HTTP!", cfg->http_port, errno);
		close(sock);
		return;
	}
	if (listen(sock, 5) < 0){
		cs_log("HTTP Server: Call to listen() failed! (errno=%d)", errno);
		close(sock);
		return;
	}
	cs_log("HTTP Server listening on port %d", cfg->http_port);
	while (1)
	{
		int s;
		FILE *f;
		if((s = accept(sock, NULL, NULL)) < 0){
			cs_log("HTTP Server: Error calling accept() (errno=%d).", errno);
			break;
		}

		f = fdopen(s, "r+");
		process_request(f);
		fflush(f);
		fclose(f);
		shutdown(s, 2);
		close(s);
  }
  close(sock);
}
