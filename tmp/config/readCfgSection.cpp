/*
 * readCfgSection.c
 *
 *      Author: aston
 */

#include "readCfgSection.h"
#include "../global.h"

//---------------------------------------------------------------------------
t_readCfgSection::t_readCfgSection()
{
   configFile = NULL;

}
//-------------------------------------------
t_readCfgSection::~t_readCfgSection()
{
   if (configFile)
    { configFile->close(); delete configFile; }
}

//-------------------------------------------
void t_readCfgSection::initSection(string* fileName, string* label)
{
   if (configFile)
    { configFile->close(); delete configFile; }
   configFile = new ifstream;
   configFile->open(fileName->c_str());
   if (!configFile->is_open())
      throw StandardException("Can not open config file %s", fileName->c_str());

   lineNumber = 0; bool foundSection = false;
   position = -1;
   while(!configFile->eof()) {
	  string line = readLine();
	  int len = line.length();
	  if (!foundSection) {
	     if (line[0] == '[' && line[len - 1] == ']')
		    if (line.compare("[reader]") == 0)
		      { foundSection = true; continue; }
	     continue;
	  }
	  size_t pos;
	  if ((pos = line.find("label", 0)) == string::npos)
		 throw StandardException("Error in config file %s at line %d expected 'label'!", fileName->c_str(), lineNumber);
	  if ((pos = line.find('=', 0)) == string::npos)
	     throw StandardException("Error in config file %s at line %d expected '='!", fileName->c_str(), lineNumber);
	  string value = line.substr(pos + 1, len - pos);
	  trim(&value);
	  if (value.compare(*label) == 0) {
		 position = configFile->tellg();
		 startLineNum = lineNumber;
		 break;
	  }
	  foundSection = false;
   }
   if (!weHaveSection())
	  throw StandardException("Can not find section with label %s in config file %s", label->c_str(), fileName->c_str());
}

//-------------------------------------------
string t_readCfgSection::readLine()
{
   if (!configFile)
	  throw StandardException("Config file is not open!");
   string line;
   int len;
   do {
      getline(*configFile, line);
      lineNumber++;
      trim(&line); strToLower(&line);
      len = line.length();
   } while ((!configFile->eof() && line[0] == '#') || len < 3);

   if (weHaveSection()) {
	  if (line[0] == '[' && line[len - 1] == ']') {
		 lineNumber = startLineNum;
		 configFile->seekg(position);
		 line = "end";
	  }
	  else if (line.find("=", 0) == string::npos)
		 throw StandardException("Error in config file at line %d can not find '=' in line!", lineNumber);
   }
   return line;
}

//-------------------------------------------
bool t_readCfgSection::getLineData(string *token, string *value)
{
   *token = readLine();
   if (token->compare("end") == 0)
      return false;
   size_t pos = token->find("=", 0);
   string tmp = token->substr(pos + 1, token->length() - pos);
   token->erase(pos - 1); trim(token);
   value->clear();
   for (int i = 0; i < (int)tmp.length(); i++)
	  if (tmp[i] != ' ') *value += tmp[i];
   return true;
}
//---------------------------------------------------------------------------
