#include "globals.h"
//
// webserver.c
//
// Simple HTTP server sample for sanos
//

//#include <os.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

#define SERVER "webserver/1.0"
#define PROTOCOL "HTTP/1.1"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

static  char*   css[] = {
			"p {color: white; }",
			"h2 {color: orange; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 20px; line-height: 20px;}",
			"h4 {color: black; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px; line-height: 12px; }",
			"TABLE{background-color:#66CCFF;}",
			"TD {border:1px solid gray; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px;padding:5px;background-color:#6666FF;}",
			"TH {border:1px solid gray; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px;padding:5px;background-color:#6699FF}",
			"DIV.log{border:1px solid black;background-color: black; font-family: Courier, \"Courier New\", monospace ; color: white;}",
			"TABLE.menu{background-color:gold;align:center;}",
			"TABLE.status{background-color:#66CCFF;empty-cells:show;}",
			"TD.menu {border:2px outset lightgrey;background-color:silver;font-color:black; font-family: Verdana, Arial, Helvetica, sans-serif;}",
			"body {background-color: #FFFF66;font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px;}",
			"A:link {text-decoration: none; color:blue}",
			"A:visited {text-decoration: none; color:blue}",
			"A:active {text-decoration: none; color:white}",
			"A:hover {text-decoration: none; color: red;}",
				NULL };

int strtoken(char *str, char *separator, char *token[]){
  int i = 0;

  token[0] = strtok(str, separator);

  while ( token[i] ) {
    i++;
    token[i] = strtok(NULL, separator);
  }
  return ( i );
}

static int x2i(int i){
	i=toupper(i);
	i = i - '0';
	if(i > 9) i = i - 'A' + '9' + 1;
	return i;
}

void urldecode(char *s){
	int c, c1, n;
	char *s0,*t;
	t = s0 = s;
	n = strlen(s);
	while(n >0){
		c = *s++;
              if(c == '+') c = ' ';
		else if(c == '%' && n > 2){
			c = *s++;
			c1 = c;
			c = *s++;
			c = 16*x2i(c1) + x2i(c);
			n -= 2;
		}
		*t++ = c;
		n--;
	}
	*t = 0;
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

void send_htmlhead(FILE *f){

	/*build HTML head and CSS definitions*/
	int i;

	fprintf(f, "<HTML>\n");
	fprintf(f, "<HEAD>\n");
	fprintf(f, "<TITLE>OSCAM %s build #%s</TITLE>\n", CS_VERSION, CS_SVN_VERSION);
	fprintf(f, "<STYLE type=\"text/css\">\n");

	for (i=0; css[i]; i++)
		fprintf(f, "\t%s\n", css[i]);

	fprintf(f, "</STYLE>\n");
	fprintf(f, "</HEAD>\n");
	fprintf(f, "<BODY>");
	fprintf(f, "<H2>OSCAM %s build #%s</H2>", CS_VERSION, CS_SVN_VERSION);

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
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./users.html\">USERS</TD>\n");
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

void send_oscam_config(FILE *f) {
	fprintf(f,"<BR><BR>Configuration not yet implemented<BR><BR>");
}

void send_oscam_reader(FILE *f) {
	int ridx;
	char *ctyp;

	fprintf(f,"<BR><BR><TABLE cellspacing=\"0\" cellpadding=\"10\">\r\n");
	fprintf(f,"<TR><TH>Label</TH><TH>Protocol</TH><TH>Action</TH></TR>");

	for(ridx=0;ridx<CS_MAXREADER;ridx++){
		if(!reader[ridx].device[0]) break;

		switch(reader[ridx].typ)		// TODO like ph
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

	fprintf(f,"<TR><TD>Key:</TD><TD><input name=\"key\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].ncd_key);
	//fprintf(f,"<TR><TD>Password:</TD><TD><input name=\"password\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].gbox_pwd);
	fprintf(f,"<TR><TD>Pincode:</TD><TD><input name=\"pincode\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].pincode);
	fprintf(f,"<TR><TD>Readnano:</TD><TD><input name=\"readnano\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].emmfile);
	//fprintf(f,"<TR><TD>Services:</TD><TD><input name=\"services\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].emmfile);
	fprintf(f,"<TR><TD>Inactivitytimeout:</TD><TD><input name=\"inactivitytimeout\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%d\"></TD></TR>\r\n", reader[ridx].tcp_ito);
	fprintf(f,"<TR><TD>Reconnecttimeout:</TD><TD><input name=\"reconnecttimeout\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%d\"></TD></TR>\r\n", reader[ridx].tcp_rto);
	fprintf(f,"<TR><TD>Disableserverfilter:</TD><TD><input name=\"disableserverfilter\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%d\"></TD></TR>\r\n", reader[ridx].ncd_disable_server_filt);
	fprintf(f,"<TR><TD>Fallback:</TD><TD><input name=\"fallback\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"%d\"></TD></TR>\r\n", reader[ridx].fallback);
	fprintf(f,"<TR><TD>Logport:</TD><TD><input name=\"logport\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%d\"></TD></TR>\r\n", reader[ridx].log_port);
	//fprintf(f,"<TR><TD>Caid:</TD><TD><input name=\"caid\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"%s\"></TD></TR>\r\n", reader[ridx].log_port);
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
}

void send_oscam_user(FILE *f) {
	/*list accounts*/
	int i = 0;
	char *status = "offline";
	struct s_auth *account;

	fprintf(f,"<BR><BR><TABLE cellspacing=\"0\" cellpadding=\"10\">\r\n");
	fprintf(f,"<TR><TH>Label</TH><TH>Status (not exact)</TH><TH>Action</TH></TR>");
	for (account=cfg->account; (account) ; account=account->next){
		status="offline";
		fprintf(f,"<TR>\r\n");
		for (i=0; i<CS_MAXPID; i++)
			if (!strcmp(client[i].usr, account->usr))
				status="<b>online</b>";

		fprintf(f,"<TD>%s</TD><TD>%s</TD><TD><A HREF=\"/userconfig.html?user=%s\">Edit Settings</A>",account->usr, status, account->usr);
		fprintf(f,"</TR>\r\n");
	}
	fprintf(f,"</TABLE>\r\n");
}

void send_oscam_user_config(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {

	struct s_auth *account;
	int i;
	CAIDTAB *ctab;
	TUNTAB *ttab;

	fprintf(f,"<BR><BR>\r\n");

	for(i=0;i<paramcount;i++)
		if (!strcmp(uriparams[i], "user")) break;

	/*identfy useraccount*/
	for (account=cfg->account; (account) ; account=account->next){
		if(!strcmp(urivalues[i], account->usr))
			break;
	}

	fprintf(f,"<form action=\"/userconfig_do.html\" method=\"get\"><input name=\"user\" type=\"hidden\" value=\"%s\">\r\n", account->usr);

	fprintf(f,"<TABLE cellspacing=\"0\">");
	fprintf(f,"<TH></TH><TH>Edit User %s </TH>", account->usr);
	fprintf(f,"<TR><TD>Password:</TD><TD><input name=\"pwd\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", account->pwd);
	fprintf(f,"<TR><TD>Group:</TD><TD><input name=\"grp\" type=\"text\" size=\"10\" maxlength=\"10\" value=\"");

		/*restore the settings format of group */
		if (account->grp > 0){
			i = 0;
			int dot=0; //flag for comma
			long lgrp = account->grp;

			while(lgrp != 0){

				if((lgrp%2) == 1) {
					if(dot==0){
						fprintf(f, "%d", i+1);
						dot=1;
					}
					else
						fprintf(f, ",%d", i+1);
				}
				lgrp = lgrp/2;
				i++;
			}
		}

	fprintf(f,"\"></TD></TR>\r\n");

	if (strlen(account->dyndns)>0)
		fprintf(f,"<TR><TD>Hostname:</TD><TD><input name=\"dyndns\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"%s\"></TD></TR>\r\n", account->dyndns);
	else
		fprintf(f,"<TR><TD>Hostname:</TD><TD><input name=\"dyndns\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"\"></TD></TR>\r\n");


	fprintf(f,"<TR><TD>Uniq:</TD><TD><select name=\"uniq\" >\r\n");
		if (account->uniq == 0)
			fprintf(f,"\t<OPTION value=\"0\" selected>(0) none</OPTION>\r\n");
		else
			fprintf(f,"\t<OPTION value=\"0\">(0) none</OPTION>\r\n");

		if (account->uniq == 1)
			fprintf(f,"\t<OPTION value=\"1\" selected>(1) strict</OPTION>\r\n");
		else
			fprintf(f,"\t<OPTION value=\"1\">(1) strict</OPTION>\r\n");

		if (account->uniq == 2)
			fprintf(f,"\t<OPTION value=\"2\" selected>(2) IP based</OPTION>\r\n");
		else
			fprintf(f,"\t<OPTION value=\"2\">(2) IP based</OPTION>\r\n");

	fprintf(f,"</SELECT></TD></TR>\r\n");

	if(!account->tosleep)
		fprintf(f,"<TR><TD>Sleep:</TD><TD><input name=\"tosleep\" type=\"text\" size=\"4\" maxlength=\"4\" value=\"0\"></TD></TR>\r\n");
	else
		fprintf(f,"<TR><TD>Sleep:</TD><TD><input name=\"tosleep\" type=\"text\" size=\"4\" maxlength=\"4\" value=\"%d\"></TD></TR>\r\n", account->tosleep);

	/*Monlevel selector*/
	fprintf(f,"<TR><TD>Monlevel:</TD><TD><select name=\"monlevel\" >\r\n");
	for(i=1;i<5;i++){
		if(i==account->monlvl)
			fprintf(f,"\t<option selected>%d</option>\r\n",i);
		else
			fprintf(f,"\t<option>%d</option>\r\n",i);
	}
	fprintf(f,"</select></TD></TR>\r\n");


	/*AU Selector*/
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
	/*services - first we have to move the long sidtabok/sidtabno to a binary array*/
	int pos;
	char sidok[33];
	for (pos=0;pos<32;pos++) sidok[pos]='0';

	char sidno[33];
	for (pos=0;pos<32;pos++) sidno[pos]='0';

	pos=0;
	long dezok = account->sidtabok;
	while (dezok!=0){
		sidok[pos]='0'+dezok % 2;
		dezok=dezok / 2;
		pos++;
	}

	pos=0;
	long dezno = account->sidtabno;
	while (dezno!=0){
		sidno[pos]='0'+dezno % 2;
		dezno=dezno / 2;
		pos++;
	}

	struct s_sidtab *sidtab = cfg->sidtab;
	fprintf(f,"<TR><TD>Services:</TD><TD><TABLE cellspacing=\"0\">");

	pos=0;
	for (; sidtab; sidtab=sidtab->next){
		if(sidok[pos]=='1')
			fprintf(f,"<TR><TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"%s\" checked> %s</TD>", sidtab->label, sidtab->label);
		else
			fprintf(f,"<TR><TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"%s\"> %s</TD>", sidtab->label, sidtab->label);

		if(sidno[pos]=='1')
			fprintf(f,"<TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"!%s\" checked> !%s</TD></TR>\r\n", sidtab->label, sidtab->label);
		else
			fprintf(f,"<TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"!%s\"> !%s</TD></TR>\r\n", sidtab->label, sidtab->label);

		pos++;
	}
	fprintf(f,"</TD></TR></TABLE>\r\n");

	/*---------------------CAID-----------------------------
	-------------------------------------------------------*/
	fprintf(f,"<TR><TD>CAID:</TD><TD><input name=\"caid\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");

	/*make string from caidtab*/
	i = 0;
	ctab = &account->ctab;

	if (ctab->caid[i]){

		while(ctab->caid[i]) {

			if (i==0)
				fprintf(f, "%04X", ctab->caid[i]);
			else
				fprintf(f, ",%04X", ctab->caid[i]);

			if(ctab->mask[i])
				fprintf(f, "&%04X", ctab->mask[i]);

			i++;
		}

	}
	fprintf(f,"\"></TD></TR>\r\n");


	/*Betatunnel*/
	fprintf(f,"<TR><TD>Betatunnel:</TD><TD><input name=\"betatunnel\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"");

	/*make string from Betatunneltab*/
	i = 0;
	ttab = &account->ttab;

	if (ttab->bt_caidfrom[i]) {

		while(ttab->bt_caidfrom[i]) {

			if (i==0)
				fprintf(f, "%04X", ttab->bt_caidfrom[i]);
			else
				fprintf(f, ",%04X", ttab->bt_caidfrom[i]);

			if(ttab->bt_caidto[i])
				fprintf(f, ".%04X", ttab->bt_caidto[i]);

			if(ttab->bt_srvid[i])
				fprintf(f, ":%04X", ttab->bt_srvid[i]);

			i++;
		}
	}

	fprintf(f,"\"></TD></TR>\r\n");
	fprintf(f,"</TABLE>\r\n");
	fprintf(f,"<input type=\"submit\" value=\"OK\"></form>\r\n");
}

void send_oscam_user_config_do(FILE *f, char *uriparams[], char *urivalues[], int paramcount) {

	struct s_auth *account;
	int i,j,updateservices;
	int paramidx = -1;
	char *ptr;
	char servicelabels[255]="";

	char *params[]={"pwd",
									"grp",
									"dyndns",
									"uniq",
									"tosleep",
									"monlevel",
									"au",
									"caid",
									"betatunnel",
									"services"};

	/* Calculate the amount of items in array */
  int paramcnt = sizeof(params)/sizeof(char *);

	for(i=0;i<paramcount;i++)
		if (!strcmp(uriparams[i], "user")) break;

	/*identfy useraccount*/
	for (account=cfg->account; (account) ; account=account->next){
		if(!strcmp(urivalues[i], account->usr))
			break;
	}

	for(j=0; j<paramcount; j++){

		/* Map page to our static page definitions */
		for (i=0; i<paramcnt; i++)
			if (!strcmp(uriparams[j], params[i])) paramidx = i;

		switch(paramidx){
			case 0: strncpy((char *)account->pwd, urivalues[j], sizeof(account->pwd)-1);
								break;
			case 1: account->grp = 0;
							if (urivalues[j]){
								for (ptr=strtok(urivalues[j], ","); ptr; ptr=strtok(NULL, ",")) {
									int g;
									g = atoi(ptr);
									if ((g>0) && (g<33)) account->grp|=(1<<(g-1));
								}
							}
								break;
			case 2: if (strlen(urivalues[j])>0)
								strncpy((char *)account->dyndns, urivalues[j], sizeof(account->dyndns)-1);
								break;
			case 3: if (strlen(urivalues[j])>0)
								account->uniq = atoi(urivalues[j]);
								break;
			case 4: if (strlen(urivalues[j])>0)
								account->tosleep = atoi(urivalues[j]);
								break;
			case 5: if (strlen(urivalues[j])>0)
								account->monlvl = atoi(urivalues[j]);
								break;
			case 6: if (strlen(urivalues[j])>0)
								account->au = atoi(urivalues[j]);
								break;
			case 7: if (strlen(urivalues[j])>0)
								chk_caidtab(urivalues[j], &account->ctab);
								break;
			case 8: if (strlen(urivalues[j])>0)
								chk_tuntab(urivalues[j], &account->ttab);
								break;
			case 9: sprintf(servicelabels+strlen(servicelabels), "%s,", urivalues[j]);
							updateservices=1;
								break;
			default: break;

//			case  5: chk_services(argarray[2], &account->sidtabok, &account->sidtabno);
//													break;                              //services
//
//			case  7: chk_ftab(argarray[2], &account->ftab, "user", account->usr, "provid");
//													break;                              //ident
//
//			case  9: chk_ftab(argarray[2], &account->fchid, "user", account->usr, "chid");
//													break;                              //chid
//			case  10:chk_cltab(argarray[2], &account->cltab);
//													break;                              //class
//
//			default: continue;
		}
	}
	if (updateservices==1)
		chk_services(servicelabels, &account->sidtabok, &account->sidtabno);

	cs_reinit_clients();
	send_oscam_user(f);
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

			switch(reader[ridx].typ)		// TODO like ph
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

void send_oscam_status(FILE *f){
	int i;

	fprintf(f,"<BR><BR><TABLE WIDTH=\"100%\" cellspacing=\"0\" class=\"status\">\n");
	fprintf(f,"<TR><TH>PID</TH><TH>Typ</TH><TH>ID</TH><TH>Label</TH><TH>AU</TH><TH>0</TH><TH>Address</TH><TH>Port</TH><TH>Protocol</TH><TH>Login</TH><TH>Login</TH><TH>Time</TH><TH>caid:srvid</TH><TH>&nbsp;</TH><TH>Idle</TH><TH>0</TH>");

	for (i=0; i<CS_MAXPID; i++)
		if (client[i].pid)  {
			fprintf(f,"<TR>");
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
			char sbuf[8];
			sprintf(sbuf, "%03d", client[cs_idx].logcounter);
			client[cs_idx].logcounter=(client[cs_idx].logcounter+1) % 1000;
			memcpy(p_txt+4, sbuf, 3);
			fprintf(f, "%s<BR>\n", p_txt);
		}
	}
#else
	fprintf(f,"the flag CS_LOGHISTORY is not set in your binary<BR>\n");
#endif
	fprintf(f,"</DIV>");
}

void send_oscam_sidtab(FILE *f)
{
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
			"/users.html",
			"/entitlements.html",
			"/status.html",
			"/userconfig.html",
			"/userconfig_do.html",
			"/readerconfig.html",
			"/readerconfig_do.html",
			"/services.html"};

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

  /* Parse parameters; parsemode = 1 means parsing next param, parsemode = -1 parsing next
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

	/* Read remaining request (we're not interested in the content) */
  while (fgets(tmp, sizeof(tmp), f))  {
    if (tmp[0] == '\r' && tmp[1] == '\n') break;
  }


  //printf("%s %d\n", path, pgidx);
  //for(i=0; i < paramcount; ++i) printf("%s : %s\n", uriparams[i], urivalues[i]);

  fseek(f, 0, SEEK_CUR); // Force change of stream direction


	/*build page*/
  send_headers(f, 200, "OK", NULL, "text/html");
  send_htmlhead(f);
  send_oscam_menu(f);

  switch(pgidx){
    case  0: send_oscam_config(f); break;
    case  1: send_oscam_reader(f); break;
    case  2: send_oscam_user(f); break;
    case  3: send_oscam_entitlement(f, uriparams, urivalues, paramcount); break;
    case  4: send_oscam_status(f); break;
    case  5: send_oscam_user_config(f, uriparams, urivalues, paramcount); break;
    case  6: send_oscam_user_config_do(f, uriparams, urivalues, paramcount); break;
    case  7: send_oscam_reader_config(f, uriparams, urivalues, paramcount); break;
    case  8: send_oscam_reader_config_do(f, uriparams, urivalues, paramcount); break;
    case	9: send_oscam_sidtab(f); break;
    default: send_oscam_status(f); break;
  }

  send_footer(f);
  fprintf(f, "</BODY></HTML>\r\n");

  return 0;
}

void http_srv() {
	int sock;
	struct sockaddr_in sin;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(cfg->http_port);
	bind(sock, (struct sockaddr *) &sin, sizeof(sin));
	listen(sock, 5);
	cs_log("HTTP Server listening on port %d", cfg->http_port);
	while (1)
	{
		int s;
		FILE *f;

		s = accept(sock, NULL, NULL);
		if (s < 0) break;

		f = fdopen(s, "r+");
		process_request(f);
		fclose(f);
  }
  close(sock);
}
