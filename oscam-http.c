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
		"h4 {color: yellow; font-family: Verdana, Arial, Helvetica, sans-serif; font-size: 12px; line-height: 12px; }",
		"TD.menu {background-color: navy; font-family: Verdana, Arial, Helvetica, sans-serif; color: white; font-size: 16px; border: 1px solid green; padding: 5px}",
		"body {background-color: black; font-family: Courier, \"Courier New\", monospace ; color: green; height:100%}",
		"A:link {text-decoration: none; color:white}",
		"A:visited {text-decoration: none; color:white}",
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
	fprintf(f, "<H4>%02d.%02d.%02d %02d:%02d:%02d</H4>\r\n",
										lt->tm_mday, lt->tm_mon+1, lt->tm_year%100,
										lt->tm_hour, lt->tm_min, lt->tm_sec);
}

void send_oscam_menu(FILE *f){

	/*create menue*/
	fprintf(f, "<TABLE border=0>\n");
	fprintf(f, "	<TR>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./status.html\">STATUS</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./config.html\">CONFIGURATION</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./readers.html\">READERS</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./users.html\">USERS</TD>\n");
	fprintf(f, "		<TD CLASS=\"menu\"><A HREF=\"./entitlements.html\">ENTITLEMENTS</TD>\n");
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
	//void send_oscam_config(FILE *f, char *parameter[], int paramlen) {
	//int i;
	fprintf(f,"<BR><H4>CONFIG</H4><BR>");
	//for (i=0; i<paramlen; i++)
	//	fprintf(f,"Parameter %d = %s<BR>",i , parameter[i]);

}

void send_oscam_reader(FILE *f) {
	fprintf(f,"<BR><H4>READER</H4><BR>");
}

void send_oscam_user(FILE *f) {

	/*list accounts*/
	int i = 0;
	int idx = 0;
	struct s_auth *account;
	CAIDTAB *ctab;
	TUNTAB *ttab;

	fprintf(f,"<BR><BR>\r\n");

	for (account=cfg->account; (account) ; account=account->next){

		fprintf(f,"[account]<BR>\r\n");
		fprintf(f,"user= %s<BR>\r\n", account->usr);
		fprintf(f,"pwd= %s<BR>\r\n", account->pwd);

		if (account->grp > 0){
			fprintf(f,"group= ");
			i=0;
			long dez = account->grp;

			while(dez!=0){

				if((dez%2)==1)
					fprintf(f,"%d",i+1);

				dez=dez/2;

				if(dez!=0)
					fprintf(f,",");
				i++;
			}
			fprintf(f,"<BR>\r\n");
		}

		if (account->dyndns[0])
			fprintf(f,"hostname= %s<BR>\r\n", account->dyndns);

		if (account->uniq > 0)
			fprintf(f,"uniq= %d<BR>\r\n", account->uniq);

		if (account->tosleep > 0)
			fprintf(f,"sleep= %d<BR>\r\n", account->tosleep);

		if (account->monlvl > 0)
			fprintf(f,"monlevel= %d<BR>\r\n", account->monlvl);

		if (account->au > -1)
			if (reader[account->au].label[0])
				fprintf(f,"au= %s<BR>\r\n", reader[account->au].label);

		/*make string from caidtab*/
		i = 0;
		ctab = &account->ctab;

		if (ctab->caid[i]){

			fprintf(f,"caid= ");

			while(ctab->caid[i]) {

				if (i==0)
					fprintf(f, "%04X", ctab->caid[i]);
				else
					fprintf(f, ",%04X", ctab->caid[i]);

				if(ctab->mask[i])
					fprintf(f, "&%04X", ctab->mask[i]);

				i++;
			}
			fprintf(f,"<BR>\r\n");
		}

		/*make string from Betatunneltab*/
		i = 0;
		ttab = &account->ttab;

		if (ttab->bt_caidfrom[i]) {

			fprintf(f,"betatunnel= ");

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
			fprintf(f,"<BR>\r\n");
		}


		//Space line between users
		fprintf(f,"<BR>\r\n");
		idx++;
	}
}

void send_oscam_entitlement(FILE *f) {

  /*build entitlements from reader init history*/
	int ridx;
	char *p;

	fprintf(f, "<BR><H4>ENTITLEMENT</H4><BR>\n");

	for (ridx=0; ridx<CS_MAXREADER; ridx++){
		if(!reader[ridx].device[0]) break;
		fprintf(f, "<BR><BR><B>Reader: %s </B><BR>\n", reader[ridx].label);

#ifdef CS_RDR_INIT_HIST

		for (p=(char *)reader[ridx].init_history; *p; p+=strlen(p)+1)
			fprintf(f, "%s<BR>\n",p);

#else

		fprintf(f, "the flag CS_RDR_INIT_HIST is not set in your binary<BR>\n");

#endif
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

	fprintf(f,"<BR><BR><TABLE WIDTH=\"100%\">\n");

	for (i=0; i<CS_MAXPID; i++)
		if (client[i].pid)  {
			fprintf(f,"<TR>");
			monitor_client_status(f, client[i].pid, i);
			fprintf(f,"</TR>\n");
		}

	fprintf(f,"</TABLE><BR>\n");

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
}

int process_request(FILE *f) {
  char buf[4096];
  char *method = NULL;
  char *path = NULL;
  char *protocol = NULL;
  char *uriparams[2];
  char *uriparam[5];
  int paramlen = 0;
  int i;
  int pgidx = -1;

  /*list of possible pages*/
  char *pages[]={	"/config.html",
								"/readers.html",
								"/users.html",
								"/entitlements.html",
								"/status.html"};

  while (fgets(buf, sizeof(buf), f)) {

		/*search for GET in HTTP header*/
    if ( buf[0] == 0x47 && buf[1] == 0x45 && buf[2] == 0x54 ) {
        method = strtok(buf, " ");
        path = strtok(NULL, " ");
        protocol = strtok(NULL, "\r");

        //check for parameters in uri and split on ?
        if (path && (strtoken(path, "?", uriparams) > 0))
        {
        	paramlen = strtoken(uriparams[1], "&", uriparam);
        	path = uriparams[0];
				}

				//find requested page
        for (i = 0; i < 5; i++)
          if (!strcmp(path, pages[i]))
            pgidx = i;

    }

    if (buf[0] == 0x0D && buf[1] == 0x0A) break;
    memset(buf, '\0', sizeof(buf));
  }

  fseek(f, 0, SEEK_CUR); // Force change of stream direction

  if (!method || !path || !protocol) return -1;

  send_headers(f, 200, "OK", NULL, "text/html");
  send_htmlhead(f);
  send_oscam_menu(f);

  switch(pgidx){
    case  0: /*send_oscam_config(f);*/ break;
    case  1: send_oscam_reader(f); break;
    case  2: send_oscam_user(f); break;
    case  3: send_oscam_entitlement(f); break;
    case  4: send_oscam_status(f); break;
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
