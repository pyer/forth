/** 
 * -- Version File
 *
 *  Copyright (C) Tektronix, Inc. 1998 - 2003.
 *  Copyright (C) 2005 - 2008 Guido U. Draheim <guidod@gmx.de>
 *  Copyright (C) Pierre Bazonnard 2013 - 2014.
 *
 *  @see     GNU LGPL
 *  @author  Guido U. Draheim            (modified by $Author: guidod $)
 *  @version $Revision: 1.5 $
 *     (modified $Date: 2008-04-20 04:51:55 $)
 *
 *  @description
 *      there are some internal functions in the version-file 
 *      that are used to print out compile-time, -date, -version 
 *      and the license of course.
 */

#include "config.h"
#include "types.h"
#include "listwords.h"
#include "terminal.h"

#define MAKETIME __TIME__
#define MAKEDATE __DATE__

const char* pf_version_string(void)
{
    return
	"\nPierre's Forth "PF_VERSION
        "\n(" MAKEDATE " " MAKETIME ")\n";
}

const char* pf_copyright_string(void)
{
    return
	"\nCopyright (C) Dirk Uwe Zoller  1993 - 1995."
	"\nCopyright (C) Tektronix, Inc.  1998 - 2003."
        "\nCopyright (C) Guido U. Draheim 2005 - 2008."
        "\nCopyright (C) Pierre Bazonnard 2013 - 2014.\n";
}

const char* pf_license_string (void)
{
    return
	"\nThis program is free software; you can redistribute it and/or"
	"\nmodify it under the terms of the GNU Library General Public"
	"\nLicense as published by the Free Software Foundation; either"
	"\nversion 2 of the License, or (at your option) any later version.\n";
}

const char* pf_warranty_string (void)
{
    return
	"\nThis program is distributed in the hope that it will be useful,"
	"\nbut WITHOUT ANY WARRANTY; without even the implied warranty of"
	"\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU"
	"\nLibrary General Public License for more details."
	"\n"
	"\nYou should have received a copy of the GNU Library General Public"
	"\nLicense along with this program; if not, write to the Free Software"
	"\nFoundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n";
}

/************************************************************************/

/** .VERSION ( -- )
 * show the version of the current PFE system
 : .VERSION [ ENVIRONMENT ] FORTH-NAME TYPE FORTH-VERSION TYPE ;
 */
FCode (pf_version)
{
    pf_outs (pf_version_string ());
}

/** .COPYRIGHT ( -- )
 */
FCode (pf_copyright)
{
    pf_outs (pf_copyright_string ());
}

/** LICENSE ( -- )
 * show a lisence info - the basic PFE system is licensed under the terms
 * of the LGPL (Lesser GNU Public License) - binary modules loaded into
 * the system and hooking into the system may carry another => LICENSE
 : LICENSE [ ENVIRONMENT ] FORTH-LICENSE TYPE ;
 */
FCode (pf_license)
{
    pf_outs (pf_license_string ());
}

/** WARRANTY ( -- )
 * show a warranty info - the basic PFE system is licensed under the terms
 * of the LGPL (Lesser GNU Public License) - which exludes almost any 
 * liabilities whatsoever - however loadable binary modules may hook into
 * the system and their functionality may have different WARRANTY infos.
 */
FCode (pf_warranty)
{
    pf_outs (pf_warranty_string ());
}

/************************************************************************/

P4_LISTWORDS (version) =
{
//    P4_INTO ("FORTH", 0),
    
    /* forth distributor info */
    P4_FXco (".VERSION",	pf_version),
    P4_FXco (".COPYRIGHT",	pf_copyright),
    P4_FXco (".LICENSE",	pf_license),
    P4_FXco (".WARRANTY",	pf_warranty),

};
P4_COUNTWORDS (version, "Version, copyright, license and warranty words");

