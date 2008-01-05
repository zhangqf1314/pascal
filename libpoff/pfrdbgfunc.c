/**********************************************************************
 * pfrdbgfunc.c
 * Read debug function information from a POFF file
 *
 *   Copyright (C) 2008 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <spudmonkey@racsa.co.cr>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 **********************************************************************/

/**********************************************************************
 * Included Files
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "keywords.h"  /* Standard types */
#include "pedefs.h"    /* error code definitions */

#include "perr.h"      /* error() */
#include "pofflib.h"   /* POFF library interface */
#include "pfprivate.h" /* POFF private definitions */

/**********************************************************************
 * Definitions
 **********************************************************************/

/**********************************************************************
 * Global Variables
 **********************************************************************/

/**********************************************************************
 * Private Variables
 **********************************************************************/

/***********************************************************************
 * Private Function Prototypes
 ***********************************************************************/

/***********************************************************************
 * Private Functions
 ***********************************************************************/

/***********************************************************************
 * Public Functions
 ***********************************************************************/

/***********************************************************************/

poffLibDebugFuncInfo_t *poffGetDebugFuncInfo(poffHandle_t handle)
{
  poffInfo_t             *poffInfo = (poffInfo_t*)handle;
  poffDebugFuncInfo_t    *pDebugInfo;
  poffDebugArgInfo_t     *pArgInfo;
  poffLibDebugFuncInfo_t *pRet;
  uint32                  debugFuncIndex;
  int                     i;

  /* Check if there is another debug entgry in the table to be had */

  debugFuncIndex = poffInfo->debugFuncIndex;

  if (debugFuncIndex + sizeof(poffDebugFuncInfo_t) >=
      poffInfo->debugFuncSection.sh_size)
    {
      /* Return NULL to signal the end of the list */

      return NULL;
    }

  /* Get a reference to the debug function entry */

  pDebugInfo = (poffDebugFuncInfo_t*)&poffInfo->debugFuncTable[debugFuncIndex];

  /* Allocate a container */

  pRet = poffCreateDebugInfoContainer(pDebugInfo->df_nparms);

  /* Return the debug function information */

  pRet->value   = pDebugInfo->df_value;
  pRet->retsize = pDebugInfo->df_size;
  pRet->nparms  = pDebugInfo->df_nparms;

  /* Return the size of each parameter */

  debugFuncIndex += sizeof(poffDebugFuncInfo_t);
  for (i = 0; i < pRet->nparms; i++)
    {
      pArgInfo = (poffDebugArgInfo_t*)&poffInfo->debugFuncTable[debugFuncIndex];
      pRet->argsize[i] = pArgInfo->da_size;
      debugFuncIndex += sizeof(poffDebugArgInfo_t);
    }

  /* Set up for the next read */

  poffInfo->debugFuncIndex = debugFuncIndex;
  return pRet;
}

/***********************************************************************/