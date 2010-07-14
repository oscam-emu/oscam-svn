/*
 * readCfgSection.h
 *
 *      Author: aston
 */

#ifndef READ_CFG_SECTION_H_
#define READ_CFG_SECTION_H_

#include <fstream>
#include <string.h>

#include "../simples.h"

using namespace std;

//---------------------------------------------------------------------------
class t_readCfgSection : public t_simples
{
private:
	ifstream *configFile;
	int lineNumber, startLineNum;
	int position;
	bool weHaveSection()  { return position > -1 ? true : false ; }
	string readLine();
public:
	 t_readCfgSection();
	~t_readCfgSection();
	void initSection(string *fileName, string *label);
	int  getLineNumber()  { return lineNumber; }
	bool getLineData(string *token, string *value);
};
//---------------------------------------------------------------------------
#endif /* READ_CFG_SECTION_H_ */
