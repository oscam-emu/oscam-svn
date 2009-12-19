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
  cs_log("send header");
  //int c;
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

  //printf("\nAusgabe der kompletten Datei : \n");
  //while( (c=getc(f)) != EOF)
  //putc(c,stdout);
}


void send_htmlhead(FILE *f)
{
  cs_log("send html header");
  int i;
        fprintf(f, "<HTML>\r\n<HEAD>\r\n<TITLE>OSCAM %s build #%s</TITLE>\r\n<STYLE type=\"text/css\">\r\n",CS_VERSION,CS_SVN_VERSION);
	for (i=0; css[i]; i++)
    	fprintf(f, "\t%s\n", css[i]);
        fprintf(f, "</STYLE>\r\n</HEAD>\r\n<BODY>");
        fprintf(f, "<H2>OSCAM %s build #%s</H2>",CS_VERSION,CS_SVN_VERSION);
}

void send_footer(FILE *f)
{
  cs_log("send footer");
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
  cs_log("send menu");
fprintf(f,"<TABLE border=0> <TR> <TD CLASS=\"menu\"><A HREF=\"./config.html\">CONFIGURATION</TD> <TD CLASS=\"menu\"><A HREF=\"./readers.html\">READERS</TD> <TD CLASS=\"menu\"><A HREF=\"./users.html\">USERS</TD> <TD CLASS=\"menu\"><A HREF=\"./entitlements.html\">ENTITLEMENTS</TD> </TR> </TABLE>");
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
  cs_log("reader request");
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
    fprintf(f,"ENTITLEMENT");
    cs_log("entitlement request");
}

int process_request(FILE *f)
{
  char buf[4096];
  char *method;
  char *path;
  char *protocol;

  if (!fgets(buf, sizeof(buf), f)) return -1;
  printf("URL: %s", buf);

  method = strtok(buf, " ");
  path = strtok(NULL, " ");
  protocol = strtok(NULL, "\r");

  if (!method || !path || !protocol) return -1;

  fseek(f, 0, SEEK_CUR); // Force change of stream direction

    send_headers(f, 200, "OK", NULL, "text/html");
    send_htmlhead(f);
    send_oscam_menu(f);

    if(!strcmp(strtolower(path),"/config.html")){
        cs_log("config request");
        send_oscam_config(f);
    }
    else if(!strcmp(strtolower(path),"/readers.html")){
        cs_log("readers request");
        send_oscam_reader(f);
    }
    else if(!strcmp(strtolower(path),"/users.html")){
        cs_log("users request");
        send_oscam_user(f);
    }
    else if(!strcmp(strtolower(path),"/entitlements.html")){
        cs_log("entitlement request");
        send_oscam_entitlement(f);
    }
    else {
        cs_log("unknown request");
        send_oscam_menu(f);
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
