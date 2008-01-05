/**********************************************************************
 * pfwrite.c
 * Write a POFF file
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

static uint16 poffCountSections(poffHandle_t handle);
static void   poffWriteFileHeader(poffHandle_t handle, FILE *poffFile);
static void   poffWriteSectionHeaders(poffHandle_t handle, FILE *poffFile);
static void   poffWriteSectionData(poffHandle_t handle, FILE *poffFile);

/***********************************************************************
 * Private Functions
 ***********************************************************************/

static uint16 poffCountSections(poffHandle_t handle)
{
  poffInfo_t *poffInfo = (poffInfo_t*)handle;
  uint16 nSections     = 1; /* We always write the string table */

  /* Count each other section that has stored data */

  if (HAVE_PROGRAM_SECTION)
    nSections++;

  if (HAVE_RODATA_SECTION)
    nSections++;

  if (HAVE_SYMBOL_TABLE)
    nSections++;

  if (HAVE_RELOC_SECTION)
    nSections++;

  if (HAVE_FILE_TABLE)
    nSections++;

  if (HAVE_LINE_NUMBER)
    nSections++;

  if (HAVE_DEBUG_SECTION)
    nSections++;

  return nSections;
}

/***********************************************************************/

static void poffWriteFileHeader(poffHandle_t handle, FILE *poffFile)
{
  poffInfo_t *poffInfo = (poffInfo_t*)handle;
  size_t entriesWritten;

  /* Get the number of section structures following the file header */

  poffInfo->fileHeader.fh_shnum = poffCountSections(handle);

  /* Write the POFF file header */

  entriesWritten = fwrite(&poffInfo->fileHeader, sizeof(poffFileHeader_t),
			  1, poffFile);
  if (entriesWritten != 1)
    {
      errmsg("Failed to write POFF header: %s\n", strerror(errno));
      fatal(ePOFFWRITEERROR);
    }
}

/***********************************************************************/

static void poffWriteSectionHeaders(poffHandle_t handle, FILE *poffFile)
{
  poffInfo_t *poffInfo = (poffInfo_t*)handle;

  /* The data starts immediately after the file header and the array
   * of section headers.
   */

  uint32 dataOffset = poffInfo->fileHeader.fh_shoff +
    poffInfo->fileHeader.fh_shnum * poffInfo->fileHeader.fh_shsize;
  size_t entriesWritten;

  /* Write the program section header (if we have one) */

  if (HAVE_PROGRAM_SECTION)
    {
      /* Add the name of the section to the string table */

      poffInfo->progSection.sh_name    = poffAddString(handle, ".text");
      poffInfo->progSection.sh_offset  = dataOffset;
      dataOffset                       += poffInfo->progSection.sh_size;

      /* Then write the section header to the output file */

      entriesWritten = fwrite(&poffInfo->progSection,
			      sizeof(poffSectionHeader_t),
			      1, poffFile);
      if (entriesWritten != 1)
	{
	  errmsg("Failed to write program section header: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the initialized read-only data section header  (if we have one) */

  if (HAVE_RODATA_SECTION)
    {
      /* Add the name of the section to the string table */

      poffInfo->roDataSection.sh_name    = poffAddString(handle, ".rodata");
      poffInfo->roDataSection.sh_offset  = dataOffset;
      dataOffset                        += poffInfo->roDataSection.sh_size;

      /* Then write the section header to the output file */

      entriesWritten = fwrite(&poffInfo->roDataSection,
			      sizeof(poffSectionHeader_t),
			      1, poffFile);
      if (entriesWritten != 1)
	{
	  errmsg("Failed to write data section header: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the symbol table section header  (if we have one) */

  if (HAVE_SYMBOL_TABLE)
    {
      /* Add the name of the section to the string table */

      poffInfo->symbolTableSection.sh_name    = poffAddString(handle, ".symtab");
      poffInfo->symbolTableSection.sh_offset  = dataOffset;
      dataOffset                             += poffInfo->symbolTableSection.sh_size;

      /* Then write the section header to the output file */

      entriesWritten = fwrite(&poffInfo->symbolTableSection,
			      sizeof(poffSectionHeader_t),
			      1, poffFile);
      if (entriesWritten != 1)
	{
	  errmsg("Failed to write symbol table section header: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the relocation table section header  (if we have one) */

  if (HAVE_RELOC_SECTION)
    {
      /* Add the name of the section to the string table */

      poffInfo->relocSection.sh_name    = poffAddString(handle, ".rel");
      poffInfo->relocSection.sh_offset  = dataOffset;
      dataOffset                       += poffInfo->relocSection.sh_size;

      /* Then write the section header to the output file */

      entriesWritten = fwrite(&poffInfo->relocSection,
			      sizeof(poffSectionHeader_t),
			      1, poffFile);
      if (entriesWritten != 1)
	{
	  errmsg("Failed to write relocation section header: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the file table section header (if we have one) */

  if (HAVE_FILE_TABLE)
    {
      /* Add the name of the section to the string table */

      poffInfo->fileNameTableSection.sh_name    = poffAddString(handle, ".filetab");
      poffInfo->fileNameTableSection.sh_offset  = dataOffset;
      dataOffset                                += poffInfo->fileNameTableSection.sh_size;

      /* Then write the section header to the output file */

      entriesWritten = fwrite(&poffInfo->fileNameTableSection,
			      sizeof(poffSectionHeader_t),
			      1, poffFile);
      if (entriesWritten != 1)
	{
	  errmsg("Failed to write file table section header: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the line number section header (if we have one) */

  if (HAVE_LINE_NUMBER)
    {
      /* Add the name of the section to the string table */

      poffInfo->lineNumberSection.sh_name    = poffAddString(handle, ".lineno");
      poffInfo->lineNumberSection.sh_offset  = dataOffset;
      dataOffset                            += poffInfo->lineNumberSection.sh_size;

      /* Then write the section header to the output file */

      entriesWritten = fwrite(&poffInfo->lineNumberSection,
			      sizeof(poffSectionHeader_t),
			      1, poffFile);
      if (entriesWritten != 1)
	{
	  errmsg("Failed to write line number section header: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the debug function info section header (if we have one) */

  if (HAVE_DEBUG_SECTION)
    {
      /* Add the name of the section to the string table */

      poffInfo->debugFuncSection.sh_name    = poffAddString(handle, ".dbgfunc");
      poffInfo->debugFuncSection.sh_offset  = dataOffset;
      dataOffset                            += poffInfo->debugFuncSection.sh_size;

      /* Then write the section header to the output file */

      entriesWritten = fwrite(&poffInfo->debugFuncSection,
			      sizeof(poffSectionHeader_t),
			      1, poffFile);
      if (entriesWritten != 1)
	{
	  errmsg("Failed to write debug section header: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the string table section header LAST (because we may have
   * added strings with the above logic.
   */

  /* Add the name of the section to the string table */

  poffInfo->stringTableSection.sh_name    = poffAddString(handle, ".strtab");
  poffInfo->stringTableSection.sh_offset  = dataOffset;
  dataOffset                             += poffInfo->stringTableSection.sh_size;

  /* Then write the section header to the output file */

  entriesWritten = fwrite(&poffInfo->stringTableSection,
			  sizeof(poffSectionHeader_t),
			  1, poffFile);
  if (entriesWritten != 1)
    {
      errmsg("Failed to write string table section header: %s\n",
	     strerror(errno));
      fatal(ePOFFWRITEERROR);
    }
}

/***********************************************************************/

static void poffWriteSectionData(poffHandle_t handle, FILE *poffFile)
{
  poffInfo_t *poffInfo = (poffInfo_t*)handle;
  size_t      entriesWritten;

  /* Write the program section data (if we have one) */

  if (HAVE_PROGRAM_SECTION)
    {
      if (!poffInfo->progSectionData) fatal(ePOFFCONFUSION);

      entriesWritten = fwrite(poffInfo->progSectionData, sizeof(ubyte),
			      poffInfo->progSection.sh_size, poffFile);
      if (entriesWritten != poffInfo->progSection.sh_size)
	{
	  errmsg("Failed to write program data: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the read-only data section data  (if we have one) */

  if (HAVE_RODATA_SECTION)
    {
      if (!poffInfo->roDataSectionData) fatal(ePOFFCONFUSION);

      entriesWritten = fwrite(poffInfo->roDataSectionData, sizeof(ubyte),
			      poffInfo->roDataSection.sh_size, poffFile);
      if (entriesWritten != poffInfo->roDataSection.sh_size)
	{
	  errmsg("Failed to write initialized data: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the symbol table section data  (if we have one) */

  if (HAVE_SYMBOL_TABLE)
    {
      if (!poffInfo->symbolTable) fatal(ePOFFCONFUSION);

      entriesWritten = fwrite(poffInfo->symbolTable, sizeof(ubyte),
			      poffInfo->symbolTableSection.sh_size, poffFile);
      if (entriesWritten != poffInfo->symbolTableSection.sh_size)
	{
	  errmsg("Failed to write symbol table data: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the relocation table section data  (if we have one) */

  if (HAVE_RELOC_SECTION)
    {
      if (!poffInfo->relocTable) fatal(ePOFFCONFUSION);

      entriesWritten = fwrite(poffInfo->relocTable, sizeof(ubyte),
			      poffInfo->relocSection.sh_size, poffFile);
      if (entriesWritten != poffInfo->relocSection.sh_size)
	{
	  errmsg("Failed to write relocation data: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the file table section data (if we have one) */

  if (HAVE_FILE_TABLE)
    {
      if (!poffInfo->fileNameTable) fatal(ePOFFCONFUSION);

      entriesWritten = fwrite(poffInfo->fileNameTable, sizeof(ubyte),
			      poffInfo->fileNameTableSection.sh_size,
			      poffFile);
      if (entriesWritten != poffInfo->fileNameTableSection.sh_size)
	{
	  errmsg("Failed to write filename table data: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the line number section data (if we have one) */

  if (HAVE_LINE_NUMBER)
    {
      if (!poffInfo->lineNumberTable) fatal(ePOFFCONFUSION);

      entriesWritten = fwrite(poffInfo->lineNumberTable, sizeof(ubyte),
			      poffInfo->lineNumberSection.sh_size,
			      poffFile);
      if (entriesWritten != poffInfo->lineNumberSection.sh_size)
	{
	  errmsg("Failed to write line number table data: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the line number section data (if we have one) */

  if (HAVE_DEBUG_SECTION)
    {
      if (!poffInfo->debugFuncTable) fatal(ePOFFCONFUSION);

      entriesWritten = fwrite(poffInfo->debugFuncTable, sizeof(ubyte),
			      poffInfo->debugFuncSection.sh_size,
			      poffFile);
      if (entriesWritten != poffInfo->debugFuncSection.sh_size)
	{
	  errmsg("Failed to write debug table data: %s\n",
		 strerror(errno));
	  fatal(ePOFFWRITEERROR);
	}
    }

  /* Write the string table section data LAST (because we may have
   * added strings with the above logic.
   */

  if (!poffInfo->stringTable) fatal(ePOFFCONFUSION);

  entriesWritten = fwrite(poffInfo->stringTable, sizeof(ubyte),
			  poffInfo->stringTableSection.sh_size, poffFile);
  if (entriesWritten != poffInfo->stringTableSection.sh_size)
    {
      errmsg("Failed to write string table data: %s\n",
	     strerror(errno));
      fatal(ePOFFWRITEERROR);
    }
}

/***********************************************************************
 * Public Functions
 ***********************************************************************/

void poffWriteFile(poffHandle_t handle, FILE *poffFile)
{
  poffWriteFileHeader(handle, poffFile);
  poffWriteSectionHeaders(handle, poffFile);
  poffWriteSectionData(handle, poffFile);
}

/***********************************************************************/