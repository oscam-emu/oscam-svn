/*
 * reader.h
 *
 *      Author: aston
 */
#ifndef READER_H_
#define READER_H_

#include "cthread.h"

//---------------------------------------------------------------------------
class t_reader : public cThread
{
protected:
	 void Execute(void);

public:
	 t_reader(char *Description, int Priority);
	~t_reader();
};
//---------------------------------------------------------------------------

#endif /* READER_H_ */
