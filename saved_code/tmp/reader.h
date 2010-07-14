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
// 'base' class (I call them parent classes) should be define
// first and all other reader class should be derived from it 
// All member variables should be prefixed by m_ to make the code
// more readable (see class t_Reader).
// ineritance should go like this
// t_Reader -> t_ReaderXXX
// t_ReaderXXX can be a parent class for other class ( like t_readerProxy
//  below).
// each derived reader should take care of its specific parameter 
// (the common one are dealt with by the parent class.
// 
// PLEASE SET YOUR EDITOR TO UNIX MODE !!!!!!! lines end with LF .. NOT CRLF !!!
//
class t_readerInterface : public cThread
{
public:
	t_readerInterface() : cThread("reader", 0) {} // todo
	virtual ~t_readerInterface() {}

	virtual string readerName() = 0;
	virtual string supportedCaids() = 0;
	virtual bool   isEnabled() = 0;
	virtual void   reloadConfig() = 0;
	virtual void   initReader() = 0;
};

//---------------------------------------------------------------------------
class t_Reader : public t_readerInterface, public t_simples
{
public:
    // this class shouldn't open the config file 
    // this should be done by another class that takes care of
    // reading the config and then instantiating the proper object
    // in this case a reader, by calling its constructor and providing
    // it a pointer to a valid config for the reader.
    // as this is the parent class it should only take care of the
    // properties that are common to all readers (label, protocol,..)
	t_Reader(t_readCfgSection *readCfgSection);
	virtual ~t_Reader() {}
	
protected:
	string m_label;
	string m_protocol;
	// should contain the reader config.
	// I haven't look at the t_readCfgSection so my guess
	// is that's not the right class. There should be a readerConfig
	// class.
	t_readCfgSection *m_readCfgSection;
    
    // shouldn't this be in a class representing the reader configuration ?
	int    m_reconTimeout, m_idleTimeout;
	uint   m_group;
	bool   m_fallback, m_enabled;
	string  m_caidsString;
	CAIDTAB m_caidTab;
	FTAB    m_ftab;
    
    // initReader should use the m_readCfgSection
	void initReader();
	string readerName() { return label; }
	string supportedCaids() { return caidsString.length() ? caidsString : "not defiend"; }
	bool   isEnabled() { return enabled; }
};

//---------------------------------------------------------------------------
class t_readerProxy : protected t_Reader
{
public:
	t_readerProxy(t_readCfgSection *readCfgSection);
	virtual ~t_readerProxy() {}

protected:
	cTimeMs activity;
	bool    connected;
	string  host;
	string  user, pass;
	int     port;
	int     conTimeout, rwTimeout; //??
	// TODO network class

	void initReader();

};

//---------------------------------------------------------------------------
class t_readerCCcam : protected t_readerProxy
{
public:
	t_readerCCcam(int Priority);
	~t_readerCCcam();

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
};


//---------------------------------------------------------------------------

#endif /* READER_H_ */
