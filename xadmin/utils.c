/* 
** NDRX helper functions.
**
** @file utils.c
** 
** -----------------------------------------------------------------------------
** Enduro/X Middleware Platform for Distributed Transaction Processing
** Copyright (C) 2015, ATR Baltic, SIA. All Rights Reserved.
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <ctype.h>

#include <ndrstandard.h>
#include <ndebug.h>

#include <ndrx.h>
#include <ndrxdcmn.h>
#include <atmi_int.h>
#include <nclopt.h>
#include <pscript.h>
#include <errno.h>
#include <userlog.h>

/*---------------------------Externs------------------------------------*/
extern const char G_resource_Exfields[];
/*---------------------------Macros-------------------------------------*/
/*---------------------------Enums--------------------------------------*/
/*---------------------------Typedefs-----------------------------------*/
/*---------------------------Globals------------------------------------*/
/*---------------------------Statics------------------------------------*/
/*---------------------------Prototypes---------------------------------*/

/**
 * If confirmation is required for command, then check it.
 * @param message
 * @param argc
 * @param argv
 * @return TRUE/FALSE
 */
public int chk_confirm(char *message, short is_confirmed)
{
    int ret=FALSE;
    char buffer[128];
    int ans_ok = FALSE;
    
    if (!is_confirmed)
    {
        if (isatty(0))
        {
            do
            {
                /* Ask Are you sure */
                fprintf(stderr, "%s [Y/N]: ", message);
                while (NULL==fgets(buffer, sizeof(buffer), stdin))
                {
                    /* do nothing */
                }

                if (toupper(buffer[0])=='Y' && '\n'==buffer[1] && EOS==buffer[2])
                {
                    ret=TRUE;
                    ans_ok=TRUE;
                }
                else if (toupper(buffer[0])=='N' && '\n'==buffer[1] && EOS==buffer[2])
                {
                    ret=FALSE;
                    ans_ok=TRUE;
                }

            } while (!ans_ok);
        }
        else
        {
            NDRX_LOG(log_warn, "Not tty, assuming no for: %s", message);
        }
    }
    else
    {
        ret=TRUE;
    }
    
    return ret;
}

/**
 * Quick wrapper for argument parsing...
 * @param message
 * @param artc
 * @param argv
 * @return 
 */
public int chk_confirm_clopt(char *message, int argc, char **argv)
{
    int ret = SUCCEED;
    short confirm = FALSE;
    
    ncloptmap_t clopt[] =
    {
        {'y', BFLD_SHORT, (void *)&confirm, 0, 
                                NCLOPT_OPT | NCLOPT_TRUEBOOL, "Confirm"},
        {0}
    };
    
    /* parse command line */
    if (nstd_parse_clopt(clopt, TRUE,  argc, argv, FALSE))
    {
        fprintf(stderr, XADMIN_INVALID_OPTIONS_MSG);
        FAIL_OUT(ret);
    }
    
    
    ret = chk_confirm(message, confirm);
    
out:
    return ret;
}

#ifndef NDRX_DISABLEPSCRIPT
/**
 * Print function for pscript
 * @param v
 * @param s
 * @param ...
 */
public void printfunc(HPSCRIPTVM v,const PSChar *s,...)
{
    va_list vl;
    va_start(vl, s);
    vfprintf(stdout, s, vl);
    va_end(vl);
}


/**
 * Error function for pscript
 * @param v
 * @param s
 * @param ...
 */
public void errorfunc(HPSCRIPTVM v,const PSChar *s,...)
{
    va_list vl;
    va_start(vl, s);
    vfprintf(stderr, s, vl);
    va_end(vl);
}


/**
 * Load key:value string into VM table
 * @param v
 * @param key_val_string
 * @return 
 */
public int load_value(HPSCRIPTVM v, char *key_val_string)
{
    int ret = SUCCEED;
    char *p;
    int len = strlen(key_val_string);
    
    p=strchr(key_val_string, '=');
    
    if (!len)
    {
        fprintf(stderr, "Empty value string!\n");
        FAIL_OUT(ret);
    }
    
    if (NULL==p)
    {
        fprintf(stderr, "Missing '=' in value [%s]!\n", key_val_string);
        FAIL_OUT(ret);
    }
    
    if (p==key_val_string)
    {
        fprintf(stderr, "Missing value [%s]!\n", key_val_string);
        FAIL_OUT(ret);
    }
    
    if (p==key_val_string)
    {
        fprintf(stderr, "Missing value [%s]!\n", key_val_string);
        FAIL_OUT(ret);
    }
    
    *p = EOS;
    p++;
    
    /* fprintf(stderr, "Setting value: %s\n", p); */
    ps_pushstring(v, key_val_string, -1); /* 4 */
    ps_pushstring(v, p, -1);/* 5 */
    ps_newslot(v, -3, PSFalse );/* 3 */
    
out:
    return ret;
}


/**
 * Add defaults from config file
 * @return 
 */
public int add_defaults_from_config(HPSCRIPTVM v, char *section)
{
    int ret = SUCCEED;
    ndrx_inicfg_section_keyval_t *val;
    char *ptr = NULL;
    char *p;
    /* get the value if have one */
    if (NULL!=(val=ndrx_keyval_hash_get(G_xadmin_config, section)))
    {
        userlog("Got config defaults for %s: [%s]", section, val->val);
        
        if (NULL==(ptr = strdup(val->val)))
        {
            userlog("Malloc failed: %s", strerror(errno));
            FAIL_OUT(ret);
        }
        
        /* OK token the string and do override */
        
        /* Now split the stuff */
        p = strtok (ptr, ARG_DEILIM);
        while (p != NULL)
        {
            if (0==strcmp(p, "-d"))
            {
                PSBool isDefaulted = TRUE;
                ps_pushstring(v, "isDefaulted", -1); /* 4 */
                ps_pushbool(v, isDefaulted);
                ps_newslot(v, -3, PSFalse );/* 3 */
            }
            else if (0==strncmp(p, "-v", 2) 
                    && 0!=strcmp(p, "-v"))
            {
                if (strlen(p) < 4)
                {
                    userlog("Invalid default settings for provision command [%s]", 
                            val->val);
                    FAIL_OUT(ret);
                }
                
                /* pass in value definition (as string) 
                 * format <key>=<value>
                 */
                if (SUCCEED!=load_value(v, p+2))
                {
                    userlog("Invalid value\n");
                    FAIL_OUT(ret);
                }
            }
            else if (0==strncmp(p, "-v", 2))
            {
                p = strtok (NULL, ARG_DEILIM);
                
                /* next value is key */
                if (NULL!=p)
                {
                    if (SUCCEED!=load_value(v, p))
                    {
                        userlog("Invalid value on provision defaults [%s]", 
                                val->val);
                        FAIL_OUT(ret);
                    }
                }
                else
                {
                    userlog("Invalid command line missing value at end [%s]", 
                            val->val);
                    FAIL_OUT(ret);
                }
            }
            
            p = strtok (NULL, ARG_DEILIM);
        }
    }
    
out:

    if (NULL!=ptr)
    {
        free(ptr);
    }

    if (SUCCEED!=ret)
    {
        fprintf(stderr, "Failed to process defaults - invalid config [%s], see ULOG\n", 
                G_xadmin_config_file);
    }

    return ret;
}


/**
 *Read the line from terminal
 *@return line read string
 */
static PSInteger _xadmin_getExfields(HPSCRIPTVM v)
{
    
    ps_pushstring(v,G_resource_Exfields,-1);

    return 1;
}

/**
 * Provide the Exfields function to root table.
 */
public int register_getExfields(HPSCRIPTVM v)
{
    
    ps_pushstring(v,"getExfields",-1);
    ps_newclosure(v,_xadmin_getExfields,0);
    ps_setparamscheck(v,1,".s");
    ps_setnativeclosurename(v,-1,"getExfields");
    ps_newslot(v,-3,PSFalse);

    return 1;
}

#endif
