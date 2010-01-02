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

#define TPLHEADER "\
<HTML>\n\
  <HEAD>\n\
    <TITLE>OSCAM ##CS_VERSION## build ###CS_SVN_VERSION##</TITLE>\n\
    <link rel=\"stylesheet\" type=\"text/css\" href=\"site.css\">\n\
    ##REFRESH##\
  </HEAD>\n\
  <BODY>\n\
    <H2>OSCAM ##CS_VERSION## build ###CS_SVN_VERSION##</H2>"

#define TPLFOOTER "\
  <HR/><H4>OSCAM Webinterface - ##CURDATE## ##CURTIME##</H4>\
  </BODY>\
</HTML>"

#define TPLREFRESH "\
<meta http-equiv=\"refresh\" content=\"##REFRESHTIME##\"; URL=/status.html\" />\n"

#define TPLMENU "\
  <TABLE border=0 class=\"menu\">\n\
    <TR>\n\
      <TD CLASS=\"menu\"><A HREF=\"status.html\">STATUS</TD>\n\
      <TD CLASS=\"menu\"><A HREF=\"config.html\">CONFIGURATION</TD>\n\
      <TD CLASS=\"menu\"><A HREF=\"readers.html\">READERS</TD>\n\
      <TD CLASS=\"menu\"><A HREF=\"userconfig.html\">USERS</TD>\n\
      <TD CLASS=\"menu\"><A HREF=\"services.html\">SERVICES</TD>\n\
    </TR>\n\
  </TABLE>"

#define TPLCONFIGMENU "\
	<BR><BR>\n\
	<TABLE border=0 class=\"menu\">\n\
		<TR>\n\
			<TD CLASS=\"menu\"><A HREF=\"config.html?part=global\">Global</TD>\n\
			<TD CLASS=\"menu\"><A HREF=\"config.html?part=camd33\">Camd3.3</TD>\n\
			<TD CLASS=\"menu\"><A HREF=\"config.html?part=camd35\">Camd3.5</TD>\n\
			<TD CLASS=\"menu\"><A HREF=\"config.html?part=newcamd\">Newcamd</TD>\n\
			<TD CLASS=\"menu\"><A HREF=\"config.html?part=radegast\">Radegast</TD>\n\
			<TD CLASS=\"menu\"><A HREF=\"config.html?part=cccam\">Cccam</TD>\n\
			<TD CLASS=\"menu\"><A HREF=\"config.html?part=gbox\">Gbox</TD>\n\
			##TPLCONFIGMENUANTICASC##\
			<TD CLASS=\"menu\"><A HREF=\"config.html?part=monitor\">Monitor</TD>\n\
		</TR>\n\
	</TABLE>"

#ifdef CS_ANTICASC
#define TPLCONFIGMENUANTICASC "<TD CLASS=\"menu\"><A HREF=\"config.html?part=anticasc\">Anticascading</TD>\n"
#endif

#define TPLSTATUS "\
  ##TPLHEADER##\
  ##TPLMENU##\n\
  <BR><BR>\n\
  <TABLE WIDTH=\"100%\" cellspacing=\"0\" class=\"status\">\n\
    <TR>\n\
      <TH>PID</TH>\n\
      <TH>Typ</TH>\n\
      <TH>ID</TH>\n\
      <TH>Label</TH>\n\
      <TH>AU</TH>\n\
      <TH>0</TH>\n\
      <TH>Address</TH>\n\
      <TH>Port</TH>\n\
      <TH>Protocol</TH>\n\
      <TH>Login</TH>\n\
      <TH>Login</TH>\n\
      <TH>Time</TH>\n\
      <TH>caid:srvid</TH>\n\
      <TH>Last Channel</TH>\n\
      <TH>Idle</TH>\n\
      <TH>CWOK</TH>\n\
      <TH>CWNOK</TH>\n\
      <TH>0</TH>\n\
    </TR>\n\
    ##CLIENTSTATUS##\
  </TABLE><BR>\n\
  <DIV class=\"log\">\n\
  ##LOGHISTORY##\
  </DIV>\n\
  ##TPLFOOTER##"

#define TPLCLIENTSTATUSBIT "\
 <TR class=\"##CLIENTTYPE##\">\n\
  <TD>##CLIENTPID##</TD>\n\
  <TD>##CLIENTTYPE##</TD>\n\
  <TD>##CLIENTCNR##</TD>\n\
  <TD>##CLIENTUSER##</TD>\n\
  <TD>##CLIENTCAU##</TD>\n\
  <TD>##CLIENTCRYPTED##</TD>\n\
  <TD>##CLIENTIP##</TD>\n\
  <TD>##CLIENTPORT##</TD>\n\
  <TD>##CLIENTPROTO##</TD>\n\
  <TD>##CLIENTLOGINDATE##</TD>\n\
  <TD>##CLIENTLOGINTIME##</TD>\n\
  <TD>##CLIENTLOGINSECS##</TD>\n\
  <TD>##CLIENTCAID##:##CLIENTSRVID##</TD>\n\
  <TD>##CLIENTSRVNAME##</TD>\n\
  <TD>##CLIENTIDLESECS##</TD>\n\
  <TD>##CWOK##</TD>\n\
  <TD>##CWNOK##</TD>\n\
  <TD>##CLIENTCON##</TD>\n\
 </TR>\n"

#define TPLUSERCONFIGLIST "\
  ##TPLHEADER##\
  ##TPLMENU##\n\
  ##MESSAGE##\
  <BR><BR>\
  <TABLE cellspacing=\"0\" cellpadding=\"10\">\n\
    <TR>\n\
      <TH>Label</TH>\n\
      <TH>Status</TH>\n\
      <TH>Last Channel</TH>\n\
      <TH>Idle (Sec)</TH>\n\
      <TH colspan=\"2\" align=\"center\">Action</TH>\n\
    </TR>\n\
    ##USERCONFIGS##\
    <TR>\n\
      <FORM action=\"user_edit.html\" method=\"get\">\n\
      <TD>New User:</TD>\n\
      <TD colspan=\"2\"><input name=\"user\" type=\"text\"></TD>\n\
      <TD colspan=\"3\" align=\"center\"><input type=\"submit\" value=\"Add User\"></TD>\n\
      </FORM>\n\
    <TR>\n\
  </TABLE>\n\
  ##TPLFOOTER##"

#define TPLUSERCONFIGLISTBIT "\
  <TR class=\"##CLASSNAME##\">\n\
    <TD>##USER##</TD>\n\
    <TD>##STATUS####EXPIRED##</TD>\n\
    <TD>##LASTCHANNEL##</TD>\n\
    <TD>##IDLESECS##</TD>\n\
    <TD><A HREF=\"user_edit.html?user=##USERENC##\">Edit Settings</A></TD>\n\
    <TD><A HREF=\"userconfig.html?user=##USERENC##&action=delete\">Delete User</A></TD>\n\
  </TR>\n"

#define TPLUSEREDIT "\
##TPLHEADER##\
##TPLMENU##\n\
##MESSAGE##\
<BR><BR>\n\
  <form action=\"user_edit.html\" method=\"get\">\n\
  <input name=\"user\" type=\"hidden\" value=\"##USERNAME##\">\n\
  <TABLE cellspacing=\"0\">\n\
    <TR>\n\
      <TH>&nbsp;</TH>\n\
      <TH>Edit User ##USERNAME##</TH>\n\
    <TR>\n\
      <TD>Password:</TD>\n\
      <TD><input name=\"pwd\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##PASSWORD##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Exp. Date:</TD>\n\
      <TD><input name=\"expdate\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##EXPDATE##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Group:</TD>\n\
      <TD><input name=\"group\" type=\"text\" size=\"10\" maxlength=\"10\" value=\"##GROUPS##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Hostname:</TD>\n\
      <TD><input name=\"hostname\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##DYNDNS##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Uniq:</TD>\n\
      <TD><select name=\"uniq\">\n\
        <option value=\"0\" ##UNIQSELECTED0##>none</option>\n\
        <option value=\"1\" ##UNIQSELECTED1##>strict</option>\n\
        <option value=\"2\" ##UNIQSELECTED2##>per IP</option>\n\
      </SELECT></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Sleep:</TD>\n\
      <TD><input name=\"sleep\" type=\"text\" size=\"4\" maxlength=\"4\" value=\"##SLEEP##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Monlevel:</TD>\n\
      <TD><select name=\"monlevel\">\n\
        <option value=\"0\" ##MONSELECTED0##>no access to monitor</option>\n\
        <option value=\"1\" ##MONSELECTED1##>only server and own procs</option>\n\
        <option value=\"2\" ##MONSELECTED2##>all procs, but viewing only, default</option>\n\
        <option value=\"3\" ##MONSELECTED3##>all procs, reload of oscam.user possible</option>\n\
        <option value=\"4\" ##MONSELECTED4##>complete access</option>\n\
      </select></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>AU:</TD>\n\
      <TD><select name=\"au\">\n\
        <option value=\" \" ##AUSELECTED##>none</option>\n\
        <option value=\"1\" ##AUTOAUSELECTED##>auto</option>\n\
        ##RDROPTION##\
      </select></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Services:</TD>\n\
      <TD>\n\
        <TABLE cellspacing=\"0\" class=\"invisible\">##SIDS##\
            </TD>\n\
          </TR>\n\
        </TABLE>\n\
    <TR>\n\
      <TD>CAID:</TD>\n\
      <TD><input name=\"caid\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##CAIDS##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Ident:</TD>\n\
      <TD><input name=\"ident\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##IDENTS##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Betatunnel:</TD>\n\
      <TD><input name=\"betatunnel\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##BETATUNNELS##\"></TD>\n\
    </TR>\n\
    ##TPLUSEREDITANTICASC##\
    <TR>\n\
      <TD align=\"center\"><input type=\"submit\" name=\"action\" value=\"Save\" title=\"Save settings and reload users\"></TD>\n\
      <TD align=\"center\"><input name=\"newuser\" type=\"text\" size=\"20\" maxlength=\"20\" title=\"Enter new username if you want to clone this user\">&nbsp;&nbsp;&nbsp;<input type=\"submit\" name=\"action\" value=\"Save As\" title=\"Save as new user and reload users\"></TD>\n\
    </TR>\n\
  </TABLE>\n\
</form>\n\
##TPLFOOTER##"

#define TPLUSEREDITRDRSELECTED "\t<option value=\"##READERNAME##\" ##SELECTED##>##READERNAME##</option>"

#define TPLUSEREDITSIDOKBIT "\
          <TR>\n\
            <TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"##SIDLABEL##\" ##CHECKED##> ##SIDLABEL##</TD>\n"

#define TPLUSEREDITSIDNOBIT "\
            <TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"!##SIDLABEL##\" ##CHECKED##> !##SIDLABEL##</TD>\n\
          </TR>\n"

#ifdef CS_ANTICASC
# define TPLUSEREDITANTICASC "\
    <TR>\n\
      <TD>Anticascading numusers:</TD>\n\
      <TD><input name=\"numusers\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"##AC_USERS##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Anticascading penalty:</TD>\n\
      <TD><input name=\"penalty\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"##AC_PENALTY##\"></TD>\n\
    </TR>\n"
#endif

#define TPLSIDTAB "\
  ##TPLHEADER##\
  ##TPLMENU##\n\
  <BR><BR><DIV class=\"log\">\n\
  ##SIDTABS##\
  </DIV>\n\
  ##TPLFOOTER##"

#define TPLSIDTABBIT "\
label=##LABEL##<BR>\n\
caid(##CAIDNUM##)=##CAIDS##<BR>\n\
provider(##PROVIDNUM##)=##PROVIDS##<BR>\n\
services(##SRVIDNUM##)=##SRVIDS##<BR><BR>\n"

#define TPLREADERS "\
##TPLHEADER##\
##TPLMENU##\n\
<BR><BR>\n\
  <TABLE cellspacing=\"0\" cellpadding=\"10\">\n\
    <TR>\n\
      <TH>Reader</TH>\n\
      <TH>Protocol</TH>\n\
      <TH>Action</TH>\n\
    </TR>\n\
    ##READERLIST##\
  </TABLE>\n\
##TPLFOOTER##"

#define TPLREADERSBIT "\
    <TR>\n\
      <TD>##READERNAME##</TD>\n\
      <TD>##CTYP##</TD>\n\
      <TD><A HREF=\"readerconfig.html?reader=##READERNAMEENC##\">Edit Settings</A> &nbsp;|&nbsp; <A HREF=\"entitlements.html?reader=##READERNAME##\">Show Entitlements</A></TD>\n\
      </TR>\n"

#define TPLENTITLEMENTS "\
##TPLHEADER##\
##TPLMENU##\n\
<BR><BR>Entitlements for ##READERNAME##<BR><BR>\n\n\
<DIV class=\"log\">\n\
  ##LOGHISTORY##\
</DIV>\n\
##TPLFOOTER##"

#define TPLREADERCONFIG "\
##TPLHEADER##\
##TPLMENU##\n\
<BR><BR>\n\
##MESSAGE##\
  <form action=\"readerconfig.html?action=execute\" method=\"get\"><input name=\"reader\" type=\"hidden\" value=\"##READERNAME##\">\n\
  <TABLE cellspacing=\"0\">\n\
    <TR>\n\
      <TH>&nbsp;</TH>\n\
      <TH>Edit Reader ##READERNAME##</TH>\n\
    </TR>\n\
    <TR>\n\
      <TD>Device:</TD>\n\
      <TD><input name=\"device\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##DEVICE####R_PORT####L_PORT##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Group:</TD>\n\
      <TD><input name=\"grp\" type=\"text\" size=\"10\" maxlength=\"10\" value=\"##GRP##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Key:</TD>\n\
      <TD><input name=\"key\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##NCD_KEY##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Pincode:</TD>\n\
      <TD><input name=\"pincode\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##PINCODE##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Readnano:</TD>\n\
      <TD><input name=\"readnano\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##EMMFILE##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Services:</TD>\n\
      <TD>\n\
        <TABLE cellspacing=\"0\" class=\"invisible\">##SIDS##\
            </TD>\n\
          </TR>\n\
        </TABLE>\n\
    <TR>\n\
      <TD>Inactivitytimeout:</TD>\n\
      <TD><input name=\"inactivitytimeout\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##INACTIVITYTIMEOUT##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Reconnecttimeout:</TD>\n\
      <TD><input name=\"reconnecttimeout\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##RECEIVETIMEOUT##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Disableserverfilter:</TD>\n\
      <TD><input name=\"disableserverfilter\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##DISABLESERVERFILTER##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Fallback:</TD>\n\
      <TD><input name=\"fallback\" type=\"text\" size=\"3\" maxlength=\"3\" value=\"##FALLBACK##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>CAID:</TD>\n\
      <TD><input name=\"caid\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##CAIDS##\"></TD>\n\
    </TR>\n\
    <TR>\n\
      <TD>Boxid:</TD>\n\
      <TD><input name=\"boxid\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"##BOXID##\"></TD>\n\
    </TR>\n\
  </TABLE>\n\
  <input type=\"submit\" value=\"OK\"></form>\n\
<BR><BR>Saving not yet implemented - Nothing changes on click<BR><BR>\n\
##TPLFOOTER##"
#define TPLSAVETEMPLATES "##TPLHEADER##\
##TPLMENU##\n\
<br><b>Saved ##CNT## templates to ##PATH##</b><br>\n\
##TPLFOOTER##"
#define TPLREADERCONFIGSIDOKBIT "\
          <TR>\n\
            <TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"##SIDLABEL##\" ##CHECKED##> ##SIDLABEL##</TD>\n"

#define TPLREADERCONFIGSIDNOBIT "\
            <TD><INPUT NAME=\"services\" TYPE=\"CHECKBOX\" VALUE=\"!##SIDLABEL##\" ##CHECKED##> !##SIDLABEL##</TD>\n\
          </TR>\n"

#define TPLCONFIGGBOX "\
##TPLHEADER##\
##TPLMENU##\n\
##TPLCONFIGMENU##\n\
<BR><BR>\n\
##MESSAGE##\
<form action=\"config.html\" method=\"get\">\n\
	<input name=\"part\" type=\"hidden\" value=\"gbox\">\n\
	<input name=\"action\" type=\"hidden\" value=\"execute\">\n\
	<TABLE cellspacing=\"0\">\n\
		<TR><TH>&nbsp;</TH><TH>Edit Gbox Config </TH></TR>\n\
		<TR><TD>Password:</TD><TD><input name=\"password\" type=\"text\" size=\"10\" maxlength=\"8\" value=\"##PASSWORD##\"></TD></TR>\n\
		<TR><TD>Maxdist:</TD><TD><input name=\"maxdist\" type=\"text\" size=\"5\" maxlength=\"2\" value=\"##MAXDIST##\"></TD></TR>\n\
		<TR><TD>Ignorelist:</TD><TD><input name=\"ignorelist\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##IGNORELIST##\"></TD></TR>\n\
		<TR><TD>Onlineinfos:</TD><TD><input name=\"onlineinfos\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##ONLINEINFOS##\"></TD></TR>\n\
		<TR><TD>Cardinfos:</TD><TD><input name=\"cardinfos\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##CARDINFOS##\"></TD></TR>\n\
		<TR><TD>Locals:</TD><TD><input name=\"locals\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##LOCALS##\"></TD></TR>\n\
	</TABLE>\n\
<input type=\"submit\" value=\"OK\"></form>\n\
##TPLFOOTER##"

#ifdef CS_ANTICASC
#define TPLCONFIGANTICASC "\
##TPLHEADER##\
##TPLMENU##\n\
##TPLCONFIGMENU##\n\
<BR><BR>\n\
##MESSAGE##\
<form action=\"config.html\" method=\"get\">\n\
	<input name=\"part\" type=\"hidden\" value=\"anticasc\">\n\
	<input name=\"action\" type=\"hidden\" value=\"execute\">\n\
	<TABLE cellspacing=\"0\">\n\
		<TR><TH>&nbsp;</TH><TH>Edit Anticascading Config</TH></TR>\n\
		<TR><TD>Enabled:</TD><TD><input name=\"enabled\" type=\"checkbox\" value=\"1\" ##CHECKED##>\n\
		<TR><TD>Numusers:</TD><TD><input name=\"numusers\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##NUMUSERS##\"></TD></TR>\n\
		<TR><TD>Sampletime:</TD><TD><input name=\"sampletime\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##SAMPLETIME##\"></TD></TR>\n\
		<TR><TD>Samples:</TD><TD><input name=\"samples\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##SAMPLES##\"></TD></TR>\n\
		<TR><TD>Penalty:</TD><TD><input name=\"penalty\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##PENALTY##\"></TD></TR>\n\
		<TR><TD>AClogfile:</TD><TD><input name=\"aclogfile\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##ACLOGFILE##\"></TD></TR>\n\
		<TR><TD>Fakedelay:</TD><TD><input name=\"fakedelay\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##FAKEDELAY##\"></TD></TR>\n\
		<TR><TD>Denysamples:</TD><TD><input name=\"denysamples\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##DENYSAMPLES##\"></TD></TR>\n\
	</TABLE>\n\
	<input type=\"submit\" value=\"OK\">\n\
</form>\n\
##TPLFOOTER##"
#endif

#define TPLCONFIGCCCAM "\
##TPLHEADER##\
##TPLMENU##\n\
##TPLCONFIGMENU##\n\
<BR><BR>\n\
##MESSAGE##\
<form action=\"config.html\" method=\"get\">\n\
	<input name=\"part\" type=\"hidden\" value=\"cccam\">\n\
	<input name=\"action\" type=\"hidden\" value=\"execute\">\n\
	<TABLE cellspacing=\"0\">\n\
		<TR><TH>&nbsp;</TH><TH>Edit Cccam Config</TH></TR>\n\
	</TABLE>\n\
	<input type=\"submit\" value=\"OK\">\n\
</form>\n\
<BR><BR>Configuration Cccam not yet implemented<BR><BR>\n\
##TPLFOOTER##"

#define TPLCONFIGMONITOR "\
##TPLHEADER##\
##TPLMENU##\n\
##TPLCONFIGMENU##\n\
<BR><BR>\n\
##MESSAGE##\
<form action=\"config.html\" method=\"get\">\n\
	<input name=\"part\" type=\"hidden\" value=\"monitor\">\n\
	<input name=\"action\" type=\"hidden\" value=\"execute\">\n\
	<TABLE cellspacing=\"0\">\n\
		<TR><TH>&nbsp;</TH><TH>Edit Monitor Config</TH></TR>\n\
		<TR><TD>Port:</TD><TD><input name=\"port\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##MONPORT##\"></TD></TR>\n\
		<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##SERVERIP##\"></TD></TR>\n\
		<TR><TD>Nocrypt:</TD><TD><input name=\"nocrypt\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##NOCRYPT##\">wrong, see Ticket #265</TD></TR>\n\
		<TR><TD>Aulow:</TD><TD><input name=\"aulow\" type=\"text\" size=\"5\" maxlength=\"1\" value=\"##AULOW##\"></TD></TR>\n\
		<TR>\n\
			<TD>Monlevel:</TD>\n\
	    <TD><select name=\"monlevel\">\n\
				<option value=\"0\" ##MONSELECTED0##>no access to monitor</option>\n\
				<option value=\"1\" ##MONSELECTED1##>only server and own procs</option>\n\
				<option value=\"2\" ##MONSELECTED2##>all procs, but viewing only, default</option>\n\
				<option value=\"3\" ##MONSELECTED3##>all procs, reload of oscam.user possible</option>\n\
				<option value=\"4\" ##MONSELECTED4##>complete access</option>\n\
			</select></TD>\n\
		</TR>\n\
		<TR><TD>Hideclientto:</TD><TD><input name=\"hideclient_to\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##HIDECLIENTTO##\"></TD></TR>\n\
		<TR><TD>Httpport:</TD><TD><input name=\"httpport\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##HTTPPORT##\"></TD></TR>\n\
		<TR><TD>Httpuser:</TD><TD><input name=\"httpuser\" type=\"text\" size=\"20\" maxlength=\"20\" value=\"##HTTPUSER##\"></TD></TR>\n\
		<TR><TD>Httppwd:</TD><TD><input name=\"httppwd\" type=\"text\" size=\"20\" maxlength=\"20\" value=\"##HTTPPASSWORD##\"></TD></TR>\n\
		<TR><TD>Httpcss:</TD><TD><input name=\"httpcss\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##HTTPCSS##\"></TD></TR>\n\
		<TR><TD>Httprefresh:</TD><TD><input name=\"httprefresh\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##HTTPREFRESH##\"></TD></TR>\n\
		<TR><TD>Httptpl:</TD><TD><input name=\"httptpl\" type=\"text\" size=\"50\" maxlength=\"100\" value=\"##HTTPTPL##\"></TD></TR>\n\
	</TABLE>\n\
	<input type=\"submit\" value=\"OK\">\n\
</form>\n\
##TPLFOOTER##"

#define TPLCONFIGRADEGAST "\
##TPLHEADER##\
##TPLMENU##\n\
##TPLCONFIGMENU##\n\
<BR><BR>\n\
##MESSAGE##\
<form action=\"config.html\" method=\"get\">\n\
	<input name=\"part\" type=\"hidden\" value=\"radegast\">\n\
	<input name=\"action\" type=\"hidden\" value=\"execute\">\n\
	<TABLE cellspacing=\"0\">\n\
		<TR><TH>&nbsp;</TH><TH>Edit Radegast Config</TH></TR>\n\
		<TR><TD>Port:</TD><TD><input name=\"port\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##PORT##\"></TD></TR>\n\
		<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##SERVERIP##\"></TD></TR>\n\
		<TR><TD>Allowed:</TD><TD><input name=\"allowed\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##ALLOWED##\">wrong, see Ticket #265</TD></TR>\n\
		<TR><TD>User:</TD><TD><input name=\"user\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##USER##\"></TD></TR>\n\
	</TABLE>\n\
	<input type=\"submit\" value=\"OK\">\n\
</form>\n\
##TPLFOOTER##"

#define TPLCONFIGNEWCAMD "\
##TPLHEADER##\
##TPLMENU##\n\
##TPLCONFIGMENU##\n\
<BR><BR>\n\
##MESSAGE##\
<form action=\"config.html\" method=\"get\">\n\
	<input name=\"part\" type=\"hidden\" value=\"newcamd\">\n\
	<input name=\"action\" type=\"hidden\" value=\"execute\">\n\
	<TABLE cellspacing=\"0\">\n\
		<TR><TH>&nbsp;</TH><TH>Edit Newcamd Config</TH></TR>\n\
		<TR><TD>Port:</TD><TD><input name=\"port\" type=\"text\" size=\"80\" maxlength=\"200\" value=\"##PORT##\"></TD></TR>\n\
		<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##SERVERIP##\"></TD></TR>\n\
		<TR><TD>Key:</TD><TD><input name=\"key\" type=\"text\" size=\"35\" maxlength=\"28\" value=\"##KEY##\"></TD></TR>\n\
	</TABLE>\n\
	<input type=\"submit\" value=\"OK\">\n\
</form>\n\
##TPLFOOTER##"

#define TPLCONFIGGLOBAL "\
##TPLHEADER##\
##TPLMENU##\n\
##TPLCONFIGMENU##\n\
<BR><BR>\n\
##MESSAGE##\
<form action=\"config.html\" method=\"get\">\n\
	<input name=\"part\" type=\"hidden\" value=\"global\">\n\
	<input name=\"action\" type=\"hidden\" value=\"execute\">\n\
	<TABLE cellspacing=\"0\">\n\
		<TR><TH>&nbsp;</TH><TH>Edit Global Config</TH></TR>\n\
		<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##SERVERIP##\"></TD></TR>\n\
		<TR><TD>Logfile:</TD><TD><input name=\"logfile\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##LOGFILE##\"></TD></TR>\n\
		<TR><TD>PID File:</TD><TD><input name=\"pidfile\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##PIDFILE##\"></TD></TR>\n\
		<TR><TD>Usrfile:</TD><TD><input name=\"usrfile\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##USERFILE##\"></TD></TR>\n\
		<TR><TD>CWlogdir:</TD><TD><input name=\"cwlogdir\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##CWLOGDIR##\"></TD></TR>\n\
		<TR><TD>Clienttimeout:</TD><TD><input name=\"clienttimeout\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##CLIENTTIMEOUT##\"></TD></TR>\n\
		<TR><TD>Fallbacktimeout:</TD><TD><input name=\"fallbacktimeout\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##FALLBACKTIMEOUT##\"></TD></TR>\n\
		<TR><TD>Clientmaxidle:</TD><TD><input name=\"clientmaxidle\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##CLIENTMAXIDLE##\"></TD></TR>\n\
		<TR><TD>Cachedelay:</TD><TD><input name=\"cachedelay\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##CACHEDELAY##\"></TD></TR>\n\
		<TR><TD>Bindwait:</TD><TD><input name=\"bindwait\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##BINDWAIT##\"></TD></TR>\n\
		<TR><TD>Netprio:</TD><TD><input name=\"netprio\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##NETPRIO##\"></TD></TR>\n\
		<TR><TD>Resolvedelay:</TD><TD><input name=\"resolvedelay\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##RESOLVEDELAY##\"></TD></TR>\n\
		<TR><TD>Sleep:</TD><TD><input name=\"sleep\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##SLEEP##\"></TD></TR>\n\
		<TR><TD>Unlockparental:</TD><TD><input name=\"unlockparental\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##UNLOCKPARENTAL##\"></TD></TR>\n\
		<TR><TD>Nice:</TD><TD><input name=\"nice\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##NICE##\"></TD></TR>\n\
		<TR><TD>Serialreadertimeout:</TD><TD><input name=\"serialreadertimeout\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##SERIALTIMEOUT##\"></TD></TR>\n\
		<TR><TD>Maxlogsize:</TD><TD><input name=\"maxlogsize\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##MAXLOGSIZE##\"></TD></TR>\n\
		<TR><TD>Showecmdw:</TD><TD><input name=\"showecmdw\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##SHOWECMDW##\"></TD></TR>\n\
		<TR><TD>Waitforcards:</TD><TD><input name=\"waitforcards\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##WAITFORCARDS##\"></TD></TR>\n\
		<TR><TD>Preferlocalcards:</TD><TD><input name=\"preferlocalcards\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##PREFERLOCALCARDS##\"></TD></TR>\n\
	</TABLE>\n\
	<input type=\"submit\" value=\"OK\">\n\
</form>\n\
<BR><BR>Configuration Global not yet implemented chengings havn't any effect<BR><BR>\n\
##TPLFOOTER##"

#define TPLCONFIGCAMD33 "\
##TPLHEADER##\
##TPLMENU##\n\
##TPLCONFIGMENU##\n\
<BR><BR>\n\
##MESSAGE##\
<form action=\"config.html\" method=\"get\">\n\
	<input name=\"part\" type=\"hidden\" value=\"camd33\">\n\
	<input name=\"action\" type=\"hidden\" value=\"execute\">\n\
	<TABLE cellspacing=\"0\">\n\
		<TR><TH>&nbsp;</TH><TH>Edit Camd33 Config</TH></TR>\n\
		<TR><TD>Port:</TD><TD><input name=\"port\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##PORT##\"></TD></TR>\n\
		<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##SERVERIP##\"></TD></TR>\n\
		<TR><TD>Key:</TD><TD><input name=\"key\" type=\"text\" size=\"35\" maxlength=\"28\" value=\"##KEY##\"></TD></TR>\n\
		<TR><TD>Passive:</TD><TD><input name=\"passive\" type=\"text\" size=\"3\" maxlength=\"1\" value=\"##PASSIVE##\"></TD></TR>\n\
		<TR><TD>Nocrypt:</TD><TD><input name=\"nocrypt\" type=\"text\" size=\"50\" maxlength=\"50\" value=\"##NOCRYPT##\">wrong, see Ticket #265</TD></TR>\n\
	</TABLE>\n\
	<input type=\"submit\" value=\"OK\">\n\
</form>\n\
<BR><BR>Configuration camd33 not yet implemented<BR><BR>\n\
##TPLFOOTER##"

#define TPLCONFIGCAMD35 "\
##TPLHEADER##\
##TPLMENU##\n\
##TPLCONFIGMENU##\n\
<BR><BR>\n\
##MESSAGE##\
<form action=\"/config.html\" method=\"get\">\n\
	<input name=\"part\" type=\"hidden\" value=\"camd35\">\n\
	<input name=\"action\" type=\"hidden\" value=\"execute\">\n\
	<TABLE cellspacing=\"0\">\n\
		<TR><TH>&nbsp;</TH><TH>Edit Camd35 Config</TH></TR>\n\
		<TR><TD>Port:</TD><TD><input name=\"port\" type=\"text\" size=\"5\" maxlength=\"5\" value=\"##PORT##\"></TD></TR>\n\
		<TR><TD>Serverip:</TD><TD><input name=\"serverip\" type=\"text\" size=\"30\" maxlength=\"30\" value=\"##SERVERIP##\"></TD></TR>\n\
	</TABLE>\n\
	<input type=\"submit\" value=\"OK\">\n\
</form>\n\
<BR><BR>Configuration camd35 not yet implemented<BR><BR>\n\
##TPLFOOTER##"

enum refreshtypes {REFR_ACCOUNTS, REFR_READERS, REFR_SERVER, REFR_ANTICASC};

char *tpl[]={
	"HEADER",
	"FOOTER",
	"MENU",
	"REFRESH",
	"STATUS",
	"CLIENTSTATUSBIT",
	"USERCONFIGLIST",
	"USERCONFIGLISTBIT",
	"SIDTAB",
	"SIDTABBIT",
	"READERS",
	"READERSBIT",
	"ENTITLEMENTS",
	"READERCONFIG",
	"READERCONFIGSIDOKBIT",
	"READERCONFIGSIDNOBIT",
	"USEREDIT",
	"USEREDITRDRSELECTED",
	"USEREDITSIDOKBIT",
	"USEREDITSIDNOBIT",
	"SAVETEMPLATES",
	"CONFIGMENU",
	"CONFIGGBOX",
	"CONFIGCCCAM",
	"CONFIGMONITOR",
	"CONFIGRADEGAST",
	"CONFIGNEWCAMD",
	"CONFIGGLOBAL",
	"CONFIGCAMD33",
	"CONFIGCAMD35"
#ifdef CS_ANTICASC
	,"USEREDITANTICASC"
	,"CONFIGANTICASC"
	,"CONFIGMENUANTICASC"
#endif
};

char *tplmap[]={
	TPLHEADER,
	TPLFOOTER,
	TPLMENU,
	TPLREFRESH,
	TPLSTATUS,
	TPLCLIENTSTATUSBIT,
	TPLUSERCONFIGLIST,
	TPLUSERCONFIGLISTBIT,
	TPLSIDTAB,
	TPLSIDTABBIT,
	TPLREADERS,
	TPLREADERSBIT,
	TPLENTITLEMENTS,
	TPLREADERCONFIG,
	TPLREADERCONFIGSIDOKBIT,
	TPLREADERCONFIGSIDNOBIT,
	TPLUSEREDIT,
	TPLUSEREDITRDRSELECTED,
	TPLUSEREDITSIDOKBIT,
	TPLUSEREDITSIDNOBIT,
	TPLSAVETEMPLATES,
	TPLCONFIGMENU,
	TPLCONFIGGBOX,
	TPLCONFIGCCCAM,
	TPLCONFIGMONITOR,
	TPLCONFIGRADEGAST,
	TPLCONFIGNEWCAMD,
	TPLCONFIGGLOBAL,
	TPLCONFIGCAMD33,
	TPLCONFIGCAMD35
#ifdef CS_ANTICASC
	,TPLUSEREDITANTICASC
	,TPLCONFIGANTICASC
	,TPLCONFIGMENUANTICASC
#endif
};

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
void send_css(FILE *f);
char *getParam(struct uriparams *params, char *name);
int tpl_saveIncludedTpls(const char *path);
