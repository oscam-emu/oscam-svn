/*
 * reader.c
 *
 *      Author: aston
 */

#include "reader.h"

//---------------------------------------------------------------------------
t_reader::t_reader(char *Description, int Priority) : cThread(Description, Priority)
{
	// TODO Auto-generated constructor stub

}

//---------------------------------------------------------------------------
t_reader::~t_reader()
{
	// TODO Auto-generated destructor stub
}

//---------------------------------------------------------------------------
// Execute - main function
//---------------------------------------------------------------------------
void t_reader::Execute(void)
{
	while (!Terminated()) {

	}
}
//---------------------------------------------------------------------------

