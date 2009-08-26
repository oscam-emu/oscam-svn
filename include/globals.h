#ifndef __GLOBALS_H__
#  define __GLOBALS_H__

/* To be removed after further testing : */
//#  include <assert.h>
//#  include <limits.h>
//#  include <sys/mman.h>
//#  include <stdio.h>
//#  include <sys/socket.h>
//#  include <netinet/in.h>

#  include <errno.h>
#  include <string.h>
#  include <sys/types.h>

#  include <sys/timeb.h>
#  include <netdb.h>

#  include "cscrypt.h"

#  include "os-types.h"

#  define CS_VERSION_NUMBER	"0.99.4svn"

#  ifdef CS_WITH_GBOX
#    include "csgbox/gbox.h"
#    define CS_VERSION		CS_VERSION_NUMBER "-gbx-" GBXVERSION
#  else
#    define CS_VERSION		CS_VERSION_NUMBER
#  endif

#  if defined(__GNUC__)
#    define GCC_PACK __attribute__((packed))
#  else
#    define GCC_PACK
#  endif

#  ifndef CS_CONFDIR
#    define CS_CONFDIR 		"/usr/local/etc"
#  endif
#  ifndef CS_MMAPFILE
#    define CS_MMAPFILE		"/tmp/oscam.mem"
#  endif
#  ifndef CS_LOGFILE
#    define CS_LOGFILE		"/var/log/oscam.log"
#  endif

#  define CS_QLEN		128	// size of request queue
#  define CS_MAXQLEN		128	// size of request queue for cardreader
#  define CS_MAXCAIDTAB		32	// max. caid-defs/user
#  define CS_MAXTUNTAB		16	// max. betatunnel mappings
#  define CS_MAXPROV		32
#  define CS_MAXPORTS		32	// max server ports
#  define CS_MAXFILTERS		16

#  define CS_MAXCARDS		4096
#  define CS_MAXIGNORE		1024
#  define CS_MAXLOCALS		16
#  define CS_ECMSTORESIZE	16	// use MD5()
#  define CS_EMMSTORESIZE	270
#  define CS_CLIENT_TIMEOUT	5000
#  define CS_CLIENT_MAXIDLE	120
#  define CS_BIND_TIMEOUT	120
#  define CS_DELAY		0
#  define CS_RESOLVE_DELAY	30
#  define CS_MAXLOGHIST		30
#  define CS_LOGHISTSIZE	160	// 32+128: username + logline

#  ifdef OLD_DEFS
#    ifdef  CS_EMBEDDED
#      define CS_MAXPENDING	32
#      define CS_ECMCACHESIZE	32
#      define CS_EMMCACHESIZE	64
#      define CS_MAXPID		32
#      define CS_MAXREADER	8
#    else
#      define CS_MAXPENDING	128
#      define CS_ECMCACHESIZE	128
#      define CS_EMMCACHESIZE	256
#      define CS_MAXPID		128
#      define CS_MAXREADER	64
#    endif
#  endif

#  ifdef  CS_EMBEDDED
#    define CS_MAXPID		32
#    define CS_MAXREADER		(CS_MAXPID>>1)
#    define CS_MAXPENDING		CS_MAXPID
#    define CS_ECMCACHESIZE		CS_MAXPID
#    define CS_EMMCACHESIZE		(CS_MAXPID<<1)
#  else
#    define CS_MAXPID		512
#    define CS_MAXREADER		(CS_MAXPID>>2)
#    define CS_MAXPENDING		(CS_MAXPID<<1)
#    define CS_ECMCACHESIZE		CS_MAXPID
#    define CS_EMMCACHESIZE		(CS_MAXPID<<1)
#  endif

#  define D_DUMP		1	// Debug Dumps
#  define D_MASTER		2	// Debug Master Process
#  define D_READER		4	// Debug Reader/Proxy Process
#  define D_CLIENT		8	// Debug Client Process
#  define D_DEVICE		16	// Debug Reader I/O
#  define D_ALL_DUMP		31

#  define R_MOUSE		0x01	// Reader local serial : mouse
#  define R_INTERN		0x02	// Reader local serial : intern
#  define R_SMART		0x03	// Reader local serial : smartreader+

#  define R_CAMD35		0x80	// Reader cascading network : camd 3.5x
#  define R_CAMD33		0x81	// Reader cascading network : camd 3.3x
#  define R_NEWCAMD		0x82	// Reader cascading network : newcamd
#  define R_RADEGAST		0x83	// Reader cascading network : radegast
#  define R_CS378X		0x84	// Reader cascading network : camd 3.5x TCP
#  ifdef CS_WITH_GBOX
#    define R_GBOX		0x85	// Reader cascading network : gbox
#  endif
#  define R_SERIAL		0xC0	// Reader cascading not network : serial

#  define R_IS_LOCAL		~0x80		// 0x00 -> 0x7F : local
#  define R_IS_CASCADING	0x80		// 0x80 -> 0xFF : cascading

#  define R_IS_SERIAL		~0xC0		// 0x00 -> 0x3F : local serial
#  define R_IS_NOT_SERIAL	~0x80 & 0x40	// 0x40 -> 0x7F : local not serial

#  define R_IS_NETWORK		0x80 & ~0x40	// 0x80 -> 0xBF : cascading network
#  define R_IS_NOT_NETWORK	0xC0		// 0xC0 -> 0xFF : cascading not network

#  define CS_MAX_MOD		8
#  define MOD_CONN_TCP		1
#  define MOD_CONN_UDP		2
#  define MOD_CONN_NET		3
#  define MOD_CONN_SERIAL	4

#  ifdef CS_CORE
char *PIP_ID_TXT[] = { "ECM", "EMM", "LOG", "CIN", "HUP", NULL };
char *RDR_CD_TXT[] = { "cd", "dsr", "cts", "ring", "none",
#    ifdef USE_GPIO
	"gpio2", "gpio3", "gpio4", "gpio5", "gpio6", "gpio7",
#    endif
	NULL
};
#  else
extern char *PIP_ID_TXT[];
extern char *RDR_CD_TXT[];
#  endif

#  define PIP_ID_ECM		0
#  define PIP_ID_EMM		1
#  define PIP_ID_LOG		2
#  define PIP_ID_CIN		3	// CARD_INFO
#  define PIP_ID_HUP		4
#  define PIP_ID_MAX		PIP_ID_HUP
#  define PIP_ID_DCW		5

#  define PIP_ID_ERR		(-1)
#  define PIP_ID_DIR		(-2)
#  define PIP_ID_NUL		(-3)

#  define cdiff *c_start

#  define NCD_AUTO		0
#  define NCD_524		1
#  define NCD_525		2

#  define CS_ANTICASC

#  define CARD_INSERTED  1
#  define CARD_NEED_INIT 2
#  define CARD_FAILURE   4

enum { E1_GLOBAL = 0, E1_USER, E1_READER, E1_SERVER, E1_LSERVER };
enum { E2_GLOBAL = 0, E2_GROUP, E2_CAID, E2_IDENT, E2_CLASS, E2_CHID, E2_QUEUE,
	E2_EA_LEN, E2_F0_LEN, E2_OFFLINE, E2_SID
};

typedef struct s_classtab {
	uchar an;
	uchar bn;
	uchar aclass[31];
	uchar bclass[31];
} GCC_PACK CLASSTAB;

typedef struct s_caidtab {
	ushort caid[CS_MAXCAIDTAB];
	ushort mask[CS_MAXCAIDTAB];
	ushort cmap[CS_MAXCAIDTAB];
} GCC_PACK CAIDTAB;

typedef struct s_tuntab {
	ushort bt_caidfrom[CS_MAXTUNTAB];
	ushort bt_caidto[CS_MAXTUNTAB];
	ushort bt_srvid[CS_MAXTUNTAB];
} GCC_PACK TUNTAB;

typedef struct s_sidtab {
	char label[33];
	ushort num_caid;
	ushort num_provid;
	ushort num_srvid;
	ushort *caid;
	ulong *provid;
	ushort *srvid;
	struct s_sidtab *next;
} GCC_PACK SIDTAB;

typedef struct s_filter {
	ushort caid;
	uchar nprids;
	ulong prids[CS_MAXPROV];
} GCC_PACK FILTER;

typedef struct s_ftab {
	int nfilts;
	FILTER filts[CS_MAXFILTERS];
} GCC_PACK FTAB;

typedef struct s_port {
	int fd;
	int s_port;
	FTAB ftab;
} GCC_PACK PORT;

typedef struct s_ptab {
	int nports;
	PORT ports[CS_MAXPORTS];
} GCC_PACK PTAB;

struct s_ecm {
	uchar ecmd5[CS_ECMSTORESIZE];
	uchar cw[16];
	ushort caid;
	ulong prid;
	ulong grp;
};

struct s_emm {
	uchar emm[CS_EMMSTORESIZE];
	uchar type;
	int count;
};

struct s_module {
	int multi;
	int type;
	int watchdog;
	char desc[16];
	char *logtxt;
	in_addr_t s_ip;
	void (*s_handler) ();
	int (*recv) ();
	void (*send_dcw) ();
	int c_multi;
	int (*c_recv_chk) ();
	int (*c_init) ();
	int (*c_send_ecm) ();
	int (*c_init_log) ();
	int (*c_recv_log) ();
	int c_port;
	PTAB *ptab;
};

struct s_client {
	pid_t pid;
	in_addr_t ip;
	in_port_t port;
	time_t login;
	time_t last;
	time_t lastswitch;
	time_t lastemm;
	time_t lastecm;
	ulong grp;
	int crypted;
	int dup;
	int au;
	int monlvl;
	int dbglvl;
	CAIDTAB ctab;
	TUNTAB ttab;
	ulong sidtabok;		// positiv services
	ulong sidtabno;		// negative services
	int typ;
	int ctyp;
	int stat;
	int ufd;
	int last_srvid;
	int last_caid;
	int tosleep;
	char usr[32];
	int udp_fd;
	int fd_m2c;
	struct sockaddr_in udp_sa;
	int log;
	int logcounter;
	int cwfound;
	int cwcache;
	int cwnot;
	uchar ucrc[4];		// needed by monitor and used by camd35
	ulong pcrc;		// pwd crc
	AES_KEY aeskey;		// needed by monitor and used by camd33, camd35
	ushort ncd_msgid;
	uchar ncd_skey[16];
	int port_idx;		// index in server ptab
	int ncd_server;		// newcamd server?
#  ifdef CS_ANTICASC
	ushort ac_idx;
	ushort ac_limit;
	uchar ac_penalty;
#  endif
	FTAB fchid;
	FTAB ftab;		// user [caid] and ident filter
	CLASSTAB cltab;
};

struct s_reader {
	char label[32];
	char device[128];
	int type;

	int card_system;
	int detect;
	int mhz;
	int custom_speed;

	int online;
	int card_status;

	int cs_idx;
	int fd;
	ulong grp;
	int fallback;
	int r_port;
	char r_usr[64];
	char r_pwd[64];
	int r_crypted;
	int l_port;
	int log_port;
	CAIDTAB ctab;
	ulong boxid;
	uchar aes_key[16];
	ulong sidtabok;		// positiv services
	ulong sidtabno;		// negative services
	uchar hexserial[8];
	int nprov;
	uchar prid[CS_MAXPROV][8];
	uchar availkeys[CS_MAXPROV][16];	// viaccess; misused in seca, if availkeys[PROV][0]=0 then expired, 1 then valid.
	uchar sa[CS_MAXPROV][4];	// viaccess & seca
	ushort acs;		// irdeto
	ushort caid[16];
	uchar b_nano[256];
	char pincode[5];
	int logemm;
	int cachemm;
	int rewritemm;
	struct s_module ph;
	uchar ncd_key[16];
	uchar ncd_skey[16];
	int ncd_disable_server_filt;
	ushort ncd_msgid;
	int ncd_proto;
	uchar tcp_connected;
	int tcp_ito;		// inactivity timeout
	int tcp_rto;		// reconnect timeout
	time_t last_g;		// get (if last_s-last_g>tcp_rto - reconnect )
	time_t last_s;		// send 
	uchar show_cls;		// number of classes subscription showed on kill -31
	int maxqlen;		// max queue length
	int qlen;		// current queue length
	FTAB fchid;
	FTAB ftab;
	CLASSTAB cltab;
#  ifdef CS_WITH_GBOX
	uchar gbox_pwd[4];
	uchar gbox_timecode[7];
	int gbox_online;
	uchar gbox_vers;
	uchar gbox_prem;
	int gbox_fd;
	struct timeb gbox_lasthello;	// incoming time stamp
#  endif
};

struct s_auth {
	char usr[33];
	char pwd[33];
	int uniq;
	int au;
	int monlvl;
	ulong grp;
	int tosleep;
	CAIDTAB ctab;
	ulong sidtabok;		// positiv services
	ulong sidtabno;		// negative services
	FTAB fchid;
	FTAB ftab;		// user [caid] and ident filter
	CLASSTAB cltab;
	TUNTAB ttab;
#  ifdef CS_ANTICASC
	int ac_idx;
	int ac_users;		// 0 - unlimited
	uchar ac_penalty;	// 0 - log, >0 - fake dw
#  endif
	in_addr_t dynip;
	uchar dyndns[64];
	struct s_auth *next;
};

struct s_srvid {
	int srvid;
	char name[33];
	struct s_srvid *next;
};

struct s_ip {
	in_addr_t ip[2];
	struct s_ip *next;
};

struct s_config {
	int nice;
	ulong netprio;
	ulong ctimeout;
	ulong ftimeout;
	int cmaxidle;
	int ulparent;
	ulong delay;
	int bindwait;
	int resolvedelay;
	int tosleep;
	in_addr_t srvip;
	char pidfile[128];
	char usrfile[128];
	char cwlogdir[128];
	struct s_auth *account;
	struct s_srvid *srvid;
	struct s_sidtab *sidtab;
	int mon_port;
	in_addr_t mon_srvip;
	struct s_ip *mon_allowed;
	int mon_aulow;
	int mon_hideclient_to;
	int mon_level;
	int c33_port;
	in_addr_t c33_srvip;
	uchar c33_key[16];
	int c33_crypted;
	int c33_passive;
	struct s_ip *c33_plain;
	int c35_port;
	in_addr_t c35_srvip;
	PTAB c35_tcp_ptab;
	in_addr_t c35_tcp_srvip;
	PTAB ncd_ptab;
	in_addr_t ncd_srvip;
	uchar ncd_key[16];
	int rad_port;
	in_addr_t rad_srvip;
	struct s_ip *rad_allowed;
	char rad_usr[32];
	char ser_device[512];
	int srtimeout;		// SerialReaderTimeount in millisec
	int max_log_size;
	int show_ecm_dw;
	int waitforcards;
	int preferlocalcards;
#  ifdef CS_WITH_GBOX
	uchar gbox_pwd[8];
#  endif
	uchar ignorefile[512];
	uchar cardfile[512];
	uchar gbxShareOnl[512];
	int maxdist;
	int num_locals;
	unsigned long locals[CS_MAXLOCALS];
#  ifdef CS_ANTICASC
	char ac_enabled;
	int ac_users;		// num of users for account (0 - default)
	int ac_stime;		// time to collect AC statistics (3 min - default)
	int ac_samples;		// qty of samples
	int ac_penalty;		// 0 - write to log
	int ac_fakedelay;	// 100-1000 ms
	int ac_denysamples;
	char ac_logfile[128];
	struct s_cpmap *cpmap;
#  endif
};

typedef struct ecm_request_t {
	uchar ecm[256];
	uchar cw[16];
	uchar ecmd5[CS_ECMSTORESIZE];
	short l;
	ushort caid;
	ushort ocaid;
	ushort srvid;
	ushort chid;
	ushort pid;
	ushort idx;
	ulong prid;
	int reader[CS_MAXREADER];
	int cidx;		// client index
	int cpti;		// client pending table index
	int stage;		// processing stage in server module
	int level;		// send-level in client module
	int rc;
	uchar rcEx;
	struct timeb tps;	// incoming time stamp
	uchar locals_done;
	ushort gbxCWFrom;
	ushort gbxFrom;
	ushort gbxTo;

	uchar gbxForward[16];
	int gbxRidx;
} GCC_PACK ECM_REQUEST;

typedef struct emm_packet_t {
	uchar emm[258];
	uchar l;
	uchar caid[2];
	uchar provid[4];
	uchar hexserial[8];
	uchar type;
	int cidx;
} GCC_PACK EMM_PACKET;

extern int pfd, rfd, fd_c2m, fd_m2c, cs_idx, *c_start, cs_ptyp, cs_dblevel, cs_hw;
extern int *logidx, *loghistidx, *log_fd;
extern int is_server, *mcl;
extern uchar mbuf[1024];
extern ushort cam_common_len4caid[256];
extern pid_t master_pid;
extern struct s_ecm *ecmcache;
extern struct s_client *client;
extern struct s_reader *reader;

extern struct card_struct *Cards;
extern struct idstore_struct *idstore;
extern unsigned long *IgnoreList;

extern struct s_config *cfg;
extern char cs_confdir[], *loghist;
extern EMM_PACKET epg;
extern struct s_module ph[CS_MAX_MOD];
extern ECM_REQUEST *ecmtask;
extern char logfile[256];

extern int ridx, logfd;

#endif // __GLOBALS_H__
