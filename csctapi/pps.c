/*
    pps.c
    Protocol Parameters Selection
  
    This file is part of the Unix driver for Towitoko smartcard readers
    Copyright (C) 2000 2001 Carlos Prados <cprados@yahoo.com>

    This version is modified by doz21 to work in a special manner ;)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "pps.h"
#include "atr.h"
#include "protocol_t0.h"
#include "protocol_t1.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Not exported constants definition
 */

#define PPS_DEFAULT_PROTOCOL	0x00

/*
 * Not exported macros definition
 */

#define PPS_HAS_PPS1(block)	((block[1] & 0x10) == 0x10)
#define PPS_HAS_PPS2(block)	((block[1] & 0x20) == 0x20)
#define PPS_HAS_PPS3(block)	((block[1] & 0x40) == 0x40)

/*
 * Not exported funtions declaration
 */

static int PPS_Exchange (PPS * pps, BYTE * params, unsigned *length);

static bool PPS_Match (BYTE * request, unsigned len_request, BYTE * reply, unsigned len_reply);

static unsigned PPS_GetLength (BYTE * block);

static int PPS_InitICC (PPS * pps, int protocol_selected);

static int PPS_InitProtocol (PPS * pps, int protocol_selected);

static BYTE PPS_GetPCK (BYTE * block, unsigned length);

/*
 * Exported functions definition
 */

PPS * PPS_New (ICC_Async * icc)
{
	PPS *pps;
	
	pps = (PPS *) malloc (sizeof (PPS));
	
	if (pps != NULL)
	{
		pps->icc = icc;
		pps->protocol = NULL;
		pps->parameters.t = PPS_DEFAULT_PROTOCOL;
		pps->parameters.FI = ATR_DEFAULT_FI;
		pps->parameters.d = ATR_DEFAULT_D;
		pps->parameters.n = ATR_DEFAULT_N;
	}
	
	return pps;
}

int PPS_Perform (PPS * pps, BYTE * params, unsigned *length)
{
	//Performs PPS Exchange on command when *length >0
	//If unsuccesfull, or when length <= 0
	//Gets parametes from ATR
	//If necessary perform PPS session
	//
	//Output is pps->params.FI,n,d,t set with correct values
	//and switched SC-device conform these values to correct baudrate
	//
	//We need to store FI instread of F, because SCI_DEV works with FI
	//and it is easier to overclock then
	//also from FI -> F is easy, other way around not
	
	ATR *atr;
	int ret;
	bool PPS_success; 
	
	/* Perform PPS Exchange if requested by command */
	if ((*length) > 0)
	{
		ret = PPS_Exchange (pps, params, length);
		
		/* Get parameters from PPS handsake */
		if (ret == PPS_OK)
		{
			pps->parameters.t = params[1] & 0x0F;
			
			if (PPS_HAS_PPS1 (params))
			{
				pps->parameters.FI = (params[2] >> 4);
				pps->parameters.d = atr_d_table[(params[2] & 0x0F)];
			}
			
/*			
			ret  = PPS_InitICC(pps);
			
			if (ret != PPS_OK)
				return ret;
*/
		}
/*		
		else
		{
			return ret;
		}
*/
	}
	PPS_success = PPS_OK;
	int protocol_selected = 0; //stores which TAi,TBi etc. bytes must be used 0 means not set
	atr = ICC_Async_GetAtr (pps->icc);
	if ((*length) <= 0 || !PPS_success) // If not by command, or PPS Exchange by command failed: Try PPS Exchange by ATR or Get parameters from ATR
	{
		int numprot = atr->pn;//number of protocol lines in ATR
		BYTE tx;
	  cs_debug("ATR reports smartcard supports %i protocols:",numprot);
		int i,point;
		char txt[50];
		bool OffersT[2]; //T14 stored as T2
		for (i = 0; i <= 2; i++)
			OffersT[i] = FALSE;
		for (i=1; i<= numprot; i++) {
			sprintf(txt,"Protocol %01i:  ",i);
			point = 12;
			if (ATR_GetInterfaceByte (atr, i, ATR_INTERFACE_BYTE_TA, &tx) == ATR_OK) {
			  sprintf((char *)txt+point,"TA%i=%02X ",i,tx);
				point +=7;
			}
			if (ATR_GetInterfaceByte (atr, i, ATR_INTERFACE_BYTE_TB, &tx) == ATR_OK) {
			  sprintf((char *)txt+point,"TB%i=%02X ",i,tx);
				point +=7;
			}
			if (ATR_GetInterfaceByte (atr, i, ATR_INTERFACE_BYTE_TC, &tx) == ATR_OK) {
			  sprintf((char *)txt+point,"TC%i=%02X ",i,tx);
				point +=7;
			}
			if (ATR_GetInterfaceByte (atr, i, ATR_INTERFACE_BYTE_TD, &tx) == ATR_OK) {
			  sprintf((char *)txt+point,"TD%i=%02X ",i,tx);
				point +=7;
				tx &= 0X0F;
			  sprintf((char *)txt+point,"(T%i)",tx);
				if (tx == 14)
					OffersT[2] = TRUE;
				else
					OffersT[tx] = TRUE;
			}
			else {
				sprintf((char *)txt+point,"no TD%i means T0",i);
				OffersT[0] = TRUE;
			}
			cs_debug("%s",txt);
		}
    
		int numprottype = 0;
		for (i = 0; i <= 2; i++)
			if (OffersT[i])
				numprottype ++;
		// cs_debug("%i protocol types detected. Historical bytes: %s",numprottype, cs_hexdump(1,atr->hb,atr->hbn));

//If more than one protocol type and/or TA1 parameter values other than the default values and/or N equeal to 255 is/are indicated in the answer to reset, the card shall know unambiguously, after having sent the answer to reset, which protocol type or/and transmission parameter values (FI, D, N) will be used. Consequently a selection of the protocol type and/or the transmission parameters values shall be specified.
		ATR_GetParameter (atr, ATR_PARAMETER_N, &(pps->parameters.n));
		ATR_GetProtocolType(atr,1,&(pps->parameters.t)); //get protocol from TD1
		bool NeedsPTS = ((pps->parameters.t != 14) && (numprottype > 1 || (atr->ib[0][ATR_INTERFACE_BYTE_TA].present == TRUE && atr->ib[0][ATR_INTERFACE_BYTE_TA].value != 0x11) || pps->parameters.n == 255)); //needs PTS according to ISO 7816 , SCI gets stuck on our PTS
		if (NeedsPTS && (pps->icc->ifd->io->com != RTYP_SCI)) {
			//             PTSS  PTS0  PTS1  PTS2  PTS3  PCK
			//             PTSS  PTS0  PTS1  PCK
			BYTE req[] = { 0xFF, 0x10, 0x00, 0x00 }; //we currently do not support PTS2, standard guardtimes

			int p; 
			for (p=1; p<=numprot; p++) {
				ATR_GetProtocolType(atr,p,&(pps->parameters.t));
				req[1]=0x10 | pps->parameters.t; //PTS0 always flags PTS1 to be sent always
				if (ATR_GetInterfaceByte (atr, p, ATR_INTERFACE_BYTE_TA, &req[2]) != ATR_OK)  //PTS1 
					req[2] = 0x11; //defaults FI and DI to 1
	  		//req[3]=PPS_GetPCK(req,sizeof(req)-1); will be set by PPS_Exchange
				unsigned int len = sizeof(req);
				ret = PPS_Exchange (pps, req, &len);
		  	if (ret == PPS_OK) {
					pps->parameters.FI = req[2] >> 4;
					BYTE DI = req[2] & 0x0F;
					pps->parameters.d = (double) (atr_d_table[DI]);
					PPS_success = TRUE;
					protocol_selected = p;
					cs_debug("PTS Succesfull, selected protocol %i: T%i, F=%.0f, D=%.6f, N=%.0f\n", protocol_selected, pps->parameters.t, (double) atr_f_table[pps->parameters.FI], pps->parameters.d, pps->parameters.n);
					break;
				}
				else
					cs_ddump(req,4,"PTS Failure for protocol %i, response:",p);
			}
		}

		//FIXME Currently InitICC sets baudrate to 9600 for all T14 cards, which is the old behaviour...
		if (!PPS_success) {//last PPS not succesfull
			BYTE TA1;
			if (ATR_GetInterfaceByte (atr, 1 , ATR_INTERFACE_BYTE_TA, &TA1) == ATR_OK && pps->parameters.t != 14) {
				pps->parameters.FI = TA1 >> 4;
				ATR_GetParameter (atr, ATR_PARAMETER_D, &(pps->parameters.d));
			}
			else { //do not obey TA1 if T14
				pps->parameters.FI = ATR_DEFAULT_FI;
				pps->parameters.d = ATR_DEFAULT_D;
			}
			// Get protocol offered by interface bytes T*2 if TD1 available,
			// FIXME or would it be wiser to not switch anything and stick to 9600? 
			ATR_GetProtocolType (atr, 1, &(pps->parameters.t));
			protocol_selected = 1;

			if (NeedsPTS) { //FIXME we MUST discover PTS routine for Dreambox reader, now all cards in DBreaders which need PTS come here!!!
				if (OffersT[0]) {
	//If the card is able to process more than one protocol type and if one of those protocol types is indicated as T=0, then the protocol type T=0 shall indicated in TD1 as the first offered protocol, and is assumed if no PTS is performed.
					pps->parameters.t =  ATR_PROTOCOL_TYPE_T0;
					pps->parameters.FI = ATR_DEFAULT_FI; //ignoring TA1 is necessary at least for all viaccess cards on dreambox readers
					pps->parameters.d = ATR_DEFAULT_D;   //but perhaps it is the general thing to do...
				}
			}
			/////Here all non-ISO behaviour
			// Nagra HD+ 
			// ATR: 3F FF 95 00 FF 91 81 71 FE 47 00 44 4E 41 53 50 31 34 32 20 52 65 76 47 43 34 63
			// Protocol 1: TA1=95 TB1=00 TC1=FF TD1=91 (T1)
			// Protocol 2: TA2=81 TD2=71 (T1)
			// Protocol 3: TA3=FE TB3=47 TC3=00 no TD3 means T0
			//
			// Problem1: card somehow fails all PTS attempts; perhaps because N=255 and we try PTS without it?
			// Problem2: card reports to support T0 but TA1=FE , which values are (still) undefined . On failure of PTS card reverts to TA=11 which 
			// makes the card fail
			// We know the card needs 115200 (with cardmhz = 368) after ATR
			// Solution: longterm: implement PTS2 
			// Solution: shorterm: this hardcoding
			//
			unsigned char nagra_hdplus[] = {0x44, 0x4E, 0x41, 0x53, 0x50}; //DNASP, not sure about timings of TIGER & others
			if (atr->hbn >= sizeof(nagra_hdplus))
				if (!memcmp(atr->hb, nagra_hdplus, sizeof(nagra_hdplus))) {
					ATR_GetProtocolType (atr, 1, &(pps->parameters.t));
					protocol_selected = 1;
					if (ATR_GetInterfaceByte (atr, 1 , ATR_INTERFACE_BYTE_TA, &TA1) == ATR_OK) {
						pps->parameters.FI = TA1 >> 4;
						ATR_GetParameter (atr, ATR_PARAMETER_D, &(pps->parameters.d));
					}
				}
			//
			// 
			/////End  all non-ISO behaviour
		  if (pps->icc->ifd->io->com == RTYP_SCI) { 
		  //// Here all fixes that are needed until PTS routine for Dreambox is found
			//// End  all fixes that are needed until PTS routine for Dreambox is found
				if (atr->hbn >=6)
					if (!memcmp( atr->hb, "IRDETO", 6 )) { //IRDETO needs TA1 behaviour on /dev/sci  ; which other cards need ignore TA1 on T14?
						pps->parameters.FI = TA1 >> 4;
						ATR_GetParameter (atr, ATR_PARAMETER_D, &(pps->parameters.d));
					}
			}

			cs_debug("No PTS %s, selected protocol 1: T%i, F=%.0f, D=%.6f, N=%.0f\n", NeedsPTS?"happened":"needed", pps->parameters.t, (double) atr_f_table[pps->parameters.FI], pps->parameters.d, pps->parameters.n);
		}
	}//end length<0
		
	//make sure no zero values
	double F =  (double) atr_f_table[pps->parameters.FI];
	if (!F) {
		pps->parameters.FI = ATR_DEFAULT_FI;
		cs_log("Warning: F=0 is invalid, forcing F=%.0f",F);
	}
	if (!pps->parameters.d) {
		pps->parameters.d = ATR_DEFAULT_D;
		cs_log("Warning: D=0 is invalid, forcing D=%.0f",pps->parameters.d);
	}

	pps->icc->protocol_type = pps->parameters.t;
	
#ifdef DEBUG_PROTOCOL
	printf("PPS: T=%i, F=%.0f, D=%.6f, N=%.0f\n", 
	pps->parameters.t, 
	F, 
	pps->parameters.d, 
	pps->parameters.n);
#endif

	ret  = PPS_InitICC(pps, protocol_selected);
			
	if (ret != PPS_OK)
		return ret;
	
	/* Initialize selected protocol with selected parameters */
	ret = PPS_InitProtocol (pps, protocol_selected);
	
	return ret;
}

void * PPS_GetProtocol (PPS * pps)
{
	return pps->protocol;
}

PPS_ProtocolParameters *PPS_GetProtocolParameters (PPS * pps)
{
	/* User must Remember not to reference this struct after removing PPS */
	return &(pps->parameters);
}

void PPS_Delete (PPS * pps)
{
	free (pps);
}

/*
 * Not exported funtions definition
 */

static int PPS_Exchange (PPS * pps, BYTE * params, unsigned *length)
{
	BYTE confirm[PPS_MAX_LENGTH];
	unsigned len_request, len_confirm;
	int ret;
#ifdef DEBUG_PROTOCOL
	int i;
#endif
	
	len_request = PPS_GetLength (params);
	params[len_request - 1] = PPS_GetPCK(params, len_request - 1);
	
#ifdef DEBUG_PROTOCOL
	printf ("PPS: Sending request: ");
	for (i = 0; i < len_request; i++)
		printf ("%X ", params[i]);
	printf ("\n");
#endif

	cs_debug("PTS: Sending request: %s", cs_hexdump(1, params, len_request));
	
	/* Send PPS request */
	if (ICC_Async_Transmit (pps->icc, len_request, params) != ICC_ASYNC_OK)
		return PPS_ICC_ERROR;
	
	/* Get PPS confirm */
	if (ICC_Async_Receive (pps->icc, 2, confirm) != ICC_ASYNC_OK)
		return PPS_ICC_ERROR;
	
	len_confirm = PPS_GetLength (confirm);
	
	if (ICC_Async_Receive (pps->icc, len_confirm - 2, confirm + 2) != ICC_ASYNC_OK)
		return PPS_ICC_ERROR;
	
#ifdef DEBUG_PROTOCOL
	printf ("PPS: Receivig confirm: ");
	for (i = 0; i < len_confirm; i++)
		printf ("%X ", confirm[i]);
	printf ("\n");
#endif
	
	cs_debug("PTS: Receiving confirm: %s", cs_hexdump(1, confirm, len_confirm));
	
	if (!PPS_Match (params, len_request, confirm, len_confirm))
		ret = PPS_HANDSAKE_ERROR;
	else
		ret = PPS_OK;
	
	/* Copy PPS handsake */
	memcpy (params, confirm, len_confirm);
	(*length) = len_confirm;
	
	return ret;
}

static bool PPS_Match (BYTE * request, unsigned len_request, BYTE * confirm, unsigned len_confirm)
{
	/* See if the reply differs from request */
	if ((len_request != len_confirm) || (memcmp (request, confirm, len_request)))
	{
		/* See if the card specifies other than default FI and D */
		//if ((PPS_HAS_PPS1 (confirm)) && (confirm[2] != request[2]))
			return FALSE;
	}
	
	return TRUE;
}

static unsigned PPS_GetLength (BYTE * block)
{
	unsigned length = 3;
	
	if (PPS_HAS_PPS1 (block))
	length++;
	
	if (PPS_HAS_PPS2 (block))
	length++;
	
	if (PPS_HAS_PPS3 (block))
	length++;
	
	return length;
}

static int PPS_InitICC (PPS * pps, int selected_protocol)
{
#ifdef SCI_DEV
#include <sys/ioctl.h>
#include "sci_global.h"
#include "sci_ioctl.h"
	if(pps->icc->ifd->io->com==RTYP_SCI)
	{
		int n;
		SCI_PARAMETERS params;
		int m;
		//memset(&params,0,sizeof(SCI_PARAMETERS));
		if (ioctl(pps->icc->ifd->io->fd, IOCTL_GET_PARAMETERS, &params) < 0 )
			return PPS_ICC_ERROR;

		ATR *atr = ICC_Async_GetAtr (pps->icc);

		params.T = pps->parameters.t;

		BYTE oldFI = params.FI;
//		params.FI = pps->parameters.FI; //somehow setting this gets "card unsupported" 
/*		if (pps->icc->ifd->io->mhz > 368) {
			int number_of_overclock_steps = ((pps->icc->ifd->io->mhz)/200) - 2; //600 is 1 overclock, 800 2 overclocks etc.
			do 
			{
				if (params.FI != 1 && params.FI != 9)
					params.FI --;
				number_of_overclock_steps --;
			} while (number_of_overclock_steps > 0);
		}*/
		if (pps->icc->ifd->io->mhz == 600)
			params.FI = 5; //routine above should be tested so hardcoding can get removed

		if (oldFI != params.FI)
			cs_log("Forcing params.FI from %i to %i", oldFI, params.FI);

		double F =  (double) atr_f_table[pps->parameters.FI];
		params.ETU = F / pps->parameters.d;
		if (pps->parameters.n == 255)
			params.EGT = 0;
		else
			params.EGT = pps->parameters.n;

		double a;
		ATR_GetParameter(atr, ATR_PARAMETER_P, &a);
		params.P=(unsigned char)a;
		ATR_GetParameter(atr, ATR_PARAMETER_I, &a);
		params.I=(unsigned char)a;

		if (ioctl(pps->icc->ifd->io->fd, IOCTL_SET_PARAMETERS, &params)!=0)
			return PPS_ICC_ERROR;
			
		ioctl(pps->icc->ifd->io->fd, IOCTL_GET_PARAMETERS, &params);
			
		cs_debug("T=%d f=%d ETU=%d WWT=%d CWT=%d BWT=%d EGT=%d clock=%d check=%d P=%d I=%d U=%d", (int)params.T,(int)atr_f_table[pps->parameters.FI], (int)params.ETU, (int)params.WWT, (int)params.CWT, (int)params.BWT, (int)params.EGT, (int)params.clock_stop_polarity, (int)params.check, (int)params.P, (int)params.I, (int)params.U);
	}
#endif
	{
	unsigned long baudrate;
	double F =  (double) atr_f_table[pps->parameters.FI];
	if (pps->parameters.t == 14)
		baudrate = 9600;
	else
		baudrate = pps->parameters.d * ICC_Async_GetClockRate (pps->icc) / F; 

#ifdef DEBUG_PROTOCOL
	printf ("PPS: Baudrate = %d\n", (int)baudrate);
#endif
	

	if (ICC_Async_SetBaudrate (pps->icc, baudrate) != ICC_ASYNC_OK)
		return PPS_ICC_ERROR;
	
	return PPS_OK;
	}
}

static int PPS_InitProtocol (PPS * pps, int selected_protocol)
{
	int ret;
	
	if (pps->parameters.t == ATR_PROTOCOL_TYPE_T0)
	{
		pps->protocol = Protocol_T0_New ();
		
		if ((pps->protocol) != NULL)
		{
			ret = Protocol_T0_Init ((Protocol_T0 *) pps->protocol, (ICC_Async *) pps->icc, &(pps->parameters), selected_protocol);
			
			if (ret != PROTOCOL_T0_OK)
			{
				Protocol_T0_Delete ((Protocol_T0 *) pps->protocol);
				pps->protocol = NULL;
				return PPS_PROTOCOL_ERROR;
			}
			
			return PPS_OK;
		}		
	}
	else if (pps->parameters.t == ATR_PROTOCOL_TYPE_T1)
	{
		pps->protocol = Protocol_T1_New ();
		
		if (pps->protocol != NULL)
		{
			ret = Protocol_T1_Init ((Protocol_T1 *) pps->protocol, (ICC_Async *) pps->icc, &(pps->parameters), selected_protocol);
			
			if (ret != PROTOCOL_T1_OK)
			{
				Protocol_T1_Delete ((Protocol_T1 *) pps->protocol);
				pps->protocol = NULL;
				return PPS_PROTOCOL_ERROR;
			}
			
			return PPS_OK;
		}
	}
	else if (pps->parameters.t == ATR_PROTOCOL_TYPE_T14)
	{
		pps->protocol = Protocol_T14_New ();
		
		if ((pps->protocol) != NULL)
		{
			ret = Protocol_T14_Init ((Protocol_T14 *) pps->protocol, (ICC_Async *) pps->icc, &(pps->parameters), selected_protocol);
			
			if (ret != PROTOCOL_T14_OK)
			{
				Protocol_T14_Delete ((Protocol_T14 *) pps->protocol);
				pps->protocol = NULL;
				return PPS_PROTOCOL_ERROR;
			}
			
			return PPS_OK;
		}		
	}
	else
	{
		pps->protocol = NULL;
	}
	
	return PPS_PROTOCOL_ERROR;
}

static BYTE PPS_GetPCK (BYTE * block, unsigned length)
{
	BYTE pck;
	unsigned i;
	
	pck = block[0];
	for (i = 1; i < length; i++)
		pck ^= block[i];
	
	return pck;
}
