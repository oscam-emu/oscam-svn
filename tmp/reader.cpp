/*
 * reader.c
 *
 *      Author: aston
 */

// PLEASE SET YOUR EDITOR TO UNIX MODE !!!!!!! lines end with LF .. NOT CRLF !!!

#include "reader.h"
#include "logger.h"

#include "t_Reader.h"

//---------------------------------------------------------------------------
t_baseReader::t_Reader(t_readCfgSection *readCfgSection)
{
   m_readCfgSection = readCfgSection;
   m_group = 0;
   m_fallback = m_enabled = false;
   m_reconTimeout = m_idleTimeout = 0;

   memset(&m_caidTab, 0,  sizeof(CAIDTAB));
   memset(&m_ftab, 0, sizeof(FTAB));
}

//-------------------------------------------
// this method shouldn't read the confiog file...
// this should be done by another class that only does 
// config parsing and config object creation.

void t_baseReader::initReader()
{
   if (!m_readCfgSection)
	  return;

   while(1) {
	  string token, value;
	  if (!m_readCfgSection->getLineData(&token, &value))
	 	 break;

	  if (token.compare("enabled") == 0) {
		 enabled = value.length() ? atoi(value.c_str()) : false;
		 continue;
	  }
	  if (token.compare("fallback") == 0) {
		 fallback = value.length() ? atoi(value.c_str()) : false;
		 continue;
	  }
	  if (token.compare("reconnecttimeout") == 0) {
		 reconTimeout = value.length() ? atoi(value.c_str()) : 0;
		 continue;
	  }
	  if (token.compare("idletimeout") == 0) {
		 idleTimeout = value.length() ? atoi(value.c_str()) : 0;
		 continue;
	  }
	  if (token.compare("group") == 0 && value.length()) {
		 size_t pos; value += ",";
		 while((pos = value.find(",")) != string::npos) {
            string tmp = value.substr(0, pos);
            value.erase(0, pos + 1);
            int grp = atoi(tmp.c_str());
            if ((grp > 0) && (grp < 33))
               group |= (1 << (grp - 1));
		 }
		 continue;
	  }
	  if (token.compare("caid") == 0 && value.length()) {
		 caidsString = value;
		 size_t pos; value += ","; int index = 0;
		 while((pos = value.find(",")) != string::npos && index < MAXCAIDTAB) {
	         string tmp = value.substr(0, pos);
	         value.erase(0, pos + 1);
	         ulong caid = 0x0000, mask = 0xffff;
	         if ((pos = tmp.find("&")) != string::npos) {
	        	if (pos != 4)
	 			   throw StandardException("Syntax error in config file %s at line %d",
	 			    		                fileName.c_str(), readCfgSection->getLineNumber());
	        	string msk = tmp.substr(pos + 1, tmp.length() - pos + 1);
                mask = a2i((char*)msk.c_str(), 2);
	         }
	         else pos = tmp.length();
	         if (pos != 4)
	 		    throw StandardException("Syntax error in config file %s at line %d",
	 			    		             fileName.c_str(), readCfgSection->getLineNumber());
	         tmp = tmp.substr(0, pos);
	         caid = a2i((char*)tmp.c_str(), 2);

	         caidTab.caid[index]   = (caid >= 0x1000) ? 0x0000 : caid;
	         caidTab.mask[index++] = (caid >= 0x1000) ? 0xffff : mask;
		 }
		 continue;
	  }
   }
}

//---------------------------------------------------------------------------
t_readerCCcam::t_readerCCcam(int Priority)// : cThread(label.c_str(), Priority)
{
   protocol = "cccam";
}

//-------------------------------------------
t_readerCCcam::~t_readerCCcam()
{

}

//-------------------------------------------
// Execute - main function
//-------------------------------------------
void t_readerCCcam::Execute(void)
{
	while (!Terminated()) {

	}
}

//-------------------------------------------
void t_readerCCcam::reloadConfig()
{

}

//-------------------------------------------
void t_readerCCcam::initReader(string fileName, string label)
{
   readCfgSection = NULL;
   this->fileName = fileName;
   this->label = label;
   try {
	  readCfgSection = new t_readCfgSection();
	  readCfgSection->initSection(&fileName, &label);

	  t_Reader::initReader(string fileName, string label);
	  initReader();

	  while(1) {
		 string token, value;
		 if (!readCfgSection->getLineData(&token, &value))
			break;

		 if (token.compare("protocol") == 0) {
		    if (value.compare(protocol) == 0)
			   continue;
		    throw StandardException("Internal error: Protocol must be %s instead of %s!",
		    		                 protocol.c_str(), value.c_str());
		 }
		 if (token.compare("disableautoblock") == 0) {
			disableAutoBlock = value.length() ? atoi(value.c_str()) : false;
			continue;
		 }
		 if (token.compare("disableretryecm") == 0) {
			disableRetryEcm = value.length() ? atoi(value.c_str()) : false;
			continue;
		 }
	  }

   }
   catch (StandardException& e) {
	  enabled = false;
	  throw StandardException(e.descrptChar());
   }
   if (readCfgSection)
	  delete readCfgSection;
}


//---------------------------------------------------------------------------
/*
cTimeMs activity;
bool    connected;
string  host;
string  user, pass;
int     port, conTimeout, rwTimeout;
*/
t_readerProxy::t_readerProxy()
{
   connected = false;
   port = conTimeout = rwTimeout = 0;
}

//-------------------------------------------
void t_readerProxy::initReaderProxy()
{
   if (!readCfgSection)
	  return;

   while(1) {
	  string token, value;
	  if (!readCfgSection->getLineData(&token, &value))
	 	 break;
	  if (token.compare("account") == 0 && value.length()) {
	     size_t pos;
	     if ((pos = value.find(",")) == string::npos)
			throw StandardException("Syntax error in config file %s at line %d",
			    		             fileName.c_str(), readCfgSection->getLineNumber());
		 user = value.substr(0, pos);
		 pass = value.substr(++pos, value.length() - pos);
	  }
	  if (token.compare("device") == 0) {
		 size_t pos;
		 if ((pos = value.find(',', 0)) == string::npos)
		    throw StandardException("Syntax error in config file %s at line %d",
									 fileName.c_str(), readCfgSection->getLineNumber());
		 host  = value.substr(0, pos);
		 value = value.substr(++pos, value.length() - pos);
		 port  = atoi(value.c_str());
		 continue;
	  }

   }
}
//---------------------------------------------------------------------------
