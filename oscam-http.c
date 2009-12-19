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

void send_headers(FILE *f, int status, char *title, char *extra, char *mime )
{
  time_t now;
  char timebuf[128];

  fprintf(f, "%s %d %s\r\n", PROTOCOL, status, title);
  fprintf(f, "Server: %s\r\n", SERVER);
  now = time(NULL);
  strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
  fprintf(f, "Date: %s\r\n", timebuf);
  if (extra) fprintf(f, "%s\r\n", extra);
  if (mime) fprintf(f, "Content-Type: %s\r\n", mime);
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
    fprintf(f, "Last-Modified: %s\r\n", timebuf);
  fprintf(f, "Connection: close\r\n");
  fprintf(f, "\r\n");
}


void send_htmlhead(FILE *f)
{
  int i;
        fprintf(f, "<HTML>\r\n<HEAD>\r\n<TITLE>OSCAM %s build #%s</TITLE>\r\n<STYLE type=\"text/css\">\r\n",CS_VERSION,CS_SVN_VERSION);
	for (i=0; css[i]; i++)
    	fprintf(f, "\t%s\n", css[i]);
        fprintf(f, "</STYLE>\r\n</HEAD>\r\n<BODY>");
        fprintf(f, "<H2>OSCAM %s build #%s</H2>",CS_VERSION,CS_SVN_VERSION);
}

void send_footer(FILE *f)
{
  time_t t;
  struct tm *lt;
	time(&t);
	lt=localtime(&t);
	fprintf(f, "<HR/><H4>%02d.%02d.%02d %02d:%02d:%02d</H4>\r\n",
                  lt->tm_mday, lt->tm_mon+1, lt->tm_year%100,
                  lt->tm_hour, lt->tm_min, lt->tm_sec);
}
void send_oscam_menu(FILE *f)
{
fprintf(f,"<TABLE border=0> <TR> <TD CLASS=\"menu\"><A HREF=\"./status.html\">STATUS</TD> <TD CLASS=\"menu\"><A HREF=\"./config.html\">CONFIGURATION</TD> <TD CLASS=\"menu\"><A HREF=\"./readers.html\">READERS</TD> <TD CLASS=\"menu\"><A HREF=\"./users.html\">USERS</TD> <TD CLASS=\"menu\"><A HREF=\"./entitlements.html\">ENTITLEMENTS</TD> </TR> </TABLE>");
}

//void send_error(FILE *f, int status, char *title, char *extra, char *text)
//{
//  send_headers(f, status, title, extra, "text/html");
//  fprintf(f, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\r\n", status, title);
//  fprintf(f, "<BODY><H4>%d %s</H4>\r\n", status, title);
//  fprintf(f, "%s\r\n", text);
//  fprintf(f, "</BODY></HTML>\r\n");
//}

void send_oscam_config(FILE *f)
{
  fprintf(f,"CONFIG");
}

void send_oscam_reader(FILE *f)
{
  fprintf(f,"READER");
}

void send_oscam_user(FILE *f)
{
  struct s_auth *account;

  //list accounts
  for (account=cfg->account; (account) ; account=account->next){
    fprintf(f,"<b>User: %s<b><br>\r\n", account->usr);
    fprintf(f,"Hostname: %s<br>\r\n", account->dyndns);
    fprintf(f,"Uniq: %d<br>\r\n", account->uniq);
    fprintf(f,"Sleep: %d<br>\r\n", account->tosleep);
    fprintf(f,"Monlevel: %d<br>\r\n", account->monlvl);
    fprintf(f,"<br>\r\n");
  }
}

void send_oscam_entitlement(FILE *f)
{
  int ridx;
  char *p;

  fprintf(f,"<br><br><b>ENTITLEMENT</b><br>\n");

  for (ridx=0; ridx<CS_MAXREADER; ridx++){
    if(!reader[ridx].device[0]) break;
    fprintf(f,"<br><br><b>Reader: %s </b><br>\n",reader[ridx].label);
    for (p=(char *)reader[ridx].init_history; *p; p+=strlen(p)+1)
      fprintf(f,"%s<br>\n",p);
  }
}

void monitor_client_status(FILE *f, char id, int i){

  if (client[i].pid)
  {
    char ldate[16], ltime[16], *usr;
    int lsec, isec, cnr, con, cau;
    time_t now;
    struct tm *lt;
    now=time((time_t)0);

    if ((cfg->mon_hideclient_to <= 0) ||
        (((now-client[i].lastecm)/60)<cfg->mon_hideclient_to) ||
        (((now-client[i].lastemm)/60)<cfg->mon_hideclient_to) ||
        (client[i].typ!='c'))
    {
      lsec=now-client[i].login;
      isec=now-client[i].last;
      usr=client[i].usr;
      if (((client[i].typ=='r') || (client[i].typ=='p')) &&
          (con=cs_idx2ridx(i))>=0)
        usr=reader[con].label;
      if (client[i].dup)
        con=2;
      else
        if ((client[i].tosleep) &&
            (now-client[i].lastswitch>client[i].tosleep))
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
      sprintf(ldate, "%2d.%02d.%02d",
                     lt->tm_mday, lt->tm_mon+1, lt->tm_year % 100);
      sprintf(ltime, "%2d:%02d:%02d",
                     lt->tm_hour, lt->tm_min, lt->tm_sec);
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
	fprintf(f,"<BR><BR><TABLE WIDTH='100%'>\n");
	for (i=0; i<CS_MAXPID; i++)
		if (client[i].pid)  {
			fprintf(f,"<TR>");
			monitor_client_status(f, client[i].pid, i);
			fprintf(f,"</TR>\n");
		}
	fprintf(f,"</TABLE><BR>\n");

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
}

int process_request(FILE *f)
{
  char buf[4096];
  char *method;
  char *path;
  char *protocol;
  char *cmd[]={"/config.html", "/readers.html", "/users.html", "/entitlements.html", "/status.html"};
  int pgidx = -1;
  int i;

  while (fgets(buf, sizeof(buf), f))  {

    if ( buf[0] == 0x47 && buf[1] == 0x45 && buf[2] == 0x54 ) {
        method = strtok(buf, " ");
        path = strtok(NULL, " ");
        protocol = strtok(NULL, "\r");

        for (i=0; i<5; i++)
          if (!strcmp(path, cmd[i]))
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
    case  0: send_oscam_config(f); break;
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

void http_srv()
{
  int sock;
  struct sockaddr_in sin;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(cfg->http_port);
  bind(sock, (struct sockaddr *) &sin, sizeof(sin));
  listen(sock, 5);
  cs_log("HTTP Server listening on port %d",cfg->http_port);
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
