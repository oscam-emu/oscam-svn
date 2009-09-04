#include "globals.h"
#include "cache.h"

#include "log.h"

extern int *ecmidx;

void cache_store_ecm(ECM_REQUEST *er)
{
//	log_normal("store ecm from reader %d", er->reader[0]);
	memcpy(ecmcache[*ecmidx].ecmd5, er->ecmd5, CS_ECMSTORESIZE);
	memcpy(ecmcache[*ecmidx].cw, er->cw, 16);
	ecmcache[*ecmidx].caid = er->caid;
	ecmcache[*ecmidx].prid = er->prid;
	ecmcache[*ecmidx].grp = reader[er->reader[0]].grp;
//	log_ddump(ecmcache[*ecmidx].ecmd5, CS_ECMSTORESIZE, "ECM stored (idx=%d)", *ecmidx);
	*ecmidx = (*ecmidx + 1) % CS_ECMCACHESIZE;
}

int cache_lookup_ecm(ECM_REQUEST *er, ulong grp)
{
	int i;

//	log_ddump(ecmd5, CS_ECMSTORESIZE, "ECM search");
//	log_normal("cache CHECK: grp=%lX", grp);
	for (i = 0; i < CS_ECMCACHESIZE; i++) {
		if ((grp & ecmcache[i].grp) && (!memcmp(ecmcache[i].ecmd5, er->ecmd5, CS_ECMSTORESIZE))) {
//			log_normal("cache found: grp=%lX cgrp=%lX", grp, ecmcache[i].grp);
			memcpy(er->cw, ecmcache[i].cw, 16);
			return 1;
		}
	}

	return 0;
}
