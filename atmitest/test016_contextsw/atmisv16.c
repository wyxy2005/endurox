/* 
** Test the full Enduro/X multi-contexting. 
**
** @file atmisv16.c
** 
** -----------------------------------------------------------------------------
** Enduro/X Middleware Platform for Distributed Transaction Processing
** Copyright (C) 2016, ATR Baltic, SIA. All Rights Reserved.
** This software is released under one of the following licenses:
** GPL or ATR Baltic's license for commercial use.
** -----------------------------------------------------------------------------
** GPL license:
** 
** This program is free software; you can redistribute it and/or modify it under
** the terms of the GNU General Public License as published by the Free Software
** Foundation; either version 2 of the License, or (at your option) any later
** version.
**
** This program is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
** PARTICULAR PURPOSE. See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software Foundation, Inc., 59 Temple
** Place, Suite 330, Boston, MA 02111-1307 USA
**
** -----------------------------------------------------------------------------
** A commercial use license is available from ATR Baltic, SIA
** contact@atrbaltic.com
** -----------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <ndebug.h>
#include <unistd.h>
#include <atmi.h>
#include <ndrstandard.h>
#include <ubf.h>
#include <test.fd.h>
#include <string.h>

/* store fist two callers */
CLIENTID M_cltid[2] = {EOS, EOS};

/**
 * This will check the client id for first & second call,
 * - odd calls must match with odd client id
 * - even calls must match the even client id
 */
void TESTSVFN (TPSVCINFO *p_svc)
{
    int ret=SUCCEED;
    UBFH *p_ub = (UBFH *)p_svc->data;
    static int call_num = 0;
    
    NDRX_LOG(log_debug, "TESTSVFN got call from client [%s]", 
            p_svc->cltid.clientdata);
    
    if (call_num < 2)
    {
        strcpy(M_cltid[call_num].clientdata, p_svc->cltid.clientdata);
    }
    else
    {
        NDRX_LOG(log_debug, "call_num = %d, mod %d, mods [%s] vs [%s]", 
                call_num, call_num %2, M_cltid[call_num %2].clientdata,
                p_svc->cltid.clientdata);
        
        if (0!=strcmp(M_cltid[call_num%2].clientdata, p_svc->cltid.clientdata))
        {
            NDRX_LOG(log_error, "TESTERROR: call_num = %d, mod %d, "
                    "mods [%s] vs [%s] - not equal!", 
                    call_num, call_num %2, M_cltid[call_num %2].clientdata,
                    p_svc->cltid.clientdata);
            
            FAIL_OUT(ret);
        }
    }
    
out:
    call_num++;
    tpreturn(  ret==SUCCEED?TPSUCCESS:TPFAIL,
                0L,
                (char *)p_ub,
                0L,
                0L);
}

/*
 * Do initialization
 */
int NDRX_INTEGRA(tpsvrinit)(int argc, char **argv)
{
    int ret = SUCCEED;
    NDRX_LOG(log_debug, "tpsvrinit called");

    if (SUCCEED!=tpadvertise("TESTSV", TESTSVFN))
    {
        NDRX_LOG(log_error, "Failed to initialize TESTSV (first)!");
        ret=FAIL;
    }
    
    return ret;
}

/**
 * Do de-initialization
 */
void NDRX_INTEGRA(tpsvrdone)(void)
{
    NDRX_LOG(log_debug, "tpsvrdone called");
}
