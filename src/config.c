#include "globals.h"
#include "config.h"

#include "simples.h"
#include "ac.h"
#include "log.h"
#include "oscam.h"
#include "network.h"

#ifdef CS_WITH_BOXKEYS
#  include "boxkeys.np"
#endif

#include <stdio.h>
#include <stdlib.h>

static char *cs_conf = "oscam.conf";
static char *cs_user = "oscam.user";
static char *cs_srvr = "oscam.server";
static char *cs_srid = "oscam.srvid";
static char *cs_l4ca = "oscam.guess";
static char *cs_cert = "oscam.cert";
static char *cs_sidt = "oscam.services";

//static char *cs_ird="oscam.ird";
#ifdef CS_ANTICASC
static char *cs_ac = "oscam.ac";
#endif

static char token[4096];

//--- WARNING: cs_proto_type_t and cctag must use the same order ---//
typedef enum cs_proto_type {
	TAG_GLOBAL,		// must be first !
	TAG_CAMD33,		// camd 3.3x
	TAG_CAMD35,		// camd 3.5x UDP
	TAG_NEWCAMD,		// newcamd
	TAG_RADEGAST,		// radegast
	TAG_SERIAL,		// serial (static)
	TAG_CS357X,		// camd 3.5x UDP
	TAG_CS378X,		// camd 3.5x TCP
#ifdef CS_WITH_GBOX
	TAG_GBOX,		// gbox
#endif
#ifdef CS_ANTICASC
	TAG_ANTICASC,		// anti-cascading
#endif
	TAG_MONITOR		// monitor
} cs_proto_type_t;

//--- WARNING: cs_proto_type_t and cctag must use the same order ---//
static char *cctag[] = { "global", "camd33", "camd35",
	"newcamd", "radegast", "serial", "cs357x", "cs378x",
#ifdef CS_WITH_GBOX
	"gbox",
#endif
#ifdef CS_ANTICASC
	"anticasc",
#endif
	"monitor",
	NULL
};

#ifdef DEBUG_SIDTAB
static void config_show_sidtab(struct s_sidtab *sidtab)
{
	for (; sidtab; sidtab = sidtab->next) {
		int i;
		char buf[1024];

		log_normal("label=%s", sidtab->label);
		sprintf(buf, "caid(%d)=", sidtab->num_caid);
		for (i = 0; i < sidtab->num_caid; i++)
			sprintf(buf + strlen(buf), "%04X ", sidtab->caid[i]);
		log_normal("%s", buf);
		sprintf(buf, "provider(%d)=", sidtab->num_provid);
		for (i = 0; i < sidtab->num_provid; i++)
			sprintf(buf + strlen(buf), "%08X ", sidtab->provid[i]);
		log_normal("%s", buf);
		sprintf(buf, "services(%d)=", sidtab->num_srvid);
		for (i = 0; i < sidtab->num_srvid; i++)
			sprintf(buf + strlen(buf), "%04X ", sidtab->srvid[i]);
		log_normal("%s", buf);
	}
}
#endif

static void config_check_iprange(char *value, struct s_ip **base)
{
	char *ptr1, *ptr2;
	struct s_ip *lip, *cip;

	for (cip = lip = *base; cip; cip = cip->next)
		lip = cip;
	if (!(cip = malloc(sizeof (struct s_ip)))) {
		fprintf(stderr, "Error allocating memory (errno=%d)\n", errno);
		exit(1);
	}
	if (*base)
		lip->next = cip;
	else
		*base = cip;

	memset(cip, 0, sizeof (struct s_ip));
	for (ptr1 = strtok(value, ","); ptr1; ptr1 = strtok(NULL, ",")) {
		if ((ptr2 = strchr(trim(ptr1), '-'))) {
			*ptr2++ = '\0';
			cip->ip[0] = network_inet_addr(trim(ptr1));
			cip->ip[1] = network_inet_addr(trim(ptr2));
		} else
			cip->ip[0] = cip->ip[1] = network_inet_addr(ptr1);
	}
}

static void config_check_caidtab(char *caidasc, CAIDTAB * ctab)
{
	int i;
	char *ptr1, *ptr2, *ptr3;

	for (i = 0, ptr1 = strtok(caidasc, ","); (i < CS_MAXCAIDTAB) && (ptr1); ptr1 = strtok(NULL, ",")) {
		ulong caid, mask, cmap;

		if ((ptr3 = strchr(trim(ptr1), ':')))
			*ptr3++ = '\0';
		else
			ptr3 = "";
		if ((ptr2 = strchr(trim(ptr1), '&')))
			*ptr2++ = '\0';
		else
			ptr2 = "";
		if (((caid = a2i(ptr1, 2)) | (mask = a2i(ptr2, -2)) | (cmap = a2i(ptr3, 2))) < 0x10000) {
			ctab->caid[i] = caid;
			ctab->mask[i] = mask;
			ctab->cmap[i++] = cmap;
		}
//		else
//			log_normal("WARNING: wrong CAID in %s -> ignored", cs_user);
	}
}

static void config_check_tuntab(char *tunasc, TUNTAB * ttab)
{
	int i;
	char *ptr1, *ptr2, *ptr3;

	for (i = 0, ptr1 = strtok(tunasc, ","); (i < CS_MAXTUNTAB) && (ptr1); ptr1 = strtok(NULL, ",")) {
		ulong bt_caidfrom, bt_caidto, bt_srvid;

		if ((ptr3 = strchr(trim(ptr1), ':')))
			*ptr3++ = '\0';
		else
			ptr3 = "";
		if ((ptr2 = strchr(trim(ptr1), '.')))
			*ptr2++ = '\0';
		else
			ptr2 = "";
		if ((bt_caidfrom = a2i(ptr1, 2)) | (bt_srvid = a2i(ptr2, -2)) | (bt_caidto = a2i(ptr3, 2))) {
			ttab->bt_caidfrom[i] = bt_caidfrom;
			ttab->bt_caidto[i] = bt_caidto;
			ttab->bt_srvid[i++] = bt_srvid;
		}
//		else
//			log_normal("WARNING: wrong Betatunnel in %s -> ignored", cs_user);
	}
}

static void config_check_services(char *labels, ulong * sidok, ulong * sidno)
{
	int i;
	char *ptr;
	SIDTAB *sidtab;

	*sidok = *sidno = 0;
	for (ptr = strtok(labels, ","); ptr; ptr = strtok(NULL, ","))
		for (trim(ptr), i = 0, sidtab = cfg->sidtab; sidtab; sidtab = sidtab->next, i++) {
			if (!strcmp(sidtab->label, ptr))
				*sidok |= (1 << i);
			if ((ptr[0] == '!') && (!strcmp(sidtab->label, ptr + 1)))
				*sidno |= (1 << i);
		}
}

static void config_check_ftab(char *zFilterAsc, FTAB * ftab, const char *zType, const char *zName, const char *zFiltName)
{
	int i, j;
	char *ptr1, *ptr2, *ptr3;
	char *ptr[CS_MAXFILTERS] = { 0 };

	memset(ftab, 0, sizeof (FTAB));
	for (i = 0, ptr1 = strtok(zFilterAsc, ";"); (i < CS_MAXFILTERS) && (ptr1); ptr1 = strtok(NULL, ";"), i++) {
		ptr[i] = ptr1;
		if ((ptr2 = strchr(trim(ptr1), ':'))) {
			*ptr2++ = '\0';
			ftab->filts[i].caid = (ushort) a2i(ptr1, 4);
			ptr[i] = ptr2;
		} else if (zFiltName && zFiltName[0] == 'c') {
			log_normal("PANIC: CAID field not found in CHID parameter!");
			oscam_exit(1);
		}
		ftab->nfilts++;
	}

	if (ftab->nfilts)
		log_debug("%s '%s' %s filter(s):", zType, zName, zFiltName);
	for (i = 0; i < ftab->nfilts; i++) {
		log_debug("CAID #%d: %04X", i, ftab->filts[i].caid);
		for (j = 0, ptr3 = strtok(ptr[i], ","); (j < CS_MAXPROV) && (ptr3); ptr3 = strtok(NULL, ","), j++) {
			ftab->filts[i].prids[j] = a2i(ptr3, 6);
			ftab->filts[i].nprids++;
			log_debug("%s #%d: %06X", zFiltName, j, ftab->filts[i].prids[j]);
		}
	}
}

static void config_check_cltab(char *classasc, CLASSTAB * clstab)
{
	int i;
	char *ptr1;

	for (i = 0, ptr1 = strtok(classasc, ","); (i < CS_MAXCAIDTAB) && (ptr1); ptr1 = strtok(NULL, ",")) {
		ptr1 = trim(ptr1);
		if (ptr1[0] == '!')
			clstab->bclass[clstab->bn++] = (uchar) a2i(ptr1 + 1, 2);
		else
			clstab->aclass[clstab->an++] = (uchar) a2i(ptr1, 2);
	}
}

static void config_check_port_tab(char *portasc, PTAB * ptab)
{
	int i, j, nfilts, ifilt, iport;
	char *ptr1, *ptr2, *ptr3;
	char *ptr[CS_MAXPORTS] = { 0 };
	int port[CS_MAXPORTS] = { 0 };
	int previous_nports = ptab->nports;

	for (nfilts = i = previous_nports, ptr1 = strtok(portasc, ";"); (i < CS_MAXCAIDTAB) && (ptr1); ptr1 = strtok(NULL, ";"), i++) {
		ptr[i] = ptr1;
		if ((ptr2 = strchr(trim(ptr1), '@'))) {
			*ptr2++ = '\0';
			ptab->ports[i].s_port = atoi(ptr1);
			ptr[i] = ptr2;
			port[i] = ptab->ports[i].s_port;
			ptab->nports++;
		}
		nfilts++;
	}

	if (nfilts == 1 && strlen(portasc) < 6 && ptab->ports[0].s_port == 0) {
		ptab->ports[0].s_port = atoi(portasc);
		ptab->nports = 1;
	}

	iport = ifilt = previous_nports;
	for (i = previous_nports; i < nfilts; i++) {
		if (port[i] != 0)
			iport = i;
		for (j = 0, ptr3 = strtok(ptr[i], ","); (j < CS_MAXPROV) && (ptr3); ptr3 = strtok(NULL, ","), j++) {
			if ((ptr2 = strchr(trim(ptr3), ':'))) {
				*ptr2++ = '\0';
				ptab->ports[iport].ftab.nfilts++;
				ifilt = ptab->ports[iport].ftab.nfilts - 1;
				ptab->ports[iport].ftab.filts[ifilt].caid = (ushort) a2i(ptr3, 4);
				ptab->ports[iport].ftab.filts[ifilt].prids[j] = a2i(ptr2, 6);
			} else {
				ptab->ports[iport].ftab.filts[ifilt].prids[j] = a2i(ptr3, 6);
			}
			ptab->ports[iport].ftab.filts[ifilt].nprids++;
		}
	}
}

static void config_check_t_global(char *token, char *value)
{
	if (!strcmp(token, "serverip"))
		cfg->srvip = inet_addr(value);
	if (!strcmp(token, "logfile"))
		strncpy(logfile, value, sizeof (logfile) - 1);
	if (!strcmp(token, "pidfile"))
		strncpy(cfg->pidfile, value, sizeof (cfg->pidfile) - 1);
	if (!strcmp(token, "usrfile"))
		strncpy(cfg->usrfile, value, sizeof (cfg->usrfile) - 1);
	if (!strcmp(token, "cwlogdir"))
		strncpy(cfg->cwlogdir, value, sizeof (cfg->cwlogdir) - 1);
	if (!strcmp(token, "clienttimeout")) {
		cfg->ctimeout = atoi(value);
		if (cfg->ctimeout < 100)
			cfg->ctimeout *= 1000;
	}
	if (!strcmp(token, "fallbacktimeout")) {
		cfg->ftimeout = atoi(value);
		if (cfg->ftimeout < 100)
			cfg->ftimeout *= 1000;
	}

	if (!strcmp(token, "clientmaxidle"))
		cfg->cmaxidle = atoi(value);
	if (!strcmp(token, "cachedelay"))
		cfg->delay = atoi(value);
	if (!strcmp(token, "bindwait"))
		cfg->bindwait = atoi(value);
	if (!strcmp(token, "netprio"))
		cfg->netprio = atoi(value);
	if (!strcmp(token, "resolvedelay"))
		cfg->resolvedelay = atoi(value);
	if (!strcmp(token, "sleep"))
		cfg->tosleep = atoi(value);
	if (!strcmp(token, "unlockparental"))
		cfg->ulparent = atoi(value);
	if (!strcmp(token, "nice")) {
		cfg->nice = atoi(value);
		if ((cfg->nice < -20) || (cfg->nice > 20))
			cfg->nice = 99;
		if (cfg->nice != 99)
			oscam_set_priority(cfg->nice);	// ignore errors
	}
	if (!strcmp(token, "serialreadertimeout")) {
		if (cfg->srtimeout < 100)
			cfg->srtimeout = atoi(value) * 1000;
		else
			cfg->srtimeout = atoi(value);
		if (cfg->srtimeout <= 0)
			cfg->srtimeout = 1500;
	}
	if (!strcmp(token, "maxlogsize")) {
		cfg->max_log_size = atoi(value);
		if (cfg->max_log_size <= 10)
			cfg->max_log_size = 10;
	}
	if (!strcmp(token, "showecmdw"))
		cfg->show_ecm_dw = atoi(value);

	if (!strcmp(token, "waitforcards"))
		cfg->waitforcards = atoi(value);
	if (!strcmp(token, "preferlocalcards"))
		cfg->preferlocalcards = atoi(value);
}

#ifdef CS_ANTICASC
static void config_check_t_ac(char *token, char *value)
{
	if (!strcmp(token, "enabled")) {
		cfg->ac_enabled = atoi(value);
		if (cfg->ac_enabled <= 0)
			cfg->ac_enabled = 0;
		else
			cfg->ac_enabled = 1;
	}

	if (!strcmp(token, "numusers")) {
		cfg->ac_users = atoi(value);
		if (cfg->ac_users < 0)
			cfg->ac_users = 0;
	}
	if (!strcmp(token, "sampletime")) {
		cfg->ac_stime = atoi(value);
		if (cfg->ac_stime < 0)
			cfg->ac_stime = 2;
	}
	if (!strcmp(token, "samples")) {
		cfg->ac_samples = atoi(value);
		if (cfg->ac_samples < 2 || cfg->ac_samples > 10)
			cfg->ac_samples = 10;
	}
	if (!strcmp(token, "penalty")) {
		cfg->ac_penalty = atoi(value);
		if (cfg->ac_penalty < 0)
			cfg->ac_penalty = 0;

	}
	if (!strcmp(token, "aclogfile"))
		strncpy(cfg->ac_logfile, value, sizeof (cfg->ac_logfile) - 1);
	if (!strcmp(token, "fakedelay")) {
		cfg->ac_fakedelay = atoi(value);
		if (cfg->ac_fakedelay < 100 || cfg->ac_fakedelay > 1000)
			cfg->ac_fakedelay = 1000;
	}
	if (!strcmp(token, "denysamples")) {
		cfg->ac_denysamples = atoi(value);
		if (cfg->ac_denysamples < 2 || cfg->ac_denysamples > cfg->ac_samples - 1)
			cfg->ac_denysamples = cfg->ac_samples - 1;
	}
#endif
}

static void config_check_t_monitor(char *token, char *value)
{
	if (!strcmp(token, "port"))
		cfg->mon_port = atoi(value);
	if (!strcmp(token, "serverip"))
		cfg->mon_srvip = inet_addr(value);
	if (!strcmp(token, "nocrypt"))
		config_check_iprange(value, &cfg->mon_allowed);
	if (!strcmp(token, "aulow"))
		cfg->mon_aulow = atoi(value);
	if (!strcmp(token, "monlevel"))
		cfg->mon_level = atoi(value);
	if (!strcmp(token, "hideclient_to"))
		cfg->mon_hideclient_to = atoi(value);
}

static void config_check_t_camd33(char *token, char *value)
{
	if (!strcmp(token, "port"))
		cfg->c33_port = atoi(value);
	if (!strcmp(token, "serverip"))
		cfg->c33_srvip = inet_addr(value);
	if (!strcmp(token, "nocrypt"))
		config_check_iprange(value, &cfg->c33_plain);
	if (!strcmp(token, "passive"))
		cfg->c33_passive = (value[0] != '0');
	if (!strcmp(token, "key")) {
		if (key_atob(value, cfg->c33_key)) {
			fprintf(stderr, "Configuration camd3.3x: Error in Key\n");
			exit(1);
		}
		cfg->c33_crypted = 1;
	}
}

static void config_check_t_camd35(char *token, char *value)
{
	if (!strcmp(token, "port"))
		cfg->c35_port = atoi(value);
	if (!strcmp(token, "serverip"))
		cfg->c35_tcp_srvip = inet_addr(value);
}

static void config_check_t_camd35_tcp(char *token, char *value)
{
	if (!strcmp(token, "port"))
		config_check_port_tab(value, &cfg->c35_tcp_ptab);
	if (!strcmp(token, "serverip"))
		cfg->c35_tcp_srvip = inet_addr(value);
}

static void config_check_t_newcamd(char *token, char *value)
{
	if (!strcmp(token, "port"))
		config_check_port_tab(value, &cfg->ncd_ptab);
	if (!strcmp(token, "serverip"))
		cfg->ncd_srvip = inet_addr(value);
	if (!strcmp(token, "key")) {
		if (key_atob14(value, cfg->ncd_key)) {
			fprintf(stderr, "Configuration newcamd: Error in Key\n");
			exit(1);
		}
	}

}

static void config_check_t_radegast(char *token, char *value)
{
	if (!strcmp(token, "port"))
		cfg->rad_port = atoi(value);
	if (!strcmp(token, "serverip"))
		cfg->rad_srvip = inet_addr(value);
	if (!strcmp(token, "allowed"))
		config_check_iprange(value, &cfg->rad_allowed);
	if (!strcmp(token, "user"))
		strncpy(cfg->rad_usr, value, sizeof (cfg->rad_usr) - 1);
}

static void config_check_t_serial(char *token, char *value)
{
	if (!strcmp(token, "device")) {
		int l;

		l = strlen(cfg->ser_device);
		if (l)
			cfg->ser_device[l++] = 1;	// use ctrl-a as delimiter
		strncpy(cfg->ser_device + l, value, sizeof (cfg->ser_device) - 1 - l);
	}
}

#ifdef CS_WITH_GBOX
static void config_check_t_gbox(char *token, char *value)
{
	if (!strcmp(token, "password"))
		cs_atob(cfg->gbox_pwd, value, 4);
	if (!strcmp(token, "maxdist"))
		cfg->maxdist = atoi(value);
	if (!strcmp(token, "ignorelist"))
		strncpy((char *) cfg->ignorefile, value, sizeof (cfg->ignorefile) - 1);
	if (!strcmp(token, "onlineinfos"))
		strncpy((char *) cfg->gbxShareOnl, value, sizeof (cfg->gbxShareOnl) - 1);
	if (!strcmp(token, "cardinfos"))
		strncpy((char *) cfg->cardfile, value, sizeof (cfg->cardfile) - 1);
	if (!strcmp(token, "locals")) {
		char *ptr1;
		int n = 0, i;

		for (i = 0, ptr1 = strtok(value, ","); (i < CS_MAXLOCALS) && (ptr1); ptr1 = strtok(NULL, ",")) {
			cfg->locals[n++] = a2i(ptr1, 8);
		}
		cfg->num_locals = n;
	}
}
#endif

static void config_check_token(char *token, char *value, int tag)
{
	switch (tag) {
		case TAG_GLOBAL:
			config_check_t_global(token, value);
			break;
		case TAG_MONITOR:
			config_check_t_monitor(token, value);
			break;
		case TAG_CAMD33:
			config_check_t_camd33(token, value);
			break;
		case TAG_CAMD35:
		case TAG_CS357X:
			config_check_t_camd35(token, value);
			break;
		case TAG_NEWCAMD:
			config_check_t_newcamd(token, value);
			break;
		case TAG_RADEGAST:
			config_check_t_radegast(token, value);
			break;
		case TAG_SERIAL:
			config_check_t_serial(token, value);
			break;
		case TAG_CS378X:
			config_check_t_camd35_tcp(token, value);
			break;
#ifdef CS_WITH_GBOX
		case TAG_GBOX:
			config_check_t_gbox(token, value);
			break;
#endif
#ifdef CS_ANTICASC
		case TAG_ANTICASC:
			config_check_t_ac(token, value);
			break;
#endif
	}
}

void config_init_cam_common_len4caid()
{
	int nr;
	FILE *fp;
	char *value;

	memset(cam_common_len4caid, 0, sizeof (ushort) << 8);
	sprintf(token, "%s%s", cs_confdir, cs_l4ca);
	if (!(fp = fopen(token, "r")))
		return;
	for (nr = 0; fgets(token, sizeof (token), fp);) {
		int i, c;
		char *ptr;

		if (!(value = strchr(token, ':')))
			continue;
		*value++ = '\0';
		if ((ptr = strchr(value, '#')))
			*ptr = '\0';
		if (strlen(trim(token)) != 2)
			continue;
		if (strlen(trim(value)) != 4)
			continue;
		if ((i = byte_atob(token)) < 0)
			continue;
		if ((c = word_atob(value)) < 0)
			continue;
		cam_common_len4caid[i] = c;
		nr++;
	}
	fclose(fp);
	log_normal("%d lengths for caid guessing loaded", nr);
}

int config_search_boxkey(ushort caid, ulong provid, char *key)
{
	int i, rc = 0;
	FILE *fp;
	char c_caid[512];

	sprintf(c_caid, "%s%s", cs_confdir, cs_cert);
	if ((fp = fopen(c_caid, "r"))) {
		for (; (!rc) && fgets(c_caid, sizeof (c_caid), fp);) {
			char *c_provid, *c_key;

			if ((c_provid = strchr(c_caid, '#')))
				*c_provid = '\0';
			if (!(c_provid = strchr(c_caid, ':')))
				continue;
			*c_provid++ = '\0';
			if (!(c_key = strchr(c_provid, ':')))
				continue;
			*c_key++ = '\0';
			if (word_atob(trim(c_caid)) != caid)
				continue;
			if ((i = (strlen(trim(c_key)) >> 1)) > 256)
				continue;
			if (cs_atob((uchar *) key, c_key, i) < 0) {
				log_normal("wrong key in \"%s\"", cs_cert);
				continue;
			}
			rc = 1;
		}
		fclose(fp);
	}
#ifdef OSCAM_INBUILD_KEYS
	for (i = 0; (!rc) && (npkey[i].keylen); i++)
		if (rc = ((caid == npkey[i].caid) && (provid == npkey[i].provid)))
			memcpy(key, npkey[i].key, npkey[i].keylen);
#endif
	return (rc);
}

int config_init()
{
	int tag = TAG_GLOBAL;
	FILE *fp;
	char *value;

#ifndef CS_EMBEDDED
#  ifdef PRIO_PROCESS
	errno = 0;
	if ((cfg->nice = getpriority(PRIO_PROCESS, 0)) == (-1))
		if (errno)
#  endif
#endif
			cfg->nice = 99;
	cfg->ctimeout = CS_CLIENT_TIMEOUT;
	cfg->ftimeout = CS_CLIENT_TIMEOUT / 2;
	cfg->cmaxidle = CS_CLIENT_MAXIDLE;
	cfg->delay = CS_DELAY;
	cfg->bindwait = CS_BIND_TIMEOUT;
	cfg->resolvedelay = CS_RESOLVE_DELAY;
	cfg->mon_level = 2;
	cfg->mon_hideclient_to = 0;
	cfg->srtimeout = 1500;
	cfg->ulparent = 0;
#ifdef CS_ANTICASC
	cfg->ac_enabled = 0;
	cfg->ac_users = 0;
	cfg->ac_stime = 2;
	cfg->ac_samples = 10;
	cfg->ac_denysamples = 8;
	cfg->ac_fakedelay = 1000;
	strcpy(cfg->ac_logfile, "./oscam_ac.log");
#endif
	sprintf(token, "%s%s", cs_confdir, cs_conf);
	if (!(fp = fopen(token, "r"))) {
		fprintf(stderr, "Cannot open config file '%s' (errno=%d)\n", token, errno);
		exit(1);
	}
	while (fgets(token, sizeof (token), fp)) {
		int i, l;

		if ((l = strlen(trim(token))) < 3)
			continue;
		if ((token[0] == '[') && (token[l - 1] == ']')) {
			for (token[l - 1] = 0, tag = -1, i = TAG_GLOBAL; cctag[i]; i++)
				if (!strcmp(cctag[i], strtolower(token + 1)))
					tag = i;
			continue;
		}
		if (!(value = strchr(token, '=')))
			continue;
		*value++ = '\0';
		config_check_token(trim(strtolower(token)), trim(value), tag);
	}
	fclose(fp);
	log_init(logfile);
	if (cfg->ftimeout >= cfg->ctimeout) {
		cfg->ftimeout = cfg->ctimeout - 100;
		log_normal("WARNING: fallbacktimeout adjusted to %lu ms (must be smaller than clienttimeout (%lu ms))", cfg->ftimeout, cfg->ctimeout);
	}
	if (cfg->ftimeout < cfg->srtimeout) {
		cfg->ftimeout = cfg->srtimeout + 100;
		log_normal("WARNING: fallbacktimeout adjusted to %lu ms (must be greater than serialreadertimeout (%lu ms))", cfg->ftimeout, cfg->srtimeout);
	}
	if (cfg->ctimeout < cfg->srtimeout) {
		cfg->ctimeout = cfg->srtimeout + 100;
		log_normal("WARNING: clienttimeout adjusted to %lu ms (must be greater than serialreadertimeout (%lu ms))", cfg->ctimeout, cfg->srtimeout);
	}
#ifdef CS_ANTICASC
	if (cfg->ac_denysamples + 1 > cfg->ac_samples) {
		cfg->ac_denysamples = cfg->ac_samples - 1;
		log_normal("WARNING: DenySamples adjusted to %d", cfg->ac_denysamples);
	}
#endif
	return 0;
}

static void config_check_account(char *token, char *value, struct s_auth *account)
{
	int i;
	char *ptr1;

	if (!strcmp(token, "user"))
		strncpy(account->usr, value, sizeof (account->usr) - 1);
	if (!strcmp(token, "pwd"))
		strncpy(account->pwd, value, sizeof (account->pwd) - 1);
	if (!strcmp(token, "hostname"))
		strncpy((char *) account->dyndns, value, sizeof (account->dyndns) - 1);
	if (!strcmp(token, "betatunnel"))
		config_check_tuntab(value, &account->ttab);
	if (!strcmp(token, "uniq"))
		account->uniq = atoi(value);
	if (!strcmp(token, "sleep"))
		account->tosleep = atoi(value);
	if (!strcmp(token, "monlevel"))
		account->monlvl = atoi(value);
	if (!strcmp(token, "caid"))
		config_check_caidtab(value, &account->ctab);
	/*
	 *    case insensitive
	 */
	strtolower(value);
	if (!strcmp(token, "au"))
		for (i = 0; i < CS_MAXREADER; i++)
			if ((reader[i].label[0]) && (!strncmp(reader[i].label, value, strlen(reader[i].label))))
				account->au = i;
	if (!strcmp(token, "group"))
		for (ptr1 = strtok(value, ","); ptr1; ptr1 = strtok(NULL, ",")) {
			int g;

			g = atoi(ptr1);
			if ((g > 0) && (g < 33))
				account->grp |= (1 << (g - 1));
		}
	if (!strcmp(token, "services"))
		config_check_services(value, &account->sidtabok, &account->sidtabno);
	if (!strcmp(token, "ident"))
		config_check_ftab(value, &account->ftab, "user", account->usr, "provid");
	if (!strcmp(token, "class"))
		config_check_cltab(value, &account->cltab);
	if (!strcmp(token, "chid"))
		config_check_ftab(value, &account->fchid, "user", account->usr, "chid");

#ifdef CS_ANTICASC
	if (!strcmp(token, "numusers"))
		account->ac_users = atoi(value);
	if (!strcmp(token, "penalty"))
		account->ac_penalty = atoi(value);
#endif
}

int config_init_userdb()
{
	int tag = 0, nr, nro;

	FILE *fp;
	char *value;
	struct s_auth *ptr;
	struct s_auth *account = (struct s_auth *) 0;

	sprintf(token, "%s%s", cs_confdir, cs_user);
	if (!(fp = fopen(token, "r"))) {
		log_normal("Cannot open file \"%s\" (errno=%d)", token, errno);
		return (1);
	}
	for (nro = 0, ptr = cfg->account; ptr; nro++) {
		struct s_auth *ptr_next;

		ptr_next = ptr->next;
		free(ptr);
		ptr = ptr_next;
	}
	nr = 0;
	while (fgets(token, sizeof (token), fp)) {
		int i, l;
		void *ptr;

		if ((l = strlen(trim(token))) < 3)
			continue;
		if ((token[0] == '[') && (token[l - 1] == ']')) {
			token[l - 1] = 0;
			tag = (!strcmp("account", strtolower(token + 1)));
			if (!(ptr = malloc(sizeof (struct s_auth)))) {
				log_normal("Error allocating memory (errno=%d)", errno);
				return (1);
			}
			if (account)
				account->next = ptr;
			else
				cfg->account = ptr;
			account = ptr;
			memset(account, 0, sizeof (struct s_auth));
			account->au = (-1);
			account->monlvl = cfg->mon_level;
			account->tosleep = cfg->tosleep;
			for (i = 1; i < CS_MAXCAIDTAB; account->ctab.mask[i++] = 0xffff);
			for (i = 1; i < CS_MAXTUNTAB; account->ttab.bt_srvid[i++] = 0x0000);
			nr++;
#ifdef CS_ANTICASC
			account->ac_users = cfg->ac_users;
			account->ac_penalty = cfg->ac_penalty;
			account->ac_idx = nr;
#endif
			continue;
		}
		if (!tag)
			continue;
		if (!(value = strchr(token, '=')))
			continue;
		*value++ = '\0';
		config_check_account(trim(strtolower(token)), trim(value), account);
	}
	fclose(fp);
	log_normal("userdb reloaded: %d accounts freed, %d accounts loaded", nro, nr);
	return (0);
}

static void config_check_entry4sidtab(char *value, struct s_sidtab *sidtab, int what)
{
	int i, b;
	char *ptr;
	ushort *slist = (ushort *) 0;
	ulong *llist = (ulong *) 0;
	ulong caid;
	char buf[512];

	strncpy(buf, value, sizeof (buf));
	b = (what == 1) ? sizeof (ulong) : sizeof (ushort);
	for (i = 0, ptr = strtok(value, ","); ptr; ptr = strtok(NULL, ",")) {
		caid = a2i(ptr, b);
		if (!errno)
			i++;
	}
	if (!i)
		return;
	if (b == sizeof (ushort))
		slist = malloc(i * sizeof (ushort));
	else
		llist = malloc(i * sizeof (ulong));
	strcpy(value, buf);
	for (i = 0, ptr = strtok(value, ","); ptr; ptr = strtok(NULL, ",")) {
		caid = a2i(ptr, b);
		if (errno)
			continue;
		if (b == sizeof (ushort))
			slist[i++] = (ushort) caid;
		else
			llist[i++] = caid;
	}
	switch (what) {
		case 0:
			sidtab->caid = slist;
			sidtab->num_caid = i;
			break;
		case 1:
			sidtab->provid = llist;
			sidtab->num_provid = i;
			break;
		case 2:
			sidtab->srvid = slist;
			sidtab->num_srvid = i;
			break;
	}
}

static void config_check_sidtab(char *token, char *value, struct s_sidtab *sidtab)
{
	if (!strcmp(token, "caid"))
		config_check_entry4sidtab(value, sidtab, 0);
	if (!strcmp(token, "provid"))
		config_check_entry4sidtab(value, sidtab, 1);
	if (!strcmp(token, "ident"))
		config_check_entry4sidtab(value, sidtab, 1);
	if (!strcmp(token, "srvid"))
		config_check_entry4sidtab(value, sidtab, 2);
}

int config_init_sidtab()
{
	int nr, nro;
	FILE *fp;
	char *value;
	struct s_sidtab *ptr;
	struct s_sidtab *sidtab = (struct s_sidtab *) 0;

	sprintf(token, "%s%s", cs_confdir, cs_sidt);
	if (!(fp = fopen(token, "r"))) {
		log_normal("Cannot open file \"%s\" (errno=%d)", token, errno);
		return (1);
	}
	for (nro = 0, ptr = cfg->sidtab; ptr; nro++) {
		struct s_sidtab *ptr_next;

		ptr_next = ptr->next;
		if (ptr->caid)
			free(ptr->caid);
		if (ptr->provid)
			free(ptr->provid);
		if (ptr->srvid)
			free(ptr->srvid);
		free(ptr);
		ptr = ptr_next;
	}
	nr = 0;
	while (fgets(token, sizeof (token), fp)) {
		int l;
		void *ptr;

		if ((l = strlen(trim(token))) < 3)
			continue;
		if ((token[0] == '[') && (token[l - 1] == ']')) {
			token[l - 1] = 0;
			if (!(ptr = malloc(sizeof (struct s_sidtab)))) {
				log_normal("Error allocating memory (errno=%d)", errno);
				return (1);
			}
			if (sidtab)
				sidtab->next = ptr;
			else
				cfg->sidtab = ptr;
			sidtab = ptr;
			nr++;
			memset(sidtab, 0, sizeof (struct s_sidtab));
			strncpy(sidtab->label, strtolower(token + 1), sizeof (sidtab->label));
			continue;
		}
		if (!sidtab)
			continue;
		if (!(value = strchr(token, '=')))
			continue;
		*value++ = '\0';
		config_check_sidtab(trim(strtolower(token)), trim(strtolower(value)), sidtab);
	}
	fclose(fp);

#ifdef DEBUG_SIDTAB
	config_show_sidtab(cfg->sidtab);
#endif
	log_normal("services reloaded: %d services freed, %d services loaded", nro, nr);
	return (0);
}

int config_init_srvid()
{
	int nr;
	FILE *fp;
	char *value;
	static struct s_srvid *srvid = (struct s_srvid *) 0;

	sprintf(token, "%s%s", cs_confdir, cs_srid);
	if (!(fp = fopen(token, "r"))) {
		log_normal("can't open file \"%s\" (err=%d), no service-id's loaded", token, errno);
		return (0);
	}
	nr = 0;
	while (fgets(token, sizeof (token), fp)) {
		int l;
		void *ptr;

		if ((l = strlen(trim(token))) < 6)
			continue;
		if (!(value = strchr(token, ':')))
			continue;
		*value++ = '\0';
		if (strlen(token) != 4)
			continue;
		if (!(ptr = malloc(sizeof (struct s_srvid)))) {
			log_normal("Error allocating memory (errno=%d)", errno);
			return (1);
		}
		if (srvid)
			srvid->next = ptr;
		else
			cfg->srvid = ptr;
		srvid = ptr;
		memset(srvid, 0, sizeof (struct s_srvid));
		srvid->srvid = word_atob(token);
		strncpy(srvid->name, value, sizeof (srvid->name) - 1);
		nr++;
	}
	fclose(fp);
	log_normal("%d service-id's loaded", nr);
	return (0);
}

static void config_check_reader(char *token, char *value, struct s_reader *rdr)
{
	int i;
	char *ptr;

	/*
	 *    case sensitive first
	 */
	if (!strcmp(token, "device"))
		for (i = 0, ptr = strtok(value, ","); (i < 3) && (ptr); ptr = strtok(NULL, ","), i++) {
			trim(ptr);
			switch (i) {
				case 0:
					strncpy(rdr->device, ptr, sizeof (rdr->device) - 1);
					break;
				case 1:
					rdr->r_port = atoi(ptr);
					break;
				case 2:
					rdr->l_port = atoi(ptr);
					break;
			}
		}
	if (!strcmp(token, "key")) {
		if (key_atob14(value, rdr->ncd_key)) {
			fprintf(stderr, "Configuration newcamd: Error in Key\n");
			exit(1);
		}
	}
#ifdef CS_WITH_GBOX
	if (!strcmp(token, "password"))
		strncpy((char *) rdr->gbox_pwd, (const char *) i2b(4, a2i(value, 4)), 4);
	if (!strcmp(token, "premium"))
		rdr->gbox_prem = 1;
#endif
	if (!strcmp(token, "account"))
		for (i = 0, ptr = strtok(value, ","); (i < 2) && (ptr); ptr = strtok(NULL, ","), i++) {
			trim(ptr);
			switch (i) {
				case 0:
					strncpy(rdr->r_usr, ptr, sizeof (rdr->r_usr) - 1);
					break;
				case 1:
					strncpy(rdr->r_pwd, ptr, sizeof (rdr->r_pwd) - 1);
					break;
			}
		}
	if (!strcmp(token, "pincode"))
		strncpy(rdr->pincode, value, sizeof (rdr->pincode) - 1);
	/*
	 *    case insensitive
	 */
	strtolower(value);

	if (!strcmp(token, "services"))
		config_check_services(value, &rdr->sidtabok, &rdr->sidtabno);
	if (!strcmp(token, "inactivitytimeout"))
		rdr->tcp_ito = atoi(value);
	if (!strcmp(token, "reconnecttimeout"))
		rdr->tcp_rto = atoi(value);
	if (!strcmp(token, "disableserverfilter"))
		rdr->ncd_disable_server_filt = atoi(value);

	if (!strcmp(token, "label"))
		strncpy(rdr->label, value, sizeof (rdr->label) - 1);
	if (!strcmp(token, "fallback"))
		rdr->fallback = atoi(value) ? 1 : 0;
	if (!strcmp(token, "logport"))
		rdr->log_port = atoi(value);
	if (!strcmp(token, "caid"))
		config_check_caidtab(value, &rdr->ctab);
	if (!strcmp(token, "boxid"))
		rdr->boxid = a2i(value, 4);
	if (!strcmp(token, "aeskey")) {
		if (key_atob(value, rdr->aes_key)) {
			fprintf(stderr, "Configuration reader: Error in AES Key\n");
			exit(1);
		}
	}
	if (!strcmp(token, "detect")) {
		strtoupper(value);

		for (i = 0; RDR_CD_TXT[i]; i++) {
			if (!strcmp(value, RDR_CD_TXT[i]))
				rdr->detect = i;
			else if ((value[0] == '!') && (!strcmp(value + 1, RDR_CD_TXT[i])))
				rdr->detect = i | 0x80;
		}
	}
	if (!strcmp(token, "frequency")) {
		char *endptr;
		double mhz = strtod(value, &endptr);
		if (*endptr == '\0') {
			rdr->frequency = mhz * 1000000;
		}
	}
	if (!strcmp(token, "card_frequency")) {
		char *endptr;
		double mhz = strtod(value, &endptr);
		if (*endptr == '\0') {
			rdr->card_frequency = mhz * 1000000;
		}
	}
	if (!strcmp(token, "protocol")) {
		if (!strcmp(value, "phoenix"))
			rdr->type = R_PHOENIX;
		if (!strcmp(value, "smartmouse"))
			rdr->type = R_SMARTMOUSE;
		if (!strcmp(value, "internal"))
			rdr->type = R_INTERN;
		if (!strcmp(value, "intern"))
			rdr->type = R_INTERN;
		if (!strcmp(value, "smartreader+"))
			rdr->type = R_SMARTREADER;

		if (!strcmp(value, "serial"))
			rdr->type = R_SERIAL;

		if (!strcmp(value, "camd35"))
			rdr->type = R_CAMD35;
		if (!strcmp(value, "cs378x"))
			rdr->type = R_CS378X;
		if (!strcmp(value, "cs357x"))
			rdr->type = R_CAMD35;
#ifdef CS_WITH_GBOX
		if (!strcmp(value, "gbox"))
			rdr->type = R_GBOX;
#endif
		if (!strcmp(value, "newcamd") || !strcmp(value, "newcamd525")) {
			rdr->type = R_NEWCAMD;
			rdr->ncd_proto = NCD_525;
		}
		if (!strcmp(value, "newcamd524")) {
			rdr->type = R_NEWCAMD;
			rdr->ncd_proto = NCD_524;
		}
	}
	if (!strcmp(token, "ident"))
		config_check_ftab(value, &rdr->ftab, "reader", rdr->label, "provid");
	if (!strcmp(token, "class"))
		config_check_cltab(value, &rdr->cltab);
	if (!strcmp(token, "chid"))
		config_check_ftab(value, &rdr->fchid, "reader", rdr->label, "chid");
	if (!strcmp(token, "showcls"))
		rdr->show_cls = atoi(value);
	if (!strcmp(token, "maxqlen"))
		rdr->maxqlen = atoi(value);
	if (rdr->maxqlen < 0 || rdr->maxqlen > CS_MAXQLEN)
		rdr->maxqlen = CS_MAXQLEN;

	if (!strcmp(token, "group"))
		for (ptr = strtok(value, ","); ptr; ptr = strtok(NULL, ",")) {
			int g;

			g = atoi(ptr);
			if ((g > 0) && (g < 33))
				rdr->grp |= (1 << (g - 1));
		}
	if (!strcmp(token, "emmcache"))
		for (i = 0, ptr = strtok(value, ","); (i < 3) && (ptr); ptr = strtok(NULL, ","), i++)
			switch (i) {
				case 0:
					rdr->cachemm = atoi(ptr);
					break;
				case 1:
					rdr->rewritemm = atoi(ptr);
					break;
				case 2:
					rdr->logemm = atoi(ptr);
					break;
			}
	if (!strcmp(token, "blocknano"))
		for (ptr = strtok(value, ","); ptr; ptr = strtok(NULL, ","))
			if ((i = byte_atob(ptr)) >= 0)
				rdr->b_nano[i] = 1;
}

int config_init_readerdb()
{
	int tag = 0, nr;
	FILE *fp;
	char *value;

	sprintf(token, "%s%s", cs_confdir, cs_srvr);
	if (!(fp = fopen(token, "r"))) {
		log_normal("can't open file \"%s\" (errno=%d)\n", token, errno);
		return (1);
	}
	nr = 0;
	while (fgets(token, sizeof (token), fp)) {
		int i, l;

		if ((l = strlen(trim(token))) < 3)
			continue;
		if ((token[0] == '[') && (token[l - 1] == ']')) {
			token[l - 1] = 0;
			tag = (!strcmp("reader", strtolower(token + 1)));
			if (reader[nr].label[0] && reader[nr].type)
				nr++;
			memset(&reader[nr], 0, sizeof (struct s_reader));
			reader[nr].tcp_rto = 30;
			reader[nr].show_cls = 10;
			reader[nr].maxqlen = CS_MAXQLEN;
			reader[nr].frequency = 3571200;
			reader[nr].card_frequency = 3571200;
			strcpy(reader[nr].pincode, "none");
			for (i = 1; i < CS_MAXCAIDTAB; reader[nr].ctab.mask[i++] = 0xffff);
			continue;
		}
		if (!tag)
			continue;
		if (!(value = strchr(token, '=')))
			continue;
		*value++ = '\0';
		config_check_reader(trim(strtolower(token)), trim(value), &reader[nr]);
	}
	fclose(fp);
	return (0);
}

#ifdef CS_ANTICASC
void config_init_ac()
{
	int nr;
	FILE *fp;

	sprintf(token, "%s%s", cs_confdir, cs_ac);
	if (!(fp = fopen(token, "r"))) {
		log_normal("can't open file \"%s\" (errno=%d) anti-cascading table not loaded", token, errno);
		return;
	}

	for (nr = 0; fgets(token, sizeof (token), fp);) {
		int i, skip;
		ushort caid, sid, chid, dwtime;
		ulong provid;
		char *ptr, *ptr1;
		struct s_cpmap *ptr_cpmap;
		static struct s_cpmap *cpmap = (struct s_cpmap *) 0;

		if (strlen(token) < 4)
			continue;

		caid = sid = chid = dwtime = 0;
		provid = 0;
		skip = 0;
		ptr1 = 0;
		for (i = 0, ptr = strtok(token, "="); (i < 2) && (ptr); ptr = strtok(NULL, "="), i++) {
			trim(ptr);
			if (*ptr == ';' || *ptr == '#' || *ptr == '-') {
				skip = 1;
				break;
			}
			switch (i) {
				case 0:
					ptr1 = ptr;
					break;
				case 1:
					dwtime = atoi(ptr);
					break;
			}
		}

		if (!skip) {
			for (i = 0, ptr = strtok(ptr1, ":"); (i < 4) && (ptr); ptr = strtok(NULL, ":"), i++) {
				trim(ptr);
				switch (i) {
					case 0:
						if (*ptr == '*')
							caid = 0;
						else
							caid = a2i(ptr, 4);
						break;
					case 1:
						if (*ptr == '*')
							provid = 0;
						else
							provid = a2i(ptr, 6);
						break;
					case 2:
						if (*ptr == '*')
							sid = 0;
						else
							sid = a2i(ptr, 4);
						break;
					case 3:
						if (*ptr == '*')
							chid = 0;
						else
							chid = a2i(ptr, 4);
						break;
				}
			}
			if (!(ptr_cpmap = (struct s_cpmap *) malloc(sizeof (struct s_cpmap)))) {
				log_normal("Error allocating memory (errno=%d)", errno);
				return;
			}
			if (cpmap)
				cpmap->next = ptr_cpmap;
			else
				cfg->cpmap = ptr_cpmap;
			cpmap = ptr_cpmap;

			cpmap->caid = caid;
			cpmap->provid = provid;
			cpmap->sid = sid;
			cpmap->chid = chid;
			cpmap->dwtime = dwtime;
			cpmap->next = 0;

			log_debug("nr=%d, caid=%04X, provid=%06X, sid=%04X, chid=%04X, dwtime=%d", nr, caid, provid, sid, chid, dwtime);
			nr++;
		}
	}
	fclose(fp);
}
#endif
