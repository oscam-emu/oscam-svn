/*
 * client.c
 *
 *      Author: aston
 */

#include "client.h"

//---------------------------------------------------------------------------
t_client::t_client(char *Description, int Priority) : cThread(Description, Priority)
{
	// TODO Auto-generated constructor stub

}

//---------------------------------------------------------------------------
t_client::~t_client()
{
	// TODO Auto-generated destructor stub
}


//---------------------------------------------------------------------------
// Execute - main function
//---------------------------------------------------------------------------
void t_client::Execute(void)
{
	while (!Terminated()) {

	}
}
//---------------------------------------------------------------------------
