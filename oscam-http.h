#include "globals.h"

#define SERVER "webserver/1.0"
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define AUTHREALM "OScam"
#define AUTHNONCEVALIDSECS 15
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
<TITLE>OSCAM ##CS_VERSION## build ###CS_SVN_VERSION##</TITLE>\n\
<link rel=\"stylesheet\" type=\"text/css\" href=\"site.css\">\n\
##REFRESH##\
</HEAD>\n\
<BODY>\n\
<H2>OSCAM ##CS_VERSION## build ###CS_SVN_VERSION##</H2>"
#define TPLFOOTER "<HR/><H4>OSCAM Webinterface - ##CURDATE## ##CURTIME##</H4></BODY></HTML>"
#define TPLREFRESH "<meta http-equiv=\"refresh\" content=\"##REFRESHTIME##\"; URL=/status.html\" />\n"
#define TPLMENU "<TABLE border=0 class=\"menu\">\n\
<TR>\n\
<TD CLASS=\"menu\"><A HREF=\"status.html\">STATUS</TD>\n\
<TD CLASS=\"menu\"><A HREF=\"config.html\">CONFIGURATION</TD>\n\
<TD CLASS=\"menu\"><A HREF=\"readers.html\">READERS</TD>\n\
<TD CLASS=\"menu\"><A HREF=\"userconfig.html\">USERS</TD>\n\
<TD CLASS=\"menu\"><A HREF=\"services.html\">SERVICES</TD>\n\
</TR>\n\
</TABLE>"
#define TPLSTATUS "##TPLHEADER##\
##TPLMENU##\n\
<BR><BR><TABLE WIDTH=\"100%\" cellspacing=\"0\" class=\"status\">\n\
<TR><TH>PID</TH><TH>Typ</TH><TH>ID</TH><TH>Label</TH><TH>AU</TH><TH>0</TH><TH>Address</TH><TH>Port</TH><TH>Protocol</TH><TH>Login</TH><TH>Login</TH><TH>Time</TH><TH>caid:srvid</TH><TH>Last Channel</TH><TH>Idle</TH><TH>CWOK</TH><TH>CWNOK</TH><TH>0</TH>\n\
##CLIENTSTATUS##\
</TABLE><BR>\n\
<DIV class=\"log\">\n\
##LOGHISTORY##\
</DIV>\n\
##TPLFOOTER##"
#define TPLCLIENTSTATUSBIT "<TR class=\"##CLIENTTYPE##\"><TD>##CLIENTPID##</TD><TD>##CLIENTTYPE##</TD><TD>##CLIENTCNR##</TD><TD>##CLIENTUSER##</TD><TD>##CLIENTCAU##</TD><TD>##CLIENTCRYPTED##</TD><TD>##CLIENTIP##</TD><TD>##CLIENTPORT##</TD><TD>##CLIENTPROTO##</TD><TD>##CLIENTLOGINDATE##</TD><TD>##CLIENTLOGINTIME##</TD><TD>##CLIENTLOGINSECS##</TD><TD>##CLIENTCAID##:##CLIENTSRVID##</TD><TD>##CLIENTSRVNAME##</TD><TD>##CLIENTIDLESECS##</TD><TD>##CWOK##</TD><TD>##CWNOK##</TD><TD>##CLIENTCON##</TD></TR>"
#define TPLUSERCONFIGLIST "##TPLHEADER##\
##TPLMENU##\n\
##MESSAGE##\
<BR><BR><TABLE cellspacing=\"0\" cellpadding=\"10\">\n\
<TR><TH>Label</TH>\r\n\t<TH>Status</TH>\r\n\t<TH>Last Channel</TH>\r\n\t<TH>Idle (Sec)</TH>\r\n\t<TH colspan=\"2\" align=\"center\">Action</TH>\r\n</TR>\
##USERCONFIGS##\
<TR>\n\
\t<FORM action=\"/user_edit.html\" method=\"get\">\n\
\t<TD>New User:</TD>\n\
\t<TD colspan=\"2\"><input name=\"user\" type=\"text\"></TD>\n\
\t<TD colspan=\"3\" align=\"center\"><input type=\"submit\" value=\"Add User\"></TD>\n\
\t</FORM>\n\
<TR>\n\
</TABLE>\n\
##TPLFOOTER##"
#define TPLUSERCONFIGLISTBIT "<TR class=\"##CLASSNAME##\">\n\
\t<TD>##USER##</TD>\n\t<TD>##STATUS####EXPIRED##</TD>\n\t<TD>##LASTCHANNEL##</TD>\r\n\t<TD>##IDLESECS##</TD>\n\t<TD><A HREF=\"user_edit.html?user=##USERENC##\">Edit Settings</A></TD>\n\
\t<TD><A HREF=\"userconfig.html?user=##USERENC##&action=delete\">Delete User</A></TD>\n\
</TR>\n"
#define TPLUSEREDIT "##TPLHEADER##\
##TPLMENU##\n\
##MESSAGE##\
<BR><BR>\n\
<form action=\"/user_edit.html\" method=\"get\">\n\
<input name=\"user\" type=\"hidden\" value=\"##USERNAME##\">\n\
<input name=\"action\" type=\"hidden\" value=\"execute\">\n\
<TABLE cellspacing=\"0\">\n\
<TH>&nbsp;</TH><TH>Edit User ##USERNAME##</TH>\n\
<TR><TD>Password:</TD><TD><input name=\"pwd\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##PASSWORD##\"></TD></TR>\n\
<TR><TD>Exp. Date:</TD><TD><input name=\"expdate\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##EXPDATE##\"></TD></TR>\n\
<TR><TD>Group:</TD><TD><input name=\"group\" type=\"text\" size=\"10\" maxlength=\"10\" value=\"##GROUPS##\"></TD></TR>\n\
<TR><TD>Hostname:</TD><TD><input name=\"hostname\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##DYNDNS##\"></TD></TR>\n\
<TR><TD>Uniq:</TD><TD><select name=\"uniq\">\n\
\t<option value=\"0\" ##UNIQSELECTED0##>none</option>\n\
\t<option value=\"1\" ##UNIQSELECTED1##>strict</option>\n\
\t<option value=\"2\" ##UNIQSELECTED2##>per IP</option>\n\
</SELECT></TD></TR>\n\
<TR><TD>Sleep:</TD><TD><input name=\"sleep\" type=\"text\" size=\"4\" maxlength=\"4\" value=\"0\"></TD></TR>\n\
<TR><TD>Monlevel:</TD><TD><select name=\"monlevel\">\n\
\t<option value=\"0\" ##MONSELECTED0##>no access to monitor</option>\n\
\t<option value=\"1\" ##MONSELECTED1##>only server and own procs</option>\n\
\t<option value=\"2\" ##MONSELECTED2##>all procs, but viewing only, default</option>\n\
\t<option value=\"3\" ##MONSELECTED3##>all procs, reload of oscam.user possible</option>\n\
\t<option value=\"4\" ##MONSELECTED4##>complete access</option>\n\
</select></TD></TR>\n\
<TR><TD>AU:</TD><TD><select name=\"au\">\n\
\t<option value=\" \" ##AUSELECTED##>none</option>\n\
\t<option value=\"1\" ##AUTOAUSELECTED##>auto</option>\n\
##RDROPTION##\
</select></TD></TR>\n\
<TR><TD>Services:</TD><TD><TABLE cellspacing=\"0\" class=\"invisible\">##SIDS##</TD></TR></TABLE>\n\
<TR><TD>CAID:</TD><TD><input name=\"caid\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##CAIDS##\"></TD></TR>\n\
<TR><TD>Ident:</TD><TD><input name=\"ident\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##IDENTS##\"></TD></TR>\n\
<TR><TD>Betatunnel:</TD><TD><input name=\"betatunnel\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##BETATUNNELS##\"></TD></TR>\n\
##TPLUSEREDITANTICASC##\
<TR><TD>&nbsp;</TD><TD align=\"right\"><input type=\"submit\" value=\"Save Settings\" title=\"Save settings and reload users\"></TD></TR>\n\
</TABLE>\n\
</form>\n\
##TPLFOOTER##"
#define TPLUSEREDITRDRSELECTED "\t<option value=\"##READERNAME##\" ##SELECTED##>##READERNAME##</option>"
#define TPLUSEREDITSIDOKBIT "<TR><TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"##SIDLABEL##\" ##CHECKED##> ##SIDLABEL##</TD>"
#define TPLUSEREDITSIDNOBIT "<TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"!##SIDLABEL##\" ##CHECKED##> !##SIDLABEL##</TD></TR>"
#ifdef CS_ANTICASC
#define TPLUSEREDITANTICASC "<TR><TD>Anticascading numusers:</TD><TD><input name=\"numusers\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"##AC_USERS##\"></TD></TR>\n\
<TR><TD>Anticascading penalty:</TD><TD><input name=\"penalty\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"##AC_PENALTY##\"></TD></TR>"
#endif
#define TPLSIDTAB "##TPLHEADER##\
##TPLMENU##\n\
<BR><BR><DIV class=\"log\">\n\
##SIDTABS##\
</DIV>\n\
##TPLFOOTER##"
#define TPLSIDTABBIT "label=##LABEL##<BR>\n\
caid(##CAIDNUM##)=##CAIDS##<BR>\n\
provider(##PROVIDNUM##)=##PROVIDS##<BR>\n\
services(##SRVIDNUM##)=##SRVIDS##<BR><BR>\n"
#define TPLREADERS "##TPLHEADER##\
##TPLMENU##\n\
<BR><BR><TABLE cellspacing=\"0\" cellpadding=\"10\">\n\
<TR><TH>Reader</TH><TH>Protocol</TH><TH>Action</TH></TR>\n\
##READERLIST##\
</TABLE>\n\
##TPLFOOTER##"
#define TPLREADERSBIT "\t<TR><TD>##READERNAME##</TD><TD>##CTYP##</TD><TD><A HREF=\"readerconfig.html?reader=##READERNAMEENC##\">Edit Settings</A> &nbsp; <A HREF=\"entitlements.html?reader=##READERNAME##\">Show Entitlements</A></TD></TR>\n"
#define TPLENTITLEMENTS "##TPLHEADER##\
##TPLMENU##\n\
<BR><BR>Entitlements for ##READERNAME##<BR><BR>\r\n\n\
<DIV class=\"log\">\n\
##LOGHISTORY##\
</DIV>\n\
##TPLFOOTER##"
#define TPLREADERCONFIG "##TPLHEADER##\
##TPLMENU##\n\
<BR><BR>\n\
##MESSAGE##\
<form action=\"/readerconfig.html?action=execute\" method=\"get\"><input name=\"reader\" type=\"hidden\" value=\"##READERNAME##\">\n\
<TABLE cellspacing=\"0\">\n\
<TH>&nbsp;</TH><TH>Edit Reader ##READERNAME##</TH>\n\
<TR><TD>Device:</TD><TD><input name=\"device\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##DEVICE####R_PORT####L_PORT##\"></TD></TR>\n\
<TR><TD>Group:</TD><TD><input name=\"grp\" type=\"text\" size=\"10\" maxlength=\"10\" value=\"##GRP##\"></TD></TR>\n\
<TR><TD>Key:</TD><TD><input name=\"key\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##NCD_KEY##\"></TD></TR>\n\
<TR><TD>Pincode:</TD><TD><input name=\"pincode\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##PINCODE##\"></TD></TR>\n\
<TR><TD>Readnano:</TD><TD><input name=\"readnano\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##EMMFILE##\"></TD></TR>\n\
<TR><TD>Services:</TD><TD><TABLE cellspacing=\"0\" class=\"invisible\">\n\
##SIDS##\
</TD></TR></TABLE>\n\
<TR><TD>Inactivitytimeout:</TD><TD><input name=\"inactivitytimeout\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##INACTIVITYTIMEOUT##\"></TD></TR>\n\
<TR><TD>Reconnecttimeout:</TD><TD><input name=\"reconnecttimeout\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##RECEIVETIMEOUT##\"></TD></TR>\n\
<TR><TD>Disableserverfilter:</TD><TD><input name=\"disableserverfilter\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##DISABLESERVERFILTER##\"></TD></TR>\n\
<TR><TD>Fallback:</TD><TD><input name=\"fallback\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"##FALLBACK##\"></TD></TR>\n\
<TR><TD>CAID:</TD><TD><input name=\"caid\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##CAIDS##\"></TD></TR>\n\
<TR><TD>Boxid:</TD><TD><input name=\"boxid\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##BOXID##\"></TD></TR>\n\
</TABLE>\n\
<input type=\"submit\" value=\"OK\"></form>\n\
<BR><BR>Saving not yet implemented - Nothing changes on click<BR><BR>\n\
##TPLFOOTER##"
#define TPLREADERCONFIGSIDOKBIT "<TR><TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"##SIDLABEL##\" ##CHECKED##> ##SIDLABEL##</TD>"
#define TPLREADERCONFIGSIDNOBIT "<TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"!##SIDLABEL##\" ##CHECKED##> !##SIDLABEL##</TD></TR>"

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
char *getParam(struct uriparams *params, char *name);
