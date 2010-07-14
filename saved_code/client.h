/*
 * client.h
 *
 *      Author: aston
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "cthread.h"

class t_client : public cThread
{
protected:
	 void Execute(void);

public:
	 t_client(char *Description, int Priority);
	~t_client();
};

#endif /* CLIENT_H_ */
