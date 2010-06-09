/*
 * simples.c
 *
 *      Author: aston
 */

#include "simples.h"

//---------------------------------------------------------------------------
t_simples::t_simples()
{
	inet_byteorder = 0;
}
//---------------------------------------------------------------------------
t_simples::~t_simples()
{
}

//---------------------------------------------------------------------------
int t_simples::key_atob14(char *asc, uchar *bin)
{
  int i, n1, n2, rc;
  for (i=rc=0; i<28; i+=2)
  {
    if ((n1=gethexval(asc[i  ]))<0) rc=(-1);
    if ((n2=gethexval(asc[i+1]))<0) rc=(-1);
    bin[i>>1]=(n1<<4)+(n2&0xff);
  }
  return(rc);
}

//---------------------------------------------------------------------------
ulong t_simples::a2i(char *asc, int bytes)
{
  int i, n;
  ulong rc;
  for (rc = i = 0, n = strlen(trim(asc)) - 1; i < (abs(bytes) << 1); n--, i++)
    if (n >= 0)
    {
      int rcl;
      if ((rcl = gethexval(asc[n])) < 0)
      {
        errno = EINVAL;
        return(0x1F1F1F);
      }
      rc |= (rcl << (i << 2));
    }
    else
      if (bytes < 0) rc |= (0xf << (i << 2));

  errno = 0;
  return(rc);
}

//---------------------------------------------------------------------------
in_addr_t t_simples::cs_inet_order(in_addr_t n)
{
  if (!inet_byteorder)
    inet_byteorder=((inet_addr("1.2.3.4")+1)==inet_addr("1.2.3.5")) ? 1 : 2;
  switch (inet_byteorder)
  {
    case 1:
      break;
    case 2:
      n=((n&0xff000000) >> 24 ) |
        ((n&0x00ff0000) >>  8 ) |
        ((n&0x0000ff00) <<  8 ) |
        ((n&0x000000ff) << 24 );
      break;
  }
  return(n);
}

//---------------------------------------------------------------------------
in_addr_t t_simples::cs_inet_addr(char *txt)
{
  if (!inet_byteorder)
    inet_byteorder=((inet_addr("1.2.3.4")+1)==inet_addr("1.2.3.5")) ? 1 : 2;
  if (inet_byteorder == 1)
    return(inet_addr(txt));
  else
    return(inet_network(txt));
}

//---------------------------------------------------------------------------
char *t_simples::trim(char *txt)
{
  register int l;
  register char *p1, *p2;

  if (*txt==' ' || *txt=='\t')
  {
    for (p1=p2=txt;
        (*p1==' ') || (*p1=='\t') || (*p1=='\n') || (*p1=='\r');
         p1++);
    while (*p1)
      *p2++=*p1++;
    *p2='\0';
  }
  if ((l=strlen(txt))>0)
    for (p1=txt+l-1;
        (*p1==' ') || (*p1=='\t') || (*p1=='\n') || (*p1=='\r');
         *p1--='\0');

  return(txt);
}

//---------------------------------------------------------------------------
void t_simples::trim(string *str)
{
   if (str->empty())
	 return;
   string searchFor(" \t\f\v\n\r");
   size_t pos = str->find_first_not_of(searchFor);
   if (pos != string::npos)
      str->erase(0, pos);
   pos = str->find_last_not_of(searchFor);
   if (pos != string::npos)
      str->erase(pos + 1);
}

//---------------------------------------------------------------------------
void t_simples::strToLower(string *str)
{
   if (str->empty())
	 return;
   char *pointer = (char*)str->c_str();
   for (; *pointer; pointer++)
      if (isupper((char)*pointer))
    	 *pointer = tolower((char)*pointer);
}

//---------------------------------------------------------------------------
int t_simples::gethexval(char c)
{
  if ((c>='0') && (c<='9')) return(c-'0');
  if ((c>='A') && (c<='F')) return(c-'A'+10);
  if ((c>='a') && (c<='f')) return(c-'a'+10);
  return(-1);
}

//---------------------------------------------------------------------------
int t_simples::key_atob(char *asc, uchar *bin)
{
  int i, n1, n2, rc;
  for (i = rc = 0; i < 32; i += 2)
  {
    if ((n1 = gethexval(asc[i  ])) < 0) rc = (-1);
    if ((n2 = gethexval(asc[i+1])) < 0) rc = (-1);
    bin[i>>1] = (n1<<4) + (n2&0xff);
  }
  return(rc);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

