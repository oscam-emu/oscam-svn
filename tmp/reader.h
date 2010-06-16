/*
 * reader.h
 *
 *      Author: aston
 */
#ifndef READER_H_
#define READER_H_

#include "cthread.h"
#include "simples.h"
#include "config/readCfgSection.h"

class t_readerInterface : public cThread
{
public:
	t_readerInterface() : cThread("reader", 0) {} // todo
	virtual ~t_readerInterface() {}

	virtual string readerName() = 0;
	virtual string supportedCaids() = 0;
	virtual bool   isEnabled() = 0;
	virtual void   reloadConfig() = 0;
	virtual void   initReader(string fileName, string label) = 0;
};

//---------------------------------------------------------------------------
class t_baseReader : public t_readerInterface, public t_simples
{
protected:
	string fileName;
	string label;
	string protocol;
	t_readCfgSection *readCfgSection;

	int    reconTimeout, idleTimeout;
	uint   group;
	bool   fallback, enabled;
	string  caidsString;
	CAIDTAB caidTab;
	FTAB    ftab;

	void initReaderBase();
	string readerName() { return label; }
	string supportedCaids() { return caidsString.length() ? caidsString : "not defiend"; }
	bool   isEnabled() { return enabled; }
public:
	t_baseReader();
	virtual ~t_baseReader() {}
};

//---------------------------------------------------------------------------
class t_readerProxy : protected t_baseReader
{
protected:
	cTimeMs activity;
	bool    connected;
	string  host;
	string  user, pass;
	int     port;
	int     conTimeout, rwTimeout; //??
	// TODO network class

	void initReaderProxy();
public:
	t_readerProxy();
	virtual ~t_readerProxy() {}
};

//---------------------------------------------------------------------------
class t_readerCCcam : protected t_readerProxy
{
private:
	bool disableAutoBlock;
	bool disableRetryEcm;

	string version, build;
	int max_hop;

protected:
	void Execute(); // main loop
	void reloadConfig();
	void initReaderProxy() {}
	void initReader(string fileName, string label);
public:
	t_readerCCcam(int Priority);
	~t_readerCCcam();
};


//---------------------------------------------------------------------------

#endif /* READER_H_ */
