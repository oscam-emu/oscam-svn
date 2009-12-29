#include "globals.h"
//
// webserver.c
//
// Simple HTTP server
//

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

#define SERVER "webserver/1.0"
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define AUTHREALM "OScam"
#define AUTHNONCEVALIDSECS 5

void send_oscam_user_config(FILE *f, char *uriparams[], char *urivalues[], int paramcount);

enum refreshtypes {REFR_ACCOUNTS, REFR_READERS, REFR_SERVER, REFR_ANTICASC};

static  char*   css[] = {
			"p {color: white; }",
			"h2 {color: orange; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 20px; line-height: 20px;}",
			"h4 {color: black; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px; line-height: 12px; }",
			"TABLE{background-color:#66CCFF;}",
			"TD {border:1px solid gray; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px;padding:5px;background-color:#6666FF;}",
			"TH {border:1px solid gray; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px;padding:5px;background-color:#6699FF}",
			"DIV.log{border:1px solid black;background-color: black; font-family: Courier, \"Courier New\", monospace ; color: white;}",
			"TABLE.menu{background-color:black;align:center;}",
			"TABLE.menu TD{border:2px outset lightgrey;background-color:silver;font-color:black; font-family: Verdana, Arial, Helvetica, sans-serif;}",
			"TABLE.status{background-color:#66CCFF;empty-cells:show;}",
			"TABLE.invisible TD {border:0px; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px;padding:5px;background-color:#6666FF;}}",
			"TD.menu {border:2px outset lightgrey;background-color:silver;font-color:black; font-family: Verdana, Arial, Helvetica, sans-serif;}",
			"body {background-color: #FFFF66;font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px;}",
			"A:link {text-decoration: none; color:blue}",
			"A:visited {text-decoration: none; color:blue}",
			"A:active {text-decoration: none; color:white}",
			"A:hover {text-decoration: none; color: red;}",
				NULL };

static char* monlevel[] = {
			"0 = no access to monitor",
			"1 = only server and own procs",
			"2 = all procs, but viewing only, default",
			"3 = all procs, reload of oscam.user possible",
			"4 = complete access",
			NULL };

static char* uniq[] = {
			"0 = none",
			"1 = strict",
			"2 = per IP",
			NULL };

static char hex2ascii[256][2];
static char noncekey[33];

/* Parses a value in an authentication string by removing all quotes/whitespace. Note that the original array is modified*/
char *parse_auth_value(char *value){
	char *pch = value;
	char *pch2;
	value = strstr(value, "=");
	if(value != NULL){
		do{
			++value;
		} while (value[0] == ' ' || value[0] == '"');
		pch = value;
		for(pch2 = value + strlen(value) - 1; pch2 >= value && (pch2[0] == ' ' || pch2[0] == '"' || pch2[0] == '\r' || pch2[0] == '\n'); --pch2) pch2[0] = '\0';
	}
	return pch;
}

/* Calculates the currently valid nonce value and copies it to result*/
void calculate_nonce(char *result, int resultlen){
	char *expectednonce, *noncetmp;
  noncetmp = (char*) malloc (128*sizeof(char));
  sprintf(noncetmp, "%d", (int)time(NULL)/AUTHNONCEVALIDSECS);
  strcat(noncetmp, ":");
  strcat(noncetmp, noncekey);
  fflush(stdout);
  expectednonce =char_to_hex(MD5((unsigned char*)noncetmp, strlen(noncetmp), NULL), MD5_DIGEST_LENGTH, hex2ascii);
  strncpy(result, expectednonce, resultlen);
  result[resultlen - 1] = '\0';
  free(noncetmp);
	free(expectednonce);
}

/* Checks if authentication is correct. Returns -1 if not correct, 1 if correct and 2 if nonce isn't valid anymore */
int check_auth(char *authstring, char *method, char *path, char *expectednonce){
	int authok = 0;
	char emptystr[]="";
	char *authnonce;
	char *authnc;
	char *authcnonce;
	char *uri;
	char *authresponse;
	char *A1tmp, *A2tmp, *A3tmp;
	char *A1, *A2, *A3;
  char *pch, *pch2;

	authnonce = emptystr;
	authnc = emptystr;
	authcnonce = emptystr;
	authresponse = emptystr;
	uri = emptystr;
	pch = authstring + 22;
	pch = strtok (pch,",");
	while (pch != NULL){
		pch2 = pch;
	  while(pch2[0] == ' ' && pch2[0] != '\0') ++pch2;
	  if(strncmp(pch2, "nonce", 5) == 0){
	  	authnonce=parse_auth_value(pch2);
	  } else if (strncmp(pch2, "nc", 2) == 0){
	  	authnc=parse_auth_value(pch2);
	  } else if (strncmp(pch2, "cnonce", 6) == 0){
	  	authcnonce=parse_auth_value(pch2);
	  } else if (strncmp(pch2, "response", 8) == 0){
	  	authresponse=parse_auth_value(pch2);
	  } else if (strncmp(pch2, "uri", 3) == 0){
	  	uri=parse_auth_value(pch2);
	  }
	  pch = strtok (NULL, ",");
	}
	if(strncmp(uri, path, strlen(path)) == 0){
		A1tmp = (char*) malloc ((3 + strlen(cfg->http_user) + strlen(AUTHREALM) + strlen(cfg->http_pwd))*sizeof(char));
		strcpy(A1tmp, cfg->http_user);
		strcat(A1tmp, ":");
		strcat(A1tmp, AUTHREALM);
		strcat(A1tmp, ":");
		strcat(A1tmp, cfg->http_pwd);
		A2tmp = (char*) malloc ((2 + strlen(method) + strlen(uri))*sizeof(char));
		strcpy(A2tmp, method);
		strcat(A2tmp, ":");
		strcat(A2tmp, uri);
		A1=char_to_hex(MD5((unsigned char*)A1tmp, strlen(A1tmp), NULL), MD5_DIGEST_LENGTH, hex2ascii);
		A2=char_to_hex(MD5((unsigned char*)A2tmp, strlen(A2tmp), NULL), MD5_DIGEST_LENGTH, hex2ascii);
		A3tmp = (char*) malloc ((10 + strlen(A1) + strlen(A2) + strlen(authnonce) + strlen(authnc) + strlen(authcnonce))*sizeof(char));
		strcpy(A3tmp, A1);
		strcat(A3tmp, ":");
		strcat(A3tmp, authnonce);
		strcat(A3tmp, ":");
		strcat(A3tmp, authnc);
		strcat(A3tmp, ":");
		strcat(A3tmp, authcnonce);
		strcat(A3tmp, ":auth:");
		strcat(A3tmp, A2);
		A3=char_to_hex(MD5((unsigned char*)A3tmp, strlen(A3tmp), NULL), MD5_DIGEST_LENGTH, hex2ascii);
		if(strcmp(A3, authresponse) == 0) {
			if(strcmp(expectednonce, authnonce) == 0) authok = 1;
			else authok = 2;
		}
		free(A1tmp);
		free(A2tmp);
		free(A3tmp);
		free(A1);
		free(A2);
		free(A3);
	}
	return authok;
}

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

void send_headers(FILE *f, int status, char *title, char *extra, char *mime ){

  time_t now;
  char timebuf[128];

  fprintf(f, "%s %d %s\r\n", PROTOCOL, status, title);
  fprintf(f, "Server: %s\r\n", SERVER);

  now = time(NULL);
  strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
  fprintf(f, "Date: %s\r\n", timebuf);

  if (extra)
		fprintf(f, "%s\r\n", extra);

  if (mime)
		fprintf(f, "Content-Type: %s\r\n", mime);

	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	fprintf(f, "Last-Modified: %s\r\n", timebuf);
  fprintf(f, "Connection: close\r\n");
  fprintf(f, "\r\n");
}

void send_htmlhead(FILE *f, int refresh){

	/*build HTML head*/
	fprintf(f, "<HTML>\n");
	fprintf(f, "<HEAD>\n");
	fprintf(f, "<TITLE>OSCAM %s build #%s</TITLE>\n", CS_VERSION, CS_SVN_VERSION);
	fprintf(f, "<link rel=\"stylesheet\" type=\"text/css\" href=\"site.css\">\n");
	if (refresh > 0)
		fprintf(f, "<meta http-equiv=\"refresh\" content=\"%d; URL=/status.html\" />", refresh);
	fprintf(f, "</HEAD>\n");
	fprintf(f, "<BODY>");
	fprintf(f, "<H2>OSCAM %s build #%s</H2>", CS_VERSION, CS_SVN_VERSION);

}

void send_css(FILE *f){
	if(strlen(cfg->http_css) > 0 && file_exists(cfg->http_css) == 1){
		FILE *fp;
		char buffer[1024];
		int read;

		if((fp = fopen(cfg->http_css,"r"))==NULL) return;
		while((read = fread(&buffer,sizeof(char),1024,fp)) > 0) fwrite(&buffer, sizeof(char), read, f);
		fclose (fp);
	} else {
		int i;
		for (i=0; css[i]; i++) fprintf(f, "\t%s\n", css[i]);
	}
}

void send_footer(FILE *f){

  /*create footline*/
  time_t t;
  struct tm *lt;
	time(&t);

	lt=localtime(&t);

	fprintf(f, "<HR/>");
	fprintf(f, "<H4>OSCAM Webinterface - %02d.%02d.%02d %02d:%02d:%02d</H4>\r\n",
										lt->tm_mday, lt->tm_mon+1, lt->tm_year%100,
										lt->tm_hour, lt->tm_min, lt->tm_sec);
}

void send_oscam_menu(FILE *f){

	/*create menue*/
	fprintf(f, "<TABLE border=0 class=\"menu\">\n");
	fprintf(f, "	<TR>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./status.html\">STATUS</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html\">CONFIGURATION</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./readers.html\">READERS</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./userconfig.html\">USERS</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./entitlements.html\">ENTITLEMENTS</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./services.html\">SERVICES</TD>\n");
	fprintf(f, "	</TR>\n");
	fprintf(f, "</TABLE>\n");

}

//void send_error(FILE *f, int status, char *title, char *extra, char *text)
//{
//  send_headers(f, status, title, extra, "text/html");
//  fprintf(f, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\r\n", status, title);
//  fprintf(f, "<BODY><H4>%d %s</H4>\r\n", status, title);
//  fprintf(f, "%s\r\n", text);
//  fprintf(f, "</BODY></HTML>\r\n");
//}

void send_oscam_config_global(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i;
	int found=0;

	fprintf(f,"<BR><BR>");

	if (paramcount>0){
		for(i=0;i<paramcount;i++){
			//check the params for execute flag
			if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "execute"))) {
				found=1;
				break;
			}
		}
		if (found==1){
			//we found the execute flag
			for(i=0;i<paramcount;i++){
				if ((strcmp(uriparams[i], "part")) && (strcmp(uriparams[i], "action"))){
					fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", uriparams[i], urivalues[i]);
					//we use the same function as used for parsing the config tokens
					chk_t_global(uriparams[i], urivalues[i]);
				}
			}

			//Disclaimer
			fprintf(f,"<BR><BR><B>Configuration Global *DONE*</B><BR><BR>");
			refresh_oscam(REFR_SERVER);
			return;
		}
	}

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

void send_oscam_config_camd33(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i;
	int found=0;

	fprintf(f,"<BR><BR>");

	if (paramcount>0){
		for(i=0;i<paramcount;i++){
			//check the params for execute flag
			if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "execute"))) {
				found=1;
				break;
			}
		}
		if (found==1){
			//we found the execute flag
			for(i=0;i<paramcount;i++){
				if ((strcmp(uriparams[i], "part")) && (strcmp(uriparams[i], "action"))){
					fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", uriparams[i], urivalues[i]);
					//we use the same function as used for parsing the config tokens
					chk_t_camd33(uriparams[i], urivalues[i]);
				}
			}

			//Disclaimer
			fprintf(f,"<BR><BR><B>Configuration camd33 *DONE*</B><BR><BR>");
			refresh_oscam(REFR_SERVER);
			return;
		}
	}

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

void send_oscam_config_camd35(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i;
	int found=0;

	fprintf(f,"<BR><BR>");

	if (paramcount>0){
		for(i=0;i<paramcount;i++){
			//check the params for execute flag
			if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "execute"))) {
				found=1;
				break;
			}
		}
		if (found==1){
			//we found the execute flag
			for(i=0;i<paramcount;i++){
				if ((strcmp(uriparams[i], "part")) && (strcmp(uriparams[i], "action"))){
					fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", uriparams[i], urivalues[i]);
					//we use the same function as used for parsing the config tokens
					chk_t_camd35(uriparams[i], urivalues[i]);
				}
			}

			//Disclaimer
			fprintf(f,"<BR><BR><B>Configuration camd35 *DONE*</B><BR><BR>");
			refresh_oscam(REFR_SERVER);
			return;
		}
	}

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

void send_oscam_config_newcamd(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i;
	int found=0;

	fprintf(f,"<BR><BR>");

	if (paramcount>0){
		for(i=0;i<paramcount;i++){
			//check the params for execute flag
			if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "execute"))) {
				found=1;
				break;
			}
		}
		if (found==1){
			//we found the execute flag
			for(i=0;i<paramcount;i++){
				if ((strcmp(uriparams[i], "part")) && (strcmp(uriparams[i], "action"))){
					fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", uriparams[i], urivalues[i]);
					//we use the same function as used for parsing the config tokens
					chk_t_newcamd(uriparams[i], urivalues[i]);
				}
			}

			//Disclaimer
			fprintf(f,"<BR><BR><B>Configuration newcamd *DONE*</B><BR><BR>");
			refresh_oscam(REFR_SERVER);
			return;
		}
	}

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

void send_oscam_config_radegast(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i;
	int found=0;

	fprintf(f,"<BR><BR>");

	if (paramcount>0){
		for(i=0;i<paramcount;i++){
			//check the params for execute flag
			if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "execute"))) {
				found=1;
				break;
			}
		}
		if (found==1){
			//we found the execute flag
			for(i=0;i<paramcount;i++){
				if ((strcmp(uriparams[i], "part")) && (strcmp(uriparams[i], "action"))){
					fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", uriparams[i], urivalues[i]);
					//we use the same function as used for parsing the config tokens
					chk_t_radegast(uriparams[i], urivalues[i]);
				}
			}

			//Disclaimer
			fprintf(f,"<BR><BR><B>Configuration Radegast *DONE*</B><BR><BR>");
			refresh_oscam(REFR_SERVER);
			return;
		}
	}

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

void send_oscam_config_cccam(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i;
	int found=0;

	fprintf(f,"<BR><BR>");

	if (paramcount>0){
		for(i=0;i<paramcount;i++){
			//check the params for execute flag
			if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "execute"))) {
				found=1;
				break;
			}
		}
		if (found==1){
			//we found the execute flag
			for(i=0;i<paramcount;i++){
				if ((strcmp(uriparams[i], "part")) && (strcmp(uriparams[i], "action"))){
					fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", uriparams[i], urivalues[i]);
					//we use the same function as used for parsing the config tokens
					chk_t_cccam(uriparams[i], urivalues[i]);
				}
			}

			//Disclaimer
			fprintf(f,"<BR><BR>Configuration Cccam Do not yet implemented<BR><BR>");
			refresh_oscam(REFR_SERVER);
			return;
		}
	}

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

void send_oscam_config_gbox(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i;
	int found=0;

	fprintf(f,"<BR><BR>");

	if (paramcount>0){
		for(i=0;i<paramcount;i++){
			//check the params for execute flag
			if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "execute"))) {
				found=1;
				break;
			}
		}
		if (found==1){
			//we found the execute flag
			for(i=0;i<paramcount;i++){
				if ((strcmp(uriparams[i], "part")) && (strcmp(uriparams[i], "action"))){
					fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", uriparams[i], urivalues[i]);
					//we use the same function as used for parsing the config tokens
					chk_t_gbox(uriparams[i], urivalues[i]);
				}
			}

			//Disclaimer
			fprintf(f,"<BR><BR><B>Configuration Gbox *DONE*</B><BR><BR>");
			refresh_oscam(REFR_SERVER);
			return;
		}
	}

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
		fprintf(f,"%s%08X", dot, cfg->locals[i]);
		dot=";";
	}
	fprintf(f,"\"></TD></TR>\r\n");

	//Tablefoot and finish form
	fprintf(f,"</TABLE>\r\n");
	fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
}

void send_oscam_config_monitor(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i;
	int found=0;

	fprintf(f,"<BR><BR>");

	if (paramcount>0){
		for(i=0;i<paramcount;i++){
			//check the params for execute flag
			if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "execute"))) {
				found=1;
				break;
			}
		}
		if (found==1){
			//we found the execute flag
			for(i=0;i<paramcount;i++){
				if ((strcmp(uriparams[i], "part")) && (strcmp(uriparams[i], "action"))){
					fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", uriparams[i], urivalues[i]);
					//we use the same function as used for parsing the config tokens
					chk_t_monitor(uriparams[i], urivalues[i]);
				}
			}

			//Disclaimer
			fprintf(f,"<BR><BR><B>Configuration Monitor *DONE*</B><BR><BR>");
			refresh_oscam(REFR_SERVER);
			return;
		}
	}

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

#ifdef CS_ANTICASC

void send_oscam_config_anticasc(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i;
	int found=0;

	fprintf(f,"<BR><BR>");

	if (paramcount>0){
		for(i=0;i<paramcount;i++){
			//check the params for execute flag
			if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "execute"))) {
				found=1;
				break;
			}
		}
		if (found==1){
			//we found the execute flag
			for(i=0;i<paramcount;i++){
				if ((strcmp(uriparams[i], "part")) && (strcmp(uriparams[i], "action"))){
					fprintf(f,"Parameter: %s set to Value: %s<BR>\r\n", uriparams[i], urivalues[i]);
					//we use the same function as used for parsing the config tokens
					chk_t_ac(uriparams[i], urivalues[i]);
				}
			}

			//Disclaimer
			fprintf(f,"<BR><BR><B>Configuration Anticascading *DONE*</B><BR><BR>");
			refresh_oscam(REFR_ANTICASC);
			return;
		}
	}

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

#endif

void send_oscam_config(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	fprintf(f,"<BR><BR>");

	/*create submenue*/
	fprintf(f, "<TABLE border=0 class=\"menu\">\n");
	fprintf(f, "	<TR>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html?part=global\">Global</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html?part=camd33\">Camd3.3</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html?part=camd35\">Camd3.5</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html?part=newcamd\">Newcamd</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html?part=radegast\">Radegast</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html?part=cccam\">Cccam</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html?part=gbox\">Gbox</TD>\n");
#ifdef CS_ANTICASC
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html?part=anticasc\">Anticascading</TD>\n");
#endif
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html?part=monitor\">Monitor</TD>\n");
	fprintf(f, "	</TR>\n");
	fprintf(f, "</TABLE>\n");

	int paramfound=0;
	int i;

	if (paramcount==0)
		send_oscam_config_global(f, uriparams, urivalues, paramcount);
	else
		for(i=0;i<paramcount;i++){
			if (!strcmp(uriparams[i], "part")) {
				paramfound=1;
				break;
			}
		}
	if (paramfound==1){
		if (!strcmp(urivalues[i],"global"))
			send_oscam_config_global(f, uriparams, urivalues, paramcount);
		else if (!strcmp(urivalues[i],"camd33"))
			send_oscam_config_camd33(f, uriparams, urivalues, paramcount);
		else if (!strcmp(urivalues[i],"camd35"))
			send_oscam_config_camd35(f, uriparams, urivalues, paramcount);
		else if (!strcmp(urivalues[i],"newcamd"))
			send_oscam_config_newcamd(f, uriparams, urivalues, paramcount);
		else if (!strcmp(urivalues[i],"radegast"))
			send_oscam_config_radegast(f, uriparams, urivalues, paramcount);
		else if (!strcmp(urivalues[i],"cccam"))
			send_oscam_config_cccam(f, uriparams, urivalues, paramcount);
		else if (!strcmp(urivalues[i],"gbox"))
			send_oscam_config_gbox(f, uriparams, urivalues, paramcount);
#ifdef CS_ANTICASC
		else if (!strcmp(urivalues[i],"anticasc"))
			send_oscam_config_anticasc(f, uriparams, urivalues, paramcount);
#endif
		else if (!strcmp(urivalues[i],"monitor"))
			send_oscam_config_monitor(f, uriparams, urivalues, paramcount);
		else
			send_oscam_config_global(f, uriparams, urivalues, paramcount);
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

		fprintf(f,"\t<TR><TD>%s</TD><TD>%s</TD><TD><A HREF=\"/readerconfig.html?user=%s\">Edit Settings</A></TD></TR>",reader[ridx].label, ctyp, reader[ridx].label);
	}
	fprintf(f,"</TABLE>\r\n");
	//kill(client[0].pid, SIGUSR1);
}

void send_oscam_reader_config(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i,ridx;

	fprintf(f,"<BR><BR>\r\n");
	for(i=0;i<paramcount;i++)
		if (!strcmp(uriparams[i], "user")) break;

	for(ridx=0;ridx<CS_MAXREADER;ridx++){
		if (!reader[ridx].device[0]){
			fprintf(f,"Reader %s not found", urivalues[i]);
			return ;
		}
		if (!strcmp(urivalues[i],reader[ridx].label)) break;
	}

	/*build form head*/
	fprintf(f,"<form action=\"/readerconfig_do.html\" method=\"get\"><input name=\"user\" type=\"hidden\" value=\"%s\">\r\n", reader[ridx].label);
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

void send_oscam_reader_config_do(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
	int i,ridx;

	fprintf(f,"<BR><BR>\r\n");
	for(i=0;i<paramcount;i++)
		if (!strcmp(uriparams[i], "user")) break;

	for(ridx=0;ridx<CS_MAXREADER;ridx++){
		if (!reader[ridx].device[0]){
			fprintf(f,"Reader %s not found", urivalues[i]);
			return ;
		}
		if (!strcmp(urivalues[i],reader[ridx].label)) break;
	}

	fprintf(f,"Reader: %s - Nothing changed", reader[ridx].label);
	fprintf(f,"<BR><BR>Reader not yet implemented<BR><BR>");
	refresh_oscam(REFR_READERS);
}

void send_oscam_user_config_add(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {
		struct s_auth *ptr;
		struct s_auth *account;
		int i, accidx=0;
		char *newuser;

		for(i=0;i<paramcount;i++)
			if (!strcmp(uriparams[i], "user")) break;

		newuser = urivalues[i];

		if (urivalues[i]) {

			for (account=cfg->account; (account) ; account=account->next){
				//account already exist - show config for this account
				if(!strcmp(urivalues[i], account->usr)){
					uriparams[0] = "user";
					urivalues[0] = newuser;
					send_oscam_user_config(f, uriparams, urivalues, 1);
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
      strncpy((char *)account->usr, urivalues[i], sizeof(account->usr)-1);
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
	uriparams[0] = "user";
	urivalues[0] = newuser;
	send_oscam_user_config(f, uriparams, urivalues, 1);

}

void send_oscam_user_config_delete(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {

	int i, found = 0;
	struct s_auth *account, *account2;

	for(i=0;i<paramcount;i++) if (!strcmp(uriparams[i], "user")) break;

	account=cfg->account;
	if(strcmp(account->usr, urivalues[i]) == 0){
		cfg->account = account->next;
		free(account);
		found = 1;
	} else if (account->next != NULL){
		do{
			if(strcmp(account->next->usr, urivalues[i]) == 0){
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

	send_oscam_user_config(f, uriparams, urivalues, 0);
}

void send_oscam_user_config_do(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {

	struct s_auth *account;
	int i;
	char servicelabels[255]="";

	//find username within params
	for(i=0;i<paramcount;i++)
		if (!strcmp(uriparams[i], "user")) break;

	//identfy useraccount
	for (account=cfg->account; (account) ; account=account->next){
		if(!strcmp(urivalues[i], account->usr))
			break;
	}

	//clear group
	account->grp = 0;

	fprintf(f,"<BR><BR>\r\n");

	for(i=0;i<paramcount;i++){
		if ((strcmp(uriparams[i], "action")) && (strcmp(uriparams[i], "user"))){
			fprintf(f,"%s = %s<BR>\r\n", uriparams[i], urivalues[i]);
			if (!strcmp(uriparams[i], "services"))
				sprintf(servicelabels + strlen(servicelabels), "%s,", urivalues[i]);
			else
				chk_account(uriparams[i], urivalues[i], account);
		}
		else if (!strcmp(uriparams[i], "user"))
			fprintf(f,"<B>User %s is reconfigured as follow</B><BR><BR>\r\n", urivalues[i]);
	}

	chk_account("services", servicelabels, account);

	if (write_userdb()==0)
		refresh_oscam(REFR_ACCOUNTS);
	else
		fprintf(f,"<B>Write Config failed</B><BR><BR>\r\n");
	send_oscam_user_config(f, uriparams, urivalues, 0);
}

void send_oscam_user_config(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {

	int i;
	struct s_auth *account;

	if (paramcount>0){
		for(i=0;i<paramcount;i++){
			if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "execute"))) {
				send_oscam_user_config_do(f, uriparams, urivalues, paramcount);
				return;
			}
			else if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "adduser"))) {
				send_oscam_user_config_add(f, uriparams, urivalues, paramcount);
				return;
			}
			else if (!strcmp(uriparams[i], "action") && (!strcmp(urivalues[i], "delete"))) {
				send_oscam_user_config_delete(f, uriparams, urivalues, paramcount);
				return;
			}
		}
	}
	else {
		/* No Parameters given --> list accounts*/
		char *status = "offline";

		fprintf(f,"<BR><BR><TABLE cellspacing=\"0\" cellpadding=\"10\">\r\n");
		fprintf(f,"<TR><TH>Label</TH><TH>Status (not exact)</TH><TH colspan=\"2\" align=\"center\">Action</TH></TR>");
		for (account=cfg->account; (account) ; account=account->next){
			status="offline";
			fprintf(f,"<TR>\r\n");
			for (i=0; i<CS_MAXPID; i++)
				if (!strcmp(client[i].usr, account->usr))
					status="<b>online</b>";

			fprintf(f,"<TD>%s</TD><TD>%s</TD><TD><A HREF=\"/userconfig.html?user=%s\">Edit Settings</A></TD>",account->usr, status, account->usr);
			fprintf(f,"<TD><A HREF=\"/userconfig.html?user=%s&action=delete\">Delete User</A></TD>",account->usr);
			fprintf(f,"</TR>\r\n");
		}
		fprintf(f,"<TR>\r\n");
		fprintf(f,"<FORM >\r\n");
		fprintf(f,"<TD>New User:</TD>\r\n");
		fprintf(f,"<TD><input name=\"user\" type=\"text\"></TD>\r\n");
		fprintf(f,"<input name=\"action\" type=\"hidden\" value=\"adduser\">\r\n");
		fprintf(f,"<TD colspan=\"2\" align=\"center\"><input type=\"submit\" value=\"Add User\"></TD></form>\r\n");
		fprintf(f,"</TABLE>\r\n");
		return;
	}

	fprintf(f,"<BR><BR>\r\n");

	for(i=0;i<paramcount;i++)
		if (!strcmp(uriparams[i], "user")) break;

	/*identfy useraccount*/
	for (account=cfg->account; (account) ; account=account->next){
		if(!strcmp(urivalues[i], account->usr))
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
	if (account->au > -1)
		fprintf(f,"\t<option value=\"-1\" selected>none</option>\r\n");
	else
		fprintf(f,"\t<option value=\"-1\">none</option>\r\n");

	int ridx;
	for (ridx=0; ridx<CS_MAXREADER; ridx++){
		if(!reader[ridx].device[0]) break;

		if (account->au == ridx)
			fprintf(f,"\t<option value=\"%d\" selected>%s</option>\r\n", ridx, reader[ridx].label);
		else
			fprintf(f,"\t<option value=\"%d\">%s</option>\r\n", ridx, reader[ridx].label);
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
//	fprintf(f,"<TR><TD>Ident:</TD><TD><input name=\"ident\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");
//	for (i=0;i<account->ftab.nfilts;i++){
//		fprintf(f, "%04X;", account->ftab.filts[i].caid);
//	}
//	fprintf(f,"\"></TD></TR>\r\n");

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

	fprintf(f,"</TABLE>\r\n");
	fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
}

void send_oscam_entitlement(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {

  /*build entitlements from reader init history*/
	int i,ridx;
	char *p, *ctyp;

	if(paramcount==0){
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
			fprintf(f, "\t<TR><TD>%s</TD><TD>%s</TD><TD><A HREF=\"/entitlements.html?user=%s\">Check</A></TD></TR>\r\n", reader[ridx].label, ctyp, reader[ridx].label);
		}
		fprintf(f, "</TABLE>\r\n");
	}
	else {

		for(i=0;i<paramcount;i++)
			if (!strcmp(uriparams[i], "user")) break;

		fprintf(f, "<BR><BR>Entitlement for %s<BR><BR>\r\n", urivalues[i]);

		for (ridx=0; ridx<CS_MAXREADER; ridx++)
			if (!strcmp(urivalues[i], reader[ridx].label)) break;

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

void monitor_client_status(FILE *f, char id, int i){

	if (client[i].pid) {
		char ldate[16], ltime[16], *usr;
		int lsec, isec, cnr, con, cau;
		time_t now;
		struct tm *lt;
		now=time((time_t)0);

		if	((cfg->mon_hideclient_to <= 0) ||
				(((now-client[i].lastecm)/60)<cfg->mon_hideclient_to) ||
				(((now-client[i].lastemm)/60)<cfg->mon_hideclient_to) ||
				(client[i].typ!='c'))
		{
			lsec=now-client[i].login;
			isec=now-client[i].last;
			usr=client[i].usr;

			if (((client[i].typ=='r') || (client[i].typ=='p')) && (con=cs_idx2ridx(i))>=0)
				usr=reader[con].label;

			if (client[i].dup)
        con=2;
      else
				if ((client[i].tosleep) && (now-client[i].lastswitch>client[i].tosleep))
					con=1;
				else
					con=0;

			if (i-cdiff>0)
				cnr=i-cdiff;
			else
				cnr=(i>1) ? i-1 : 0;

			if( (cau=client[i].au+1) )
				if ((now-client[i].lastemm)/60>cfg->mon_aulow)
					cau=-cau;

      lt=localtime(&client[i].login);
      sprintf(ldate, "%2d.%02d.%02d", lt->tm_mday, lt->tm_mon+1, lt->tm_year % 100);
      sprintf(ltime, "%2d:%02d:%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
      fprintf(f, "<TD>%d</TD><TD>%c</TD><TD>%d</TD><TD>%s</TD><TD>%d</TD><TD>%d</TD><TD>%s</TD><TD>%d</TD><TD>%s</TD><TD>%s</TD><TD>%s</TD><TD>%d</TD><TD>%04X:%04X</TD><TD>%s</TD><TD>%d</TD><TD>%d</TD>\n",
              client[i].pid, client[i].typ, cnr, usr, cau, client[i].crypted,
              cs_inet_ntoa(client[i].ip), client[i].port, monitor_get_proto(i),
              ldate, ltime, lsec, client[i].last_caid, client[i].last_srvid,
              monitor_get_srvname(client[i].last_srvid), isec, con);
    }
  }
}

void send_oscam_status(FILE *f) {
	int i;

	fprintf(f,"<BR><BR><TABLE WIDTH=\"100%%\" cellspacing=\"0\" class=\"status\">\n");
	fprintf(f,"<TR><TH>PID</TH><TH>Typ</TH><TH>ID</TH><TH>Label</TH><TH>AU</TH><TH>0</TH><TH>Address</TH><TH>Port</TH><TH>Protocol</TH><TH>Login</TH><TH>Login</TH><TH>Time</TH><TH>caid:srvid</TH><TH>&nbsp;</TH><TH>Idle</TH><TH>0</TH>");

	for (i=0; i<CS_MAXPID; i++)
		if (client[i].pid)  {
			fprintf(f,"<TR class=\"%c\">", client[i].typ);
			monitor_client_status(f, client[i].pid, i);
			fprintf(f,"</TR>\n");
		}

	fprintf(f,"</TABLE><BR>\n");
	fprintf(f,"<DIV class=\"log\">");
#ifdef CS_LOGHISTORY
	for (i=(*loghistidx+3) % CS_MAXLOGHIST; i!=*loghistidx; i=(i+1) % CS_MAXLOGHIST){
		char *p_usr, *p_txt;
		p_usr=(char *)(loghist+(i*CS_LOGHISTSIZE));
		p_txt=p_usr+32;
		if (p_txt[0]){
//			char sbuf[8];
//			sprintf(sbuf, "%03d", client[cs_idx].logcounter);
//			client[cs_idx].logcounter=(client[cs_idx].logcounter+1) % 1000;
//			memcpy(p_txt+4, sbuf, 3);
			fprintf(f, "%s<BR>\n", p_txt+8);
		}
	}
#else
	fprintf(f,"the flag CS_LOGHISTORY is not set in your binary<BR>\n");
#endif
	fprintf(f,"</DIV>");
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
  int maxparams=100;
  char buf[4096];
  char tmp[4096];

  int authok = 0;
  char expectednonce[64];

  char *method;
  char *path;
  char *protocol;
  char *pch;
  char *pch2;
  char *uriparams[maxparams];
  char *urivalues[maxparams];
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

  /* Calculate the amount of items in array */
  int pagescnt = sizeof(pages)/sizeof(char *);
  int pgidx = -1;
  int i;
  int paramcount = 0;
  int parsemode = 1;

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
        if(paramcount >= maxparams) break;
        ++paramcount;
        uriparams[paramcount-1] = pch2;
      } else {
        urivalues[paramcount-1] = pch2;
      }
      parsemode = -parsemode;
      pch2 = pch + 1;
    }
    ++pch;
  }
  /* last value wasn't processed in the loop yet... */
  if(parsemode == -1 && paramcount <= maxparams){
      urivalues[paramcount-1] = pch2;
  }

	if(strlen(cfg->http_user) == 0 || strlen(cfg->http_pwd) == 0) authok = 1;
	else calculate_nonce(expectednonce, sizeof(expectednonce)/sizeof(char));

	/* Read remaining request (we're only interested in auth header) */
  while (fgets(tmp, sizeof(tmp), f))  {
    if (tmp[0] == '\r' && tmp[1] == '\n') break;
    else if(authok == 0 && strlen(tmp) > 50 && strncmp(tmp, "Authorization: Digest ", 22) == 0) {
    	authok = check_auth(tmp, method, path, expectednonce);
    }
  }

  //printf("%s %d\n", path, pgidx);
  //for(i=0; i < paramcount; ++i) printf("%s : %s\n", uriparams[i], urivalues[i]);

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
  if(pgidx == 8) send_css(f);
	else {

		if (pgidx == 3)	send_htmlhead(f, cfg->http_refresh); //status
		else send_htmlhead(f,0);

  send_oscam_menu(f);
  switch(pgidx){
    case  0: send_oscam_config(f, uriparams, urivalues, paramcount); break;
    case  1: send_oscam_reader(f); break;
    case  2: send_oscam_entitlement(f, uriparams, urivalues, paramcount); break;
    case  3: send_oscam_status(f); break;
    case  4: send_oscam_user_config(f, uriparams, urivalues, paramcount); break;
    case  5: send_oscam_reader_config(f, uriparams, urivalues, paramcount); break;
    case  6: send_oscam_reader_config_do(f, uriparams, urivalues, paramcount); break;
    case	7: send_oscam_sidtab(f); break;
    default: send_oscam_status(f); break;
  }

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
