/*
 * $Id: gifdecomp.c,v 1.1 2009/12/19 19:42:49 rhabarber1848 Exp $
 *
 * tuxwetter - d-box2 linux project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
*/

#include <config.h>
#define HAVE_VARARGS_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "gif_lib.h"
#include "gifdecomp.h"

#define PROGRAM_NAME	"GifDecomp"
#define GIF_ASM_NAME   "Tuxwetter"
#define COMMENT_GIF_ASM    "New-Tuxwetter-Team"
#define SQR(x)     ((x) * (x))
#ifdef MARTII
# ifndef TRUE
#  define TRUE -1
# endif
# ifndef FALSE
#  define FALSE 0
# endif
#endif

static int
   ImageNum = 0,
    InterlacedFlag = FALSE,
    InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */
    InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */

int LoadImage(GifFileType *GifFile, GifRowType **ImageBuffer);
int DumpImage(GifFileType *GifFile, GifRowType *ImageBuffer);


void xremove(char *fname);

/******************************************************************************
* Perform the disassembly operation - take one input files into few output.   *
******************************************************************************/
int gifdecomp(char *InFileName, char *OutFileName)
{
int i;

    GifRowType *ImageBuffer;
    char TempGifName[80];
    sprintf (TempGifName,"/tmp/tempgif.gif");
    int	ExtCode, CodeSize, FileNum = 0, FileEmpty;
    GifRecordType RecordType;
    char CrntFileName[80];
    char tempout[80];
    GifByteType *Extension, *CodeBlock;
    GifFileType *GifFileIn = NULL, *GifFileOut = NULL;
    for(i=0; i<32; i++)
    {
    	sprintf(tempout,"%s%02d.gif",OutFileName,i);
    	xremove(tempout);
    }
    xremove(TempGifName);
    /* Open input file: */
    if (InFileName != NULL) 
    {	
#ifdef ENABLE_GIFLIB
	if ((GifFileIn = DGifOpenFileName(InFileName, NULL)) == NULL)
#else
	if ((GifFileIn = DGifOpenFileName(InFileName)) == NULL)
#endif
		QuitGifError(GifFileIn, GifFileOut);
    }
#ifdef ENABLE_GIFLIB
    if ((GifFileIn = DGifOpenFileName(InFileName, NULL)) != NULL)
#else
    if ((GifFileIn = DGifOpenFileName(InFileName)) != NULL)
#endif
    {
#ifdef ENABLE_GIFLIB
		if ((GifFileOut = EGifOpenFileName(TempGifName, TRUE, NULL)) == NULL)
#else
		if ((GifFileOut = EGifOpenFileName(TempGifName, TRUE)) == NULL)
#endif
		QuitGifError(GifFileIn, GifFileOut);
   
    
    		if (EGifPutScreenDesc(GifFileOut,
		GifFileIn->SWidth, GifFileIn->SHeight,
		GifFileIn->SColorResolution, GifFileIn->SBackGroundColor,
		GifFileIn->SColorMap) == GIF_ERROR)
		QuitGifError(GifFileIn, GifFileOut);

    		/* Scan the content of the GIF file and load the image(s) in: */
    		do {
		if (DGifGetRecordType(GifFileIn, &RecordType) == GIF_ERROR)
	    	QuitGifError(GifFileIn, GifFileOut);

		switch (RecordType) {
	    	case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(GifFileIn) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);

			/* Put the image descriptor to out file: */
			if (EGifPutImageDesc(GifFileOut,
			    GifFileIn->Image.Left, GifFileIn->Image.Top,
			    GifFileIn->Image.Width, GifFileIn->Image.Height,
			    InterlacedFlag,
			    GifFileIn->Image.ColorMap) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);

			/* Load the image (either Interlaced or not), and dump it as */
			/* defined in GifFileOut->Image.Interlaced.		     */
			if (LoadImage(GifFileIn, &ImageBuffer) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);
			if (DumpImage(GifFileOut, ImageBuffer) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);
			break;
		    case EXTENSION_RECORD_TYPE:
			/* Skip any extension blocks in file: */
			if (DGifGetExtension(GifFileIn, &ExtCode, &Extension) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);
			if (EGifPutExtension(GifFileOut, ExtCode, Extension[0],
							Extension) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);

			/* No support to more than one extension blocks, so discard: */
			while (Extension != NULL) {
			    if (DGifGetExtensionNext(GifFileIn, &Extension) == GIF_ERROR)
				QuitGifError(GifFileIn, GifFileOut);
			}
			break;
		    case TERMINATE_RECORD_TYPE:
			break;
		    default:		    /* Should be traps by DGifGetRecordType. */
			break;
		}
   	 }
  	  while (RecordType != TERMINATE_RECORD_TYPE);

  	  if (DGifCloseFile(GifFileIn) == GIF_ERROR)
		QuitGifError(GifFileIn, GifFileOut);
  	  if (EGifCloseFile(GifFileOut) == GIF_ERROR)
		QuitGifError(GifFileIn, GifFileOut);
               
#ifdef ENABLE_GIFLIB
	if ((GifFileIn = DGifOpenFileName(TempGifName, NULL)) == NULL)
#else
	if ((GifFileIn = DGifOpenFileName(TempGifName)) == NULL)
#endif
	QuitGifError(GifFileIn, GifFileOut);

    
   		 /* Scan the content of GIF file and dump image(s) to seperate file(s): */
    		do {
		sprintf(CrntFileName, "%s%02d.gif", OutFileName, FileNum++);
#ifdef ENABLE_GIFLIB
		if ((GifFileOut = EGifOpenFileName(CrntFileName, TRUE, NULL)) == NULL)
#else
		if ((GifFileOut = EGifOpenFileName(CrntFileName, TRUE)) == NULL)
#endif
		    QuitGifError(GifFileIn, GifFileOut);
		FileEmpty = TRUE;

		/* And dump out its exactly same screen information: */
		if (EGifPutScreenDesc(GifFileOut,
		    GifFileIn->SWidth, GifFileIn->SHeight,
		    GifFileIn->SColorResolution, GifFileIn->SBackGroundColor,
		    GifFileIn->SColorMap) == GIF_ERROR)
		    QuitGifError(GifFileIn, GifFileOut);

		do {
			if (DGifGetRecordType(GifFileIn, &RecordType) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);

	   		switch (RecordType) {
			case IMAGE_DESC_RECORD_TYPE:
			FileEmpty = FALSE;
			if (DGifGetImageDesc(GifFileIn) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);
		 	   /* Put same image descriptor to out file: */
			if (EGifPutImageDesc(GifFileOut,
			    GifFileIn->Image.Left, GifFileIn->Image.Top,
			    GifFileIn->Image.Width, GifFileIn->Image.Height,
			    GifFileIn->Image.Interlace,
			    GifFileIn->Image.ColorMap) == GIF_ERROR)
			    QuitGifError(GifFileIn, GifFileOut);

			    /* Now read image itself in decoded form as we dont      */
			    /* really care what is there, and this is much faster.   */
		  	if (DGifGetCode(GifFileIn, &CodeSize, &CodeBlock) == GIF_ERROR
		     	   || EGifPutCode(GifFileOut, CodeSize, CodeBlock) == GIF_ERROR)
			   QuitGifError(GifFileIn, GifFileOut);
		    	while (CodeBlock != NULL)
				if (DGifGetCodeNext(GifFileIn, &CodeBlock) == GIF_ERROR ||
			   	    EGifPutCodeNext(GifFileOut, CodeBlock) == GIF_ERROR)
				    QuitGifError(GifFileIn, GifFileOut);
		    	break;
			case EXTENSION_RECORD_TYPE:
				FileEmpty = FALSE;
		    		/* Skip any extension blocks in file: */
		    		if (DGifGetExtension(GifFileIn, &ExtCode, &Extension)
				    == GIF_ERROR)
				    QuitGifError(GifFileIn, GifFileOut);
		    		if (EGifPutExtension(GifFileOut, ExtCode, Extension[0],
							Extension) == GIF_ERROR)
				    QuitGifError(GifFileIn, GifFileOut);

		    		/* No support to more than one extension blocks, discard.*/
		    		while (Extension != NULL)
				if (DGifGetExtensionNext(GifFileIn, &Extension)
			   	 == GIF_ERROR)
			   	 QuitGifError(GifFileIn, GifFileOut);
		    		break;
			case TERMINATE_RECORD_TYPE:
		    	break;
			default:	    /* Should be traps by DGifGetRecordType. */
		    	break;
	    	}
		}
		while (RecordType != IMAGE_DESC_RECORD_TYPE &&
	               RecordType != TERMINATE_RECORD_TYPE);

		if (EGifCloseFile(GifFileOut) == GIF_ERROR)
	    	    QuitGifError(GifFileIn, GifFileOut);
		if (FileEmpty) {
	  	  /* Might happen on last file - delete it if so: */
		    unlink(CrntFileName);
		}
  	 }
    	while (RecordType != TERMINATE_RECORD_TYPE);

    	if (DGifCloseFile(GifFileIn) == GIF_ERROR)
		QuitGifError(GifFileIn, GifFileOut);
   	FileNum=FileNum-1; 
  	}
return FileNum;
}

/******************************************************************************
* Close both input and output file (if open), and exit.			      *
******************************************************************************/
void QuitGifError(GifFileType *GifFileIn, GifFileType *GifFileOut)
{
#ifndef ENABLE_GIFLIB
    PrintGifError();
#endif
    if (GifFileIn != NULL) DGifCloseFile(GifFileIn);
    if (GifFileOut != NULL) EGifCloseFile(GifFileOut);
//    exit(EXIT_FAILURE);
}


#ifdef ENABLE_GIFLIB
static void GIF_EXIT(char *err) {
	fprintf(stderr, "%s\n", err);
	exit(-1);
}
#endif
int LoadImage(GifFileType *GifFile, GifRowType **ImageBufferPtr)
{
    int Size, i, j, Count;
    GifRowType *ImageBuffer;

    /* Allocate the image as vector of column of rows. We cannt allocate     */
    /* the all screen at once, as this broken minded CPU can allocate up to  */
    /* 64k at a time and our image can be bigger than that:		     */
    if ((ImageBuffer = (GifRowType *)
	malloc(GifFile->Image.Height * sizeof(GifRowType *))) == NULL)
	    GIF_EXIT("Failed to allocate memory required, aborted.");

    Size = GifFile->Image.Width * sizeof(GifPixelType);/* One row size in bytes.*/
    for (i = 0; i < GifFile->Image.Height; i++) {
	/* Allocate the rows: */
	if ((ImageBuffer[i] = (GifRowType) malloc(Size)) == NULL)
	    GIF_EXIT("Failed to allocate memory required, aborted.");
    }

    *ImageBufferPtr = ImageBuffer;

#ifndef ENABLE_GIFLIB
    GifQprintf("\n%s: Image %d at (%d, %d) [%dx%d]:     ",
	PROGRAM_NAME, ++ImageNum, GifFile->Image.Left, GifFile->Image.Top,
				 GifFile->Image.Width, GifFile->Image.Height);
#endif
    if (GifFile->Image.Interlace) {
	/* Need to perform 4 passes on the images: */
	for (Count = i = 0; i < 4; i++)
	    for (j = InterlacedOffset[i]; j < GifFile->Image.Height;
						 j += InterlacedJumps[i]) {
//		GifQprintf("\b\b\b\b%-4d", Count++);
		if (DGifGetLine(GifFile, ImageBuffer[j], GifFile->Image.Width)
		    == GIF_ERROR) return GIF_ERROR;
	    }
    }
    else {
	for (i = 0; i < GifFile->Image.Height; i++) {
//	    GifQprintf("\b\b\b\b%-4d", i);
	    if (DGifGetLine(GifFile, ImageBuffer[i], GifFile->Image.Width)
		== GIF_ERROR) return GIF_ERROR;
	}
    }

    return GIF_OK;
}

/******************************************************************************
* Routine to dump image out. The given Image buffer should always hold the    *
* image sequencially. Image will be dumped according to IInterlaced flag in   *
* GifFile structure. Once dumped, the memory holding the image is freed.      *
* Return GIF_OK if succesful, GIF_ERROR otherwise.			      *
******************************************************************************/
int DumpImage(GifFileType *GifFile, GifRowType *ImageBuffer)
{
    int i, j, Count;

    if (GifFile->Image.Interlace) {
	/* Need to perform 4 passes on the images: */
	for (Count = GifFile->Image.Height, i = 0; i < 4; i++)
	    for (j = InterlacedOffset[i]; j < GifFile->Image.Height;
						 j += InterlacedJumps[i]) {
//		GifQprintf("\b\b\b\b%-4d", Count--);
		if (EGifPutLine(GifFile, ImageBuffer[j], GifFile->Image.Width)
		    == GIF_ERROR) return GIF_ERROR;
	    }
    }
    else {
	for (Count = GifFile->Image.Height, i = 0; i < GifFile->Image.Height; i++) {
//	    GifQprintf("\b\b\b\b%-4d", Count--);
	    if (EGifPutLine(GifFile, ImageBuffer[i], GifFile->Image.Width)
		== GIF_ERROR) return GIF_ERROR;
	}
    }

    /* Free the m emory used for this image: */
    for (i = 0; i < GifFile->Image.Height; i++) free((char *) ImageBuffer[i]);
    free((char *) ImageBuffer);

    return GIF_OK;
}
