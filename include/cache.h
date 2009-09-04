#ifndef __CACHE_H__
#  define __CACHE_H__

void cache_store_ecm(ECM_REQUEST *);
int cache_lookup_ecm(ECM_REQUEST *, ulong);

#endif // __CACHE_H__
