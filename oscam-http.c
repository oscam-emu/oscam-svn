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
			"4 = complete access"
			};

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

void send_oscam_reader(struct templatevars *vars, FILE *f) {
	int ridx;
	char *ctyp;

	for(ridx=0;ridx<CS_MAXREADER;ridx++){
		if(!reader[ridx].device[0]) break;
		switch(reader[ridx].typ){
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
		tpl_addVar(vars, 0, "CTYP", ctyp);
		tpl_addVar(vars, 0, "READERNAME", reader[ridx].label);
		tpl_addVar(vars, 0, "READERNAMEENC", tpl_addTmp(vars, urlencode(reader[ridx].label)));
		tpl_addVar(vars, 1, "READERLIST", tpl_getTpl(vars, "READERSBIT"));
	}
	fputs(tpl_getTpl(vars, "READERS"), f);
}

void send_oscam_reader_config(struct templatevars *vars, FILE *f, struct uriparams *params) {
	int ridx;
	char *reader_ = getParam(params, "reader");
	for(ridx = 0; ridx < CS_MAXREADER && strcmp(reader_, reader[ridx].label) != 0; ++ridx);
	if(ridx == CS_MAXREADER){
		tpl_addVar(vars, 0, "MESSAGE", "<BR><BR>Reader not found<BR><BR>");
	} else if(strcmp(getParam(params, "action"), "execute") == 0){
		tpl_addVar(vars, 0, "MESSAGE", "<BR><BR>Saving not yet implemented<BR><BR>");
		refresh_oscam(REFR_READERS);
	}
	int i;
	tpl_addVar(vars, 0, "READERNAME", reader[ridx].label);
	tpl_addVar(vars, 0, "DEVICE", reader[ridx].device);
	tpl_addVar(vars, 0, "NCD_KEY", (char *)reader[ridx].ncd_key);
	tpl_addVar(vars, 0, "PINCODE", reader[ridx].pincode);
	tpl_addVar(vars, 0, "EMMFILE", (char *)reader[ridx].emmfile);
	tpl_addVar(vars, 0, "GBOXPWD", (char *)reader[ridx].gbox_pwd);
	tpl_printf(vars, 0, "INACTIVITYTIMEOUT", "%d", reader[ridx].tcp_ito);
	tpl_printf(vars, 0, "RECEIVETIMEOUT", "%d", reader[ridx].tcp_rto);
	tpl_printf(vars, 0, "DISABLESERVERFILTER", "%d", reader[ridx].ncd_disable_server_filt);
	tpl_printf(vars, 0, "FALLBACK", "%d", reader[ridx].fallback);
	tpl_printf(vars, 0, "LOGPORT", "%d", reader[ridx].log_port);
	tpl_printf(vars, 0, "BOXID", "%ld", reader[ridx].boxid);

	if(reader[ridx].r_port) tpl_printf(vars, 0, "R_PORT", "%d", reader[ridx].r_port);
	if(reader[ridx].l_port) tpl_printf(vars, 0, "L_PORT", "%d", reader[ridx].l_port);

	//Group
	/*restore the settings format of group from long over bitarray*/
	char *dot = ""; //flag for comma
	char grpbit[33];
	long2bitchar(reader[ridx].grp, grpbit);
	for(i = 0; i < 32; i++){
		if (grpbit[i] == '1'){
			tpl_printf(vars, 1, "GRP", "%s%d", dot, i+1);
				dot = ",";
			}
		}

	//services
	char sidok[33];
	long2bitchar(reader[ridx].sidtabok, sidok);
	char sidno[33];
	long2bitchar(reader[ridx].sidtabno,sidno);
	struct s_sidtab *sidtab = cfg->sidtab;
	//build matrix
	while(sidtab != NULL){
		tpl_addVar(vars, 0, "SIDLABEL", sidtab->label);
		if(sidok[i]=='1') tpl_addVar(vars, 0, "CHECKED", "checked");
		else tpl_addVar(vars, 0, "CHECKED", "");
		tpl_addVar(vars, 1, "SIDS", tpl_getTpl(vars, "READERCONFIGSIDOKBIT"));
		if(sidno[i]=='1') tpl_addVar(vars, 0, "CHECKED", "checked");
		else tpl_addVar(vars, 0, "CHECKED", "");
		tpl_addVar(vars, 1, "SIDS", tpl_getTpl(vars, "READERCONFIGSIDNOBIT"));
		sidtab=sidtab->next;
	}

	// CAID
	i = 0;
	CAIDTAB *ctab = &reader[ridx].ctab;
		while(ctab->caid[i]) {
		if (i == 0) tpl_printf(vars, 1, "CAIDS", "%04X", ctab->caid[i]);
		else tpl_printf(vars, 1, "CAIDS", ",%04X", ctab->caid[i]);;
		if(ctab->mask[i])	tpl_printf(vars, 1, "CAIDS", "&%04X", ctab->mask[i]);
			i++;
		}
	fputs(tpl_getTpl(vars, "READERCONFIG"), f);
	}

void send_oscam_user_config_edit(struct templatevars *vars, FILE *f, struct uriparams *params){
	struct s_auth *account, *ptr;
	char *user = getParam(params, "user");
	int i, j;

	for (account = cfg->account; account != NULL && strcmp(user, account->usr) != 0; account = account->next);

	// Create a new user if it doesn't yet
	if (account == NULL){
	  if (!(account=malloc(sizeof(struct s_auth)))){
        cs_log("Error allocating memory (errno=%d)", errno);
        return;
      }
	  if(cfg->account == NULL) cfg->account = account;
	  else {
	  	for (ptr = cfg->account; ptr != NULL && ptr->next != NULL; ptr = ptr->next);
	  	ptr->next = account;
	  }
      memset(account, 0, sizeof(struct s_auth));
	  strncpy((char *)account->usr, user, sizeof(account->usr)-1);
      account->au=(-1);
      account->monlvl=cfg->mon_level;
      account->tosleep=cfg->tosleep;
      for (i=1; i<CS_MAXCAIDTAB; account->ctab.mask[i++]=0xffff);
      for (i=1; i<CS_MAXTUNTAB; account->ttab.bt_srvid[i++]=0x0000);
#ifdef CS_ANTICASC
      account->ac_users=cfg->ac_users;
      account->ac_penalty=cfg->ac_penalty;
	  account->ac_idx = account->ac_idx + 1;
#endif
		tpl_addVar(vars, 1, "MESSAGE", "<b>New user has been added with default settings</b><BR>");
		if (write_userdb()==0) refresh_oscam(REFR_ACCOUNTS);
		else tpl_addVar(vars, 1, "MESSAGE", "<b>Writing configuration to disk failed!</b><BR>"); 
		// need to reget account as writing to disk changes account!
		for (account = cfg->account; account != NULL && strcmp(user, account->usr) != 0; account = account->next);
    }

	if(strcmp(getParam(params, "action"), "execute") == 0){
	char servicelabels[255]="";
	//clear group
	account->grp = 0;

	for(i=0;i<(*params).paramcount;i++){
		if ((strcmp((*params).params[i], "action")) && (strcmp((*params).params[i], "user"))){
			if (!strcmp((*params).params[i], "services"))
				sprintf(servicelabels + strlen(servicelabels), "%s,", (*params).values[i]);
			else
				chk_account((*params).params[i], (*params).values[i], account);
		}
	}
	chk_account("services", servicelabels, account);
		tpl_addVar(vars, 1, "MESSAGE", "<B>Settings updated</B><BR><BR>");	
		if (write_userdb()==0) refresh_oscam(REFR_ACCOUNTS);
		else tpl_addVar(vars, 1, "MESSAGE", "<B>Write Config failed</B><BR><BR>");
	}

	tpl_addVar(vars, 0, "USERNAME", account->usr);
	tpl_addVar(vars, 0, "PASSWORD", account->pwd);	

	//Expirationdate
	struct tm * timeinfo = localtime (&account->expirationdate);
	char buf [80];
	strftime (buf,80,"%Y-%m-%d",timeinfo);
	if(strcmp(buf,"1970-01-01")) tpl_addVar(vars, 0, "EXPDATE", buf);
	//Group
	/*restore the settings format of group from long over bitarray*/
	char *dot = ""; //flag for comma
	char grpbit[33];
	long2bitchar(account->grp, grpbit);
	for(i = 0; i < 32; i++){
		if (grpbit[i] == '1'){
				tpl_printf(vars, 1, "GROUPS", "%s%d", dot, i+1);
				dot = ",";
			}
	}
	//Hostname
	tpl_addVar(vars, 0, "DYNDNS", (char *)account->dyndns);
	
	//Uniq
	tpl_printf(vars, 0, "TMP", "UNIQSELECTED%d", account->uniq);
	tpl_addVar(vars, 0, tpl_getVar(vars, "TMP"), "selected");
	
	//Sleep
	if(!account->tosleep) tpl_addVar(vars, 0, "SLEEP", "0");
	else tpl_printf(vars, 0, "SLEEP", "%d", account->tosleep);
	//Monlevel selector
	tpl_printf(vars, 0, "TMP", "MONSELECTED%d", account->monlvl);
	tpl_addVar(vars, 0, tpl_getVar(vars, "TMP"), "selected");

	//AU Selector
	if (!account->au) tpl_addVar(vars, 0, "AUSELECTED", "selected");
	if (account->autoau == 1) tpl_addVar(vars, 0, "AUTOAUSELECTED", "selected");
	int ridx;
	for (ridx=0; ridx<CS_MAXREADER; ridx++){
		if(!reader[ridx].device[0]) break;
		tpl_addVar(vars, 0, "READERNAME", reader[ridx].label);
		if (account->au == ridx) tpl_addVar(vars, 0, "SELECTED", "selected");
		else tpl_addVar(vars, 0, "SELECTED", "");
		tpl_addVar(vars, 1, "RDROPTION", tpl_getTpl(vars, "USEREDITRDRSELECTED"));
	}

	/* SERVICES */
	//services - first we have to move the long sidtabok/sidtabno to a binary array
	char sidok[33];
	long2bitchar(account->sidtabok,sidok);
	char sidno[33];
	long2bitchar(account->sidtabno,sidno);
	struct s_sidtab *sidtab = cfg->sidtab;
	//build matrix
	while(sidtab != NULL){
		tpl_addVar(vars, 0, "SIDLABEL", sidtab->label);
		if(sidok[i]=='1') tpl_addVar(vars, 0, "CHECKED", "checked");
		else tpl_addVar(vars, 0, "CHECKED", "");
		tpl_addVar(vars, 1, "SIDS", tpl_getTpl(vars, "USEREDITSIDOKBIT"));
		if(sidno[i]=='1') tpl_addVar(vars, 0, "CHECKED", "checked");
		else tpl_addVar(vars, 0, "CHECKED", "");
		tpl_addVar(vars, 1, "SIDS", tpl_getTpl(vars, "USEREDITSIDNOBIT"));
		sidtab=sidtab->next;
	}
	/* CAID */
	dot="";
	i = 0;
	CAIDTAB *ctab = &account->ctab;
		while(ctab->caid[i]) {
		tpl_printf(vars, 1, "CAIDS", "%s%04X", dot, ctab->caid[i]);
		if(ctab->mask[i])	tpl_printf(vars, 1, "CAIDS", "&%04X", ctab->mask[i]);
		if(ctab->cmap[i])	tpl_printf(vars, 1, "CAIDS", ":%04X", ctab->cmap[i]);
		dot=",";
		++i;
	}

	/*IDENT*/
	dot="";
	FTAB *ftab = &account->ftab;
	for (i = 0; i < ftab->nfilts; ++i){
		tpl_printf(vars, 1, "IDENTS", "%s%04X", dot, ftab->filts[i].caid);
		dot=":";
		for (j = 0; j < ftab->filts[i].nprids; ++j) {
			tpl_printf(vars, 1, "IDENTS", "%s%06lX", dot, ftab->filts[i].prids[j]);
			dot=",";
		}
		dot=";";
	}

	/*Betatunnel*/
	dot="";
	i = 0;
	TUNTAB *ttab = &account->ttab;
		while(ttab->bt_caidfrom[i]) {
		tpl_printf(vars, 1, "BETATUNNELS", "%s%04X", dot, ttab->bt_caidfrom[i]);
		if(ttab->bt_srvid[i])	tpl_printf(vars, 1, "BETATUNNELS", ".%04X", ttab->bt_srvid[i]);
		if(ttab->bt_caidto[i]) tpl_printf(vars, 1, "BETATUNNELS", ":%04X", ttab->bt_caidto[i]);
		dot=",";
		++i;
	}

#ifdef CS_ANTICASC
	tpl_printf(vars, 0, "AC_USERS", "%d", account->ac_users);
	tpl_printf(vars, 0, "AC_PENALTY", "%d", account->ac_penalty);
#endif
	fputs(tpl_getTpl(vars, "USEREDIT"), f);
}

void send_oscam_user_config(struct templatevars *vars, FILE *f, struct uriparams *params) {
	struct s_auth *account, *account2;	
	char *user = getParam(params, "user");
	int i, found = 0;

	if (strcmp(getParam(params, "action"), "delete") == 0){
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

		if (found > 0){
			tpl_addVar(vars, 1, "MESSAGE", "<b>Account has been deleted!</b><BR>");
			if (write_userdb()==0) refresh_oscam(REFR_ACCOUNTS);
			else tpl_addVar(vars, 1, "MESSAGE", "<b>Writing configuration to disk failed!</b><BR>");
		} else tpl_addVar(vars, 1, "MESSAGE", "<b>Sorry but the specified user doesn't exist. No deletion will be made!</b><BR>");
	}
	/* List accounts*/
	char *status = "offline";
	char *expired = "";
	char *classname="offline";
	char *lastchan="&nbsp;";
	time_t now = time((time_t)0);
	int isec=0;

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
		tpl_addVar(vars, 0, "CLASSNAME", classname);
		tpl_addVar(vars, 0, "USER", account->usr);
		tpl_addVar(vars, 0, "USERENC", tpl_addTmp(vars, urlencode(account->usr)));
		tpl_addVar(vars, 0, "STATUS", status);
		tpl_addVar(vars, 0, "EXPIRED", expired);
		tpl_addVar(vars, 0, "LASTCHANNEL", lastchan);
		tpl_printf(vars, 0, "IDLESECS", "%d", isec);
		tpl_addVar(vars, 1, "USERCONFIGS", tpl_getTpl(vars, "USERCONFIGLISTBIT"));
		isec = 0;
		lastchan = "&nbsp;";
	}
	fputs(tpl_getTpl(vars, "USERCONFIGLIST"), f);
}

void send_oscam_entitlement(struct templatevars *vars, FILE *f, struct uriparams *params) {
  /* build entitlements from reader init history */
	int ridx;
	char *p;
	char *reader_ = getParam(params, "reader");
	if(strlen(reader_) > 0){	
#ifdef CS_RDR_INIT_HIST
		for (ridx=0; ridx<CS_MAXREADER && strcmp(reader_, reader[ridx].label) != 0; ridx++);
		if(ridx<CS_MAXREADER){
			for (p=(char *)reader[ridx].init_history; *p; p+=strlen(p)+1){
				tpl_printf(vars, 1, "LOGHISTORY", "%s<BR>\n", p);
			}
		}
#else
		tpl_addVar(vars, 0, "LOGHISTORY", "The flag CS_RDR_INIT_HIST is not set in your binary<BR>\n");
#endif
		tpl_addVar(vars, 0, "READERNAME", reader_);
	}
	fputs(tpl_getTpl(vars, "ENTITLEMENTS"), f);
}

void send_oscam_status(struct templatevars *vars, FILE *f) {
	int i;
		char *usr;
		int lsec, isec, cnr, con, cau;
		time_t now = time((time_t)0);
		struct tm *lt;
	for (i=0; i<CS_MAXPID; i++)	{
		if (client[i].pid) {			
		if ((cfg->mon_hideclient_to <= 0) ||	(((now-client[i].lastecm)/60)<cfg->mon_hideclient_to) ||
		(((now-client[i].lastemm)/60)<cfg->mon_hideclient_to) || (client[i].typ!='c')){
			lsec=now-client[i].login;
			isec=now-client[i].last;
			usr=client[i].usr;
			
			if (((client[i].typ=='r') || (client[i].typ=='p')) && (con=cs_idx2ridx(i))>=0) usr=reader[con].label;
			
			if (client[i].dup) con=2;
				else if ((client[i].tosleep) && (now-client[i].lastswitch>client[i].tosleep)) con=1;
				else con=0;
			
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

void send_oscam_services(struct templatevars *vars, FILE *f) {
  struct s_sidtab *sidtab = cfg->sidtab;
	int i;	

	while(sidtab != NULL){	
    for (i=0; i<sidtab->num_caid; i++){
			if (i==0) tpl_printf(vars, 0, "CAIDS", "%04X", sidtab->caid[i]);
			else tpl_printf(vars, 1, "CAIDS", ",%04X", sidtab->caid[i]);
    }

    for (i=0; i<sidtab->num_provid; i++){
			if (i==0) tpl_printf(vars, 0, "PROVIDS", "%ld08X", sidtab->provid[i]);
			else tpl_printf(vars, 1, "PROVIDS", ",%ld08X", sidtab->provid[i]);
    }

    for (i=0; i<sidtab->num_srvid; i++){
			if (i==0) tpl_printf(vars, 0, "SRVIDS", "%04X", sidtab->srvid[i]);
			else tpl_printf(vars, 1, "SRVIDS", ",%04X", sidtab->srvid[i]);
		}
		tpl_addVar(vars, 0, "LABEL", sidtab->label);
		tpl_printf(vars, 0, "CAIDNUM", "%d", sidtab->num_caid);
		tpl_printf(vars, 0, "PROVIDNUM", "%d",sidtab->num_provid);
		tpl_printf(vars, 0, "SRVIDNUM", "%d", sidtab->num_srvid);
		tpl_addVar(vars, 1, "SIDTABS", tpl_getTpl(vars, "SIDTABBIT"));
		sidtab=sidtab->next;
  }
	fputs(tpl_getTpl(vars, "SIDTAB"), f);
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
  /* List of possible pages */
  char *pages[]={	"/config.html",
			"/readers.html",
			"/entitlements.html",
			"/status.html",
			"/userconfig.html",
			"/readerconfig.html",
			"/services.html",
			"/user_edit.html",
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
      urldecode(pch2);
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
	  return 0;
	}

	/*build page*/
  send_headers(f, 200, "OK", NULL, "text/html");
  if(pgidx == 8) send_css(f);
	else if(pgidx != 0){
		time_t t;
		struct templatevars *vars = tpl_create();
		struct tm *lt;
		time(&t);
	
		lt=localtime(&t);	
		tpl_addVar(vars, 0, "CS_VERSION", CS_VERSION);
		tpl_addVar(vars, 0, "CS_SVN_VERSION", CS_SVN_VERSION);
		if(cfg->http_refresh > 0 && (pgidx == 3 || pgidx == -1)){
		tpl_printf(vars, 0, "REFRESHTIME", "%d", cfg->http_refresh);
			tpl_addVar(vars, 0, "REFRESH", tpl_getTpl(vars, "REFRESH"));
		}
		tpl_printf(vars, 0, "CURDATE", "%02d.%02d.%02d", lt->tm_mday, lt->tm_mon+1, lt->tm_year%100);
		tpl_printf(vars, 0, "CURTIME", "%02d:%02d:%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
		switch(pgidx){
	    case  0: break;
	    case  1: send_oscam_reader(vars, f); break;
	    case  2: send_oscam_entitlement(vars, f, &params); break;
	    case  3: send_oscam_status(vars, f); break;
	    case  4: send_oscam_user_config(vars, f, &params); break;
	    case  5: send_oscam_reader_config(vars, f, &params); break;
	    case	6: send_oscam_services(vars, f); break;
	    case  7: send_oscam_user_config_edit(vars, f, &params); break;
	    default: send_oscam_status(vars, f); break;
	  }
		tpl_clear(vars);
	} else {
	 	send_htmlhead(f,0);
  send_oscam_menu(f);
	  send_oscam_config(f, &params);
  send_footer(f);
  fprintf(f, "</BODY></HTML>\r\n");
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
