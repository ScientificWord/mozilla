/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
/* Registry interpreter */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "VerReg.h"
#include "NSReg.h"

extern char *errstr(REGERR err);
extern int DumpTree(void);


int error(char *func, int err)
{
	if (err == REGERR_OK)
	{
    	printf("\t%s -- OK\n", func);
	}
	else
	{
		printf("\t%s -- %s\n", func, errstr(err));
	}

	return err;

}	/* error */

static char  *GetNextWord(char *cmd, char *buf)
{
	/* copies until ',' or eos, then skips spaces */
	if (!cmd || !buf)
		return 0;
	while (*cmd && *cmd != ',')
		*buf++ = *cmd++;
	*buf = '\0';
	if (*cmd == ',')
	{
		cmd++;
		while(*cmd && *cmd == ' ')
			cmd++;
	}
	return cmd;

}	/* GetNextWord */

static int vr_ParseVersion(char *verstr, VERSION *result)
{

	result->major = result->minor = result->release = result->build = 0;
	result->major = atoi(verstr);
	while (*verstr && *verstr != '.')
		verstr++;
	if (*verstr)
	{
		verstr++;
		result->minor = atoi(verstr);
		while (*verstr && *verstr != '.')
			verstr++;
		if (*verstr)
		{
			verstr++;
			result->release = atoi(verstr);
			while (*verstr && *verstr != '.')
				verstr++;
			if (*verstr)
			{
				verstr++;
				result->build = atoi(verstr);
				while (*verstr && *verstr != '.')
					verstr++;
			}
		}
	}

	return REGERR_OK;

}	/* ParseVersion */


void vCreate(char *cmd)
{

	/* Syntax: Create [new,] 5.0b1 */
	char buf[512];
    
	int flag = 0;
	cmd = GetNextWord(cmd, buf);
    
	error("VR_CreateRegistry", VR_CreateRegistry("Communicator", buf, cmd));

}	/* vCreate */



void vFind(char *cmd)
{

	VERSION ver;
	char path[MAXREGPATHLEN];

	if (error("VR_GetVersion", VR_GetVersion(cmd, &ver)) == REGERR_OK)
	{
		if (error("VR_GetPath", VR_GetPath(cmd, sizeof(path), path)) == REGERR_OK)
		{
			printf("%s found: ver=%d.%d.%d.%d, check=0x%04x, path=%s\n", 
				cmd, ver.major, ver.minor, ver.release, ver.build, ver.check,
				path);
			return;
		}
	}

	printf("%s not found.\n", cmd);
	return;

}	/* vFind */


void vHelp(char *cmd)
{

	puts("Enter a command:");
    puts("\tN)ew <dir> [, <ver>] - create a new registry");
    puts("\tA)pp <dir>       - set application directory");
    puts("\tC)lose           - close the registry");
    puts("");
	puts("\tI)nstall <name>, <version>, <path> - install a new component");
	puts("\tR)emove <name>   - deletes a component from the Registry");
    puts("\tX)ists <name>    - checks for existence in registry");
    puts("\tT)est <name>     - validates physical existence");
    puts("\tE)num <name>     - dumps named subtree");
    puts("");
    puts("\tV)ersion <name>  - gets component version");
    puts("\tP)ath <name>     - gets component path");
    puts("\treF)count <name> - gets component refcount");
    puts("\tD)ir <name>      - gets component directory");
    puts("\tSR)efcount <name>- sets component refcount");
    puts("\tSD)ir <name>     - sets component directory");
    puts("");
	puts("\tQ)uit            - end the program");

}	/* vHelp */


void vInstall(char *cmd)
{

	char name[MAXREGPATHLEN+1];
	char path[MAXREGPATHLEN+1];
    char ver[MAXREGPATHLEN+1];

    char *pPath, *pVer;

	cmd = GetNextWord(cmd, name);
    cmd = GetNextWord(cmd, ver);
    cmd = GetNextWord(cmd, path);

    pVer  = ( ver[0]  != '*' ) ? ver  : NULL;
    pPath = ( path[0] != '*' ) ? path : NULL;

	error("VR_Install", VR_Install(name, pPath, pVer, FALSE));

}	/* vInstall */




			

void interp(void)
{

	char line[256];
	char *p;

	while(1)
	{
		putchar('>');
		putchar(' ');
		fflush(stdin); fflush(stdout); fflush(stderr);
		gets(line);

		/* p points to next word after verb on command line */
		p = line;
		while (*p && *p!=' ')
			p++;
		if (!*p)
			p = 0;
		else
		{
			while(*p && *p==' ')
				p++;
		}

		switch(toupper(line[0]))
		{
		case 'N':
			vCreate(p);
			break;
        case 'A':
            error("VR_SetRegDirectory", VR_SetRegDirectory(p));
            break;
        case 'C':
            error("VR_Close", VR_Close());
            break;

		case 'I':
			vInstall(p);
			break;
		case 'R':
        	error("VR_Remove", VR_Remove(p));
			break;
        case 'X':
        	error("VR_InRegistry", VR_InRegistry(p));
            break;
        case 'T':
        	error("VR_ValidateComponent", VR_ValidateComponent(p));
            break;

#if  LATER
        case 'E':
            vEnum(p);
            break;

        case 'V':
            vVersion(p);
            break;
        case 'P':
            vPath(p);
            break;
        case 'F':
            vGetRefCount(p);
            break;
        case 'D':
            vGetDir(p);
            break;
        
        case 'S':
            puts("--Unsupported--");
#endif

		case 'H':
		default:
			vHelp(line);
			break;
		case 'Q':
			return;
		}	/* switch */
	}	/* while */

	assert(0);
	return;	/* shouldn't get here */

}	/* interp */

/* EOF: interp.c */
