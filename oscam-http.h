#include "globals.h"

#define SERVER "webserver/1.0"
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define AUTHREALM "OScam"
#define AUTHNONCEVALIDSECS 60
#define MAXGETPARAMS 100

#define CSS "p {color: white; }\n\
h2 {color: orange; font-family: Arial; font-size: 14px; line-height: 12px;}\n\
h4 {color: black; font-family: Arial; font-size: 12px; line-height: 9px; }\n\
TABLE{background-color:#66CCFF;}\n\
TD{height:10px; border:1px solid gray; font-family: Arial; font-size: 11px; padding:5px; background-color:#6666FF;}\n\
TH{height:10px; border:1px solid gray; font-family: Arial; font-size: 12px; padding:5px; background-color:#330033;color:#FFFF00;}\n\
TR.s TD{background-color:#6666FF;}\n\
TR.r TD{background-color:orange;}\n\
TR.p TD{background-color:yellow;}\n\
TR.c TD{background-color:green;}\n\
TR.online TD{background-color:#009900;}\n\
TR.expired TD{background-color:orange;}\n\
DIV.log{border:1px solid black;background-color: black; font-family:\"Courier New\", monospace ; color:yellow; font-size: 11px;}\n\
TABLE.menu{background-color:black; align:center; font-size: 10px;}\n\
TABLE.menu TD{border:2px outset lightgrey; background-color:silver; font-color:black; font-family: Arial;}\n\
TABLE.status{background-color:#66CCFF;empty-cells:show;}\n\
TABLE.invisible TD {border:0px; font-family: Arial; font-size: 12px; padding:5px; background-color:#6666FF;}}\n\
TD.menu {border:2px outset lightgrey; background-color:silver; font-color:black; font-family: Arial; font-size:11px;}\n\
body {background-color: grey; font-family: Arial; font-size: 12px;}\n\
A:link {text-decoration: none; color:blue}\n\
A:visited {text-decoration: none; color:blue}\n\
A:active {text-decoration: none; color:white}\n\
A:hover {text-decoration: none; color: red;}"

#define TPLHEADER "<HTML>\n\
<HEAD>\n\
<TITLE>OSCAM ##CS_VERSION## build ##CS_SVN_VERSION##</TITLE>\n\
<link rel=\"stylesheet\" type=\"text/css\" href=\"site.css\">\n\
##REF##\
</HEAD>\n\
<BODY>\n\
<H2>OSCAM ##CS_VERSION## build ##CS_SVN_VERSION##</H2>"
#define TPLFOOTER "<HR/><H4>OSCAM Webinterface - ##CURDATE## ##CURTIME##</H4></BODY></HTML>"
#define TPLREF "<meta http-equiv=\"refresh\" content=\"##REFRESHTIME##\"; URL=/status.html\" />\n"
#define TPLMENU "<TABLE border=0 class=\"menu\">\n\
<TR>\n\
<TD CLASS=\"menu\"><A HREF=\"status.html\">STATUS</TD>\n\
<TD CLASS=\"menu\"><A HREF=\"config.html\">CONFIGURATION</TD>\n\
<TD CLASS=\"menu\"><A HREF=\"readers.html\">READERS</TD>\n\
<TD CLASS=\"menu\"><A HREF=\"userconfig.html\">USERS</TD>\n\
<TD CLASS=\"menu\"><A HREF=\"entitlements.html\">ENTITLEMENTS</TD>\n\
<TD CLASS=\"menu\"><A HREF=\"services.html\">SERVICES</TD>\n\
</TR>\n\
</TABLE>"
#define TPLREFRESH "<meta http-equiv=\"refresh\" content=\"##REFRESHTIME##\"; URL=\"/status.html\" />"
#define TPLSTATUS "##TPLHEADER##\
##TPLMENU##\n\
<BR><BR><TABLE WIDTH=\"100%\" cellspacing=\"0\" class=\"status\">\n\
<TR><TH>PID</TH><TH>Typ</TH><TH>ID</TH><TH>Label</TH><TH>AU</TH><TH>0</TH><TH>Address</TH><TH>Port</TH><TH>Protocol</TH><TH>Login</TH><TH>Login</TH><TH>Time</TH><TH>caid:srvid</TH><TH>&nbsp;</TH><TH>Idle</TH><TH>0</TH>\n\
##CLIENTSTATUS##\n\
</TABLE><BR>\n\
<DIV class=\"log\">\n\
##LOGHISTORY##\n\
</DIV>\n\
##TPLFOOTER##"
#define TPLCLIENTSTATUSBIT "<TR class=\"##CLIENTTYPE##\"><TD>##CLIENTPID##</TD><TD>##CLIENTTYPE##</TD><TD>##CLIENTCNR##</TD><TD>##CLIENTUSER##</TD><TD>##CLIENTCAU##</TD><TD>##CLIENTCRYPTED##</TD><TD>##CLIENTIP##</TD><TD>##CLIENTPORT##</TD><TD>##CLIENTPROTO##</TD><TD>##CLIENTLOGINDATE##</TD><TD>##CLIENTLOGINTIME##</TD><TD>##CLIENTLOGINSECS##</TD><TD>##CLIENTCAID##:##CLIENTSRVID##</TD><TD>##CLIENTSRVNAME##</TD><TD>##CLIENTIDLESECS##</TD><TD>##CLIENTCON##</TD></TR>"

enum refreshtypes {REFR_ACCOUNTS, REFR_READERS, REFR_SERVER, REFR_ANTICASC};

struct templatevars {
	int varscnt;
	int varsalloc;
	int tmpcnt;
	int tmpalloc;
	char **names;
	char **values;
	char **tmp;
};

struct uriparams {
	int paramcount;
	char *params[MAXGETPARAMS];
	char *values[MAXGETPARAMS];
};

static char hex2ascii[256][2];
static char noncekey[33];


char *tpl_addVar(struct templatevars *vars, int append, char *name, char *value);
char *tpl_addTmp(struct templatevars *vars, char *value);
char *tpl_printf(struct templatevars *vars, int append, char *varname, char *fmtstring, ...);
char *tpl_getVar(struct templatevars *vars, char *name);
struct templatevars *tpl_create();
void tpl_clear(struct templatevars *vars);
char *tpl_getUnparsedTpl(const char* name);
char *tpl_getTpl(struct templatevars *vars, const char* name);
char *parse_auth_value(char *value);
void calculate_nonce(char *result, int resultlen);
int check_auth(char *authstring, char *method, char *path, char *expectednonce);
void send_headers(FILE *f, int status, char *title, char *extra, char *mime);
void send_htmlhead(FILE *f, int refresh);
void send_css(FILE *f);
void send_footer(FILE *f);
void send_oscam_menu(FILE *f);
void send_oscam_user_config(FILE *f, struct uriparams *params);
char *getParam(struct uriparams *params, char *name);
