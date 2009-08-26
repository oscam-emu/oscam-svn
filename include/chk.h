#ifndef __CHK_H__
#  define __CHK_H__

int chk_srvid(ECM_REQUEST *, int);
int chk_sfilter(ECM_REQUEST *, PTAB *);
int chk_ufilters(ECM_REQUEST *);
int chk_rfilter(ECM_REQUEST *, struct s_reader *);
int chk_rsfilter(ECM_REQUEST *, int);
int chk_avail_reader(ECM_REQUEST *, struct s_reader *);
int matching_reader(ECM_REQUEST *, struct s_reader *);

#endif // __CHK_H__
