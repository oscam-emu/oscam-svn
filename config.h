/*
 * config.h
 *
 *      Author: aston
 */
#ifndef CONFIG_H_
#define CONFIG_H_

#include "main.h"
#include "cthread.h"

#include <fstream>

//---------------------------------------------------------------------------
#define CS_CONF 	"oscam.conf";
#define CS_USER 	"oscam.user";
#define CS_SERVER	"oscam.server";
#define CS_SRVID 	"oscam.srvid";
#define cs_GUESS 	"oscam.guess";
#define cs_CERT     "oscam.cert";
#define cs_SERVICES "oscam.services";

#ifdef CS_ANTICASC
static char *cs_ac="oscam.ac";
#endif

#define CS_CLIENT_TIMEOUT 5000
#define CS_CLIENT_MAXIDLE 120
#define CS_DELAY          0
#define CS_BIND_TIMEOUT   120
#define CS_RESOLVE_DELAY  30
#define CS_MAXPROV    32
#define CS_MAXPORTS   32  // max server ports
#define CS_MAXFILTERS   16
#define CS_MAXCAIDTAB 32  // max. caid-defs/user

//------------------------------------------
struct s_ip
{
  in_addr_t ip[2];
  struct s_ip *next;
};

//------------------------------------------
typedef struct s_filter
{
  ushort caid;
  uchar  nprids;
  ulong  prids[CS_MAXPROV];
} FILTER;

//------------------------------------------
typedef struct s_ftab
{
  int    nfilts;
  FILTER filts[CS_MAXFILTERS];
} FTAB;

//------------------------------------------
typedef struct s_port
{
  int    fd;
  int    s_port;
  FTAB   ftab;
} PORT;

//------------------------------------------
typedef struct s_ptab
{
  int    nports;
  PORT   ports[CS_MAXPORTS];
} PTAB;

//------------------------------------------
struct s_provid
{
	int		caid;
	ulong	provid;
	char	prov[33];
	char	sat[33];
	char	lang[33];
	struct	s_provid *next;
};

//------------------------------------------
typedef struct s_sidtab
{
  char     label[33];
  ushort   num_caid;
  ushort   num_provid;
  ushort   num_srvid;
  ushort   *caid;
  ulong    *provid;
  ushort   *srvid;
  struct   s_sidtab *next;
} SIDTAB;

//---------------------------------------------------------------------------
struct s_config
{
	int				nice;
	int				debuglvl;
	ulong			netprio;
	ulong			ctimeout;
	ulong			ftimeout;
	ulong			cmaxidle;
	int				ulparent;
	ulong			delay;
	int				bindwait;
	int				resolvedelay;
	int				clientdyndns;
	int				tosleep;
	in_addr_t		srvip;
	string			pidfile;
	string			usrfile;
	string			cwlogdir;
	string			logfile;
	int				disablelog;
	int				disableuserfile;
	int				usrfileflag;
//	struct s_auth 	account;
//	struct s_srvid *srvid;
	int				mon_port;
	in_addr_t		mon_srvip;
	struct s_ip    *mon_allowed;
	int				mon_aulow;
	int				mon_hideclient_to;
	int				mon_level;
	int				mon_appendchaninfo;

	int			c33_port;
	in_addr_t	c33_srvip;
	uchar		c33_key[16];
	int			c33_crypted;
	int			c33_passive;
	struct s_ip *c33_plain;

	int			c35_port;
	in_addr_t	c35_srvip;
	int			c35_suppresscmd08;
	PTAB		c35_tcp_ptab;
	in_addr_t	c35_tcp_srvip;

	PTAB		ncd_ptab;
	in_addr_t	ncd_srvip;
	uchar		ncd_key[16];
	int			ncd_keepalive;
	int			ncd_mgclient;
	struct s_ip *ncd_allowed;
	PTAB		cc_ptab;
	int			rad_port;
	in_addr_t	rad_srvip;

	struct s_ip *rad_allowed;
	string		rad_usr;
	string		ser_device;
	ulong		srtimeout;  // SerialReaderTimeount in millisec
	int			max_log_size;
	bool		waitforcards;
	bool		preferlocalcards;
	bool		saveinithistory;
	bool		reader_auto_loadbalance;
	bool		reader_auto_loadbalance_save;
	int     	reader_restart_seconds; //Schlocke Reader restart auf x seconds, disable = 0

	int			cc_port;
	bool		cc_reshare;
	in_addr_t	cc_srvip;
	string		cc_version;//[7];
	string		cc_build;//[5];
	//Todo #ifdef CCCAM
	struct s_provid *provid;
	struct s_sidtab *sidtab;

#ifdef WEBIF
	int			http_port;
	char		http_user[65];
	char		http_pwd[65];
	char		http_css[128];
	char		http_tpl[128];
	char		http_script[128];
	int			http_refresh;
	int			http_hide_idle_clients;
	struct 	s_ip *http_allowed;
	int			http_readonly;
	in_addr_t	http_dynip;
	uchar		http_dyndns[64];
#endif

#ifdef CS_WITH_GBOX
	uchar	gbox_pwd[8];
	uchar	ignorefile[128];
	uchar	cardfile[128];
	uchar	gbxShareOnl[128];
	int		maxdist;
	int		num_locals;
	unsigned long 	locals[CS_MAXLOCALS];
#endif

#ifdef IRDETO_GUESSING
	struct s_irdeto_quess *itab[0xff];
#endif

#ifdef HAVE_DVBAPI
	int		dvbapi_enabled;
	int		dvbapi_au;
	char	dvbapi_usr[33];
	int		dvbapi_boxtype;
	CAIDTAB	dvbapi_prioritytab;
	CAIDTAB	dvbapi_ignoretab;
	CAIDTAB	dvbapi_delaytab;
#endif

#ifdef CS_ANTICASC
	char	ac_enabled;
	int		ac_users;       // num of users for account (0 - default)
	int		ac_stime;       // time to collect AC statistics (3 min - default)
	int		ac_samples;     // qty of samples
	int		ac_penalty;     // 0 - write to log
	int		ac_fakedelay;   // 100-1000 ms
	int		ac_denysamples;
	char	ac_logfile[128];
	struct	s_cpmap *cpmap;
#endif
};

typedef enum cs_proto_type
{
	TAG_GLOBAL,		// must be first !
	TAG_MONITOR,	// monitor
	TAG_CAMD33,		// camd 3.3x
	TAG_CAMD35,		// camd 3.5x UDP
	TAG_NEWCAMD,	// newcamd
	TAG_RADEGAST,	// radegast
	TAG_SERIAL,		// serial (static)
	TAG_CS357X,		// camd 3.5x UDP
	TAG_CS378X,		// camd 3.5x TCP
	TAG_GBOX,		// gbox
	TAG_CCCAM,		// cccam
	TAG_DVBAPI,		// dvbapi
	TAG_WEBIF,		// webif
	TAG_ANTICASC	// anti-cascading
} cs_proto_type_t;

//---------------------------------------------------------------------------
class t_config
{
private:
	fstream *oscamConfFile;
	void delete_s_ip(s_ip *current);
	void chk_iprange(char *value, struct s_ip **base);
	void chk_port_tab(char *portasc, PTAB *ptab);

	void chk_token(string* token, string* value, int);
	void chk_t_global(string* token, string* value);
	void chk_t_monitor(string* token, string* value);
	void chk_t_camd33(string* token, string* value);
	void chk_t_camd35(string* token, string* value);
	void chk_t_newcamd(string* token, string* value);
	void chk_t_radegast(string* token, string* value);
	void chk_t_serial(string* token, string* value);
	void chk_t_camd35_tcp(string* token, string* value);
	void chk_t_cccam(string* token, string* value);
public:
	 t_config();
	~t_config();
	s_config *oscamConf;
	void init_config();
};
//---------------------------------------------------------------------------

#endif /* CONFIG_H_ */

