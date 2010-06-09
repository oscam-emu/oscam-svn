/*
 * oscam.c
 *
 *      Author: aston
 */

#include <typeinfo>
#include <stdexcept>

#include "main.h"

t_main *mainClass;
//---------------------------------------------------------------------------
int main (int argc, char *argv[])
{
   mainClass = new t_main();
   try {
	   mainClass->run();
	   cout << "oscam exit -> normal" << endl;
   }
   catch (StandardException& e) {
	  cerr << e.descrptChar() << endl;
	  cerr << "oscam exit -> with error!" << endl;
   }
   delete mainClass;
}
//---------------------------------------------------------------------------
