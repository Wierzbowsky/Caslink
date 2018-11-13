//******************************************************************************
//  FILE..........: CASLINK3.C
//  COPYRIGHT.....: Copyright (C) 1999-2018 Alexey Podrezov
//  VERSION.......: 3.1
//  DESCRIPTION...: Cassette Interface Emulation utility for MSX computers
//  NOTES.........: Encodes/decodes files to be transfered through MSX cassette interface
//  HISTORY.......:
//  PERSON          DATE         COMMENT
//  --------        ----------   -----------------------------------------------
//  Podrezov        07/09/1999   Initial version created
//  Podrezov        26/08/2005   Sources ported to Visual Studio .NET
//  Podrezov        28/08/2005   Added support for 16 bit, stereo, 44 kHz
//  Podrezov        29/08/2005   Modified ROM loaders
//  Podrezov        30/08/2005   Added sound file playback
//  Podrezov        05/09/2005   Increased parts number to 15, multifile loader modified
//  Podrezov        12/09/2005   Fixed detokenized Basic file loading bug in multifile loader routine
//  Podrezov        25/09/2005   Added preloader for BINs, ROMs
//  Podrezov        26/09/2005   Modified and recompiled ROM loaders, created BIN2INC utility
//  Podrezov        29/09/2005   Added preloader for multi-file ROMs
//  Podrezov        03/10/2005   Wrote WAV converter from CD to RD quality
//  Podrezov        14/03/2008   Sources ported to Visual Studio 2005
//  Podrezov        17/03/2008   Added MP3 conversion capability, added colors for messages
//  Podrezov        19/03/2008   Disabled signal level question (now it always is 100), fixed waveform of CD quality WAV file
//  Podrezov        21/03/2008   Added option for normal and inverted waveforms for different MP3 players
//  Podrezov        22/03/2008   Fixed a bug in WAV file length calculation when only binary files are encoded
//  Podrezov        23/03/2008   Added copyright message to WAV files, added wrong CR+LF check for input ASCII files
//  Podrezov        26/03/2008   Re-wrote all ROM loaders, created single BIN file loader, added CRC check
//  Podrezov        28/03/2008   Added 3000 baud MSX cassette I/O speed support!
//  Podrezov        05/08/2008   Added missing detokenized file encoding code in single file encoding routine
//  Podrezov        29/10/2008   Fixed a bug in 32kb ROM loading routine, now all such ROMs should start normally
//	Podrezov		08/02/2016   Changed the second loader for 32kb files so it does not reset a computer. Added new loader that resets the computer and changed /r option to be used with it, removed decoding and playback functions
//	Podrezov		13/02/2016   Rewrote 8-16kb ROM loaders to work correctly with files that start from different addresses, introduced the final 32kb loader with reset and fixed the bug in the 32kb loader without reset
//	Podrezov		14/02/2016   Added 49kb ROM loaders and handling routines, fixed MP3 file waiting routine
//	Podrezov		18/02/2016   Fixed bug in 3rd 49kb loader routine (with reset)
//	Podrezov		23/02/2016   Refined file recognition routine to exclude files that can't be loaded, added rudimentary detection of ROM files with mappers
//  Podrezov		07/03/2016   Fixed the 49kb final loader, tested on numerous MSXs (1,2,2+), corrected patching routine for multi file loader (after its optimization)
//	Podrezov		05/05/2016   Fixed the 49kb final non-reset loader, now BIOS is enabled before passing control to the ROM
//	Podrezov		06/05/2016   Added a patcher for 3 erased bytes at 8000H into the loaders with reset functionality
//	Podrezov		07/05/2016   Increased the max amount of loaded files to 20
//	Podrezov		08/05/2016   The data in loaded BIN files is also moved to the destination address from the end to avoid corruption
//	Podrezov		09/05/2016   Wrong input does not cause the program to exit with error any more, only correct input is accepted; BIN loaders now start from 8200h to load 24kb BIN files
//	Podrezov		21/05/2016   Fixed missing CCF opcode before ADC and SBC operations, the set flag could affect the operations
//	Podrezov		04/03/2018   Fixed the 2400 baud file generation so that it is also accepted by emulators; adjusted the length of headers, verified the highest baud rate to be around 3000 baud
//******************************************************************************

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <stddef.h>
#include <iostream>
#include <shellapi.h>
#include <ctype.h>
#include <malloc.h>
#include "caslink3.h"


// Main routine ----------------------------------------------------------------

int main(int argc, char *argv[])
{
	
	// Console initialization
	
	hStdIn	=	GetStdHandle(STD_INPUT_HANDLE);
	hStdOut	=	GetStdHandle(STD_OUTPUT_HANDLE);
	
	// Print title
	SetConsoleTitle(szCaption);		
	SetConsoleTextAttribute(hStdOut, 0x0e);
	WriteConsole(hStdOut, szTitle1, (DWORD) strlen(szTitle1), &Result, NULL);
	SetConsoleTextAttribute(hStdOut, 0x07);
	WriteConsole(hStdOut, szTitle2, (DWORD) strlen(szTitle2), &Result, NULL);
	SetConsoleTextAttribute(hStdOut, 0x0f);

	//Output help hint
	if (argc == 1)
	{
	   	WriteConsole(hStdOut, szNoAction, (DWORD) strlen(szNoAction), &Result, NULL);
		return 0;
	}

	// Check command line parameters
	if (*argv[1] != '/') ExitOnError(1);
	if (*(argv[1] + 1) == '?')
	{
		WriteConsole(hStdOut, szHelp, (DWORD) strlen(szHelp), &Result, NULL);
		return 0;
	}

	else if (*(argv[1] + 1) == 'E' || *(argv[1] + 1) == 'e' || *(argv[1] + 1) == 'M' || *(argv[1] + 1) == 'm' || *(argv[1] + 1) == 'R' || *(argv[1] + 1) == 'r')	// Encode to WAV
	{
		// Check if filename is missing
		EncodeDecode = 1;
		if (argc < 3) ExitOnError(2);
		if ((*(argv[1] + 1) == 'R' || *(argv[1] + 1) == 'r') && argc > 3) ExitOnError(1);
		if ((*(argv[1] + 1) == 'M' || *(argv[1] + 1) == 'm') && argc <= 3) ExitOnError(1);
		if ((*(argv[1] + 1) == 'E' || *(argv[1] + 1) == 'e') && argc > 3) ExitOnError(1);
		if (strlen(argv[2]) > 12 || *(argv[1] + 1) < '0' || *(argv[1] + 1) > 'z') ExitOnError(1);

		// Lowercase the string and copy input/output file name to buffers
		for (i = 0; i < strlen(argv[2]); i++)
		{
			*(argv[2]+i) = (char)tolower(*(argv[2]+i));
			InputFileName[i] = *(argv[2]+i);
		}
		InputFileName[i] = '\0';
		for (i = 0; i < 8; i++)
		{
			if (InputFileName[i] == '.') break;
			OutputFileName[i] = InputFileName[i];
		}
		OutputFileName[i] = '\0';
		if (EncodeDecode < 3) strcat_s(OutputFileName, 30, ".wav");	  // Add extension

		if (*(argv[1] + 1) == 'R' || *(argv[1] + 1) == 'r') EncodeDecode = 3;
		else if (*(argv[1] + 1) == 'M' || *(argv[1] + 1) == 'm')
		{
			// Check if more filenames are missing
			EncodeDecode = 2;	// Encode to WAV
			if (argc < 3) ExitOnError(2);
			k = argc;
			if (k > 22) k = 22;
			for (l = 3; l < k; l++)	if (strlen(argv[l]) > 12 || *(argv[l] + 1) < '0' || *(argv[l] + 1) > 'z') ExitOnError(1);
			for (l = 0; l < (unsigned)argc; l++)
			{
				for (i = 0; i < strlen(argv[l]); i++)
				{
					*(argv[l]+i) = (char)tolower(*(argv[l]+i));
				}
				FileNamePointer[l] = argv[l];
			}
	   	}
	}

	// In other cases exit
	else ExitOnError(1);

	// Ask MSX speed interface, sample rate and other questions
	SignalLevel = 100;	// always maximum signal level

	WriteConsole(hStdOut, szSpeed, (DWORD) strlen(szSpeed), &Result, NULL);
	MSXInterfaceSpeed = 4800;
	while (MSXInterfaceSpeed == 4800)
	{
		ch = _getch();
		if (ch == '3') MSXInterfaceSpeed = 3000;
		else if (ch == '2') MSXInterfaceSpeed = 2400;
		else if (ch == '1') MSXInterfaceSpeed = 1200;
	}
	//else ExitOnError(3);
	wsprintf(Buffer, TEXT("%c"), ch);
	WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);	// print speed

	if (MSXInterfaceSpeed == 1200)
	{
		WriteConsole(hStdOut, szQuality, (DWORD) strlen(szQuality), &Result, NULL);
		ConvertFlag = 2;
		while (ConvertFlag == 2)
		{
			ch = _getch();
			if (ch == '2') ConvertFlag = 1;
			else if (ch == '1') ConvertFlag = 0;
		}
		//else ExitOnError(3);
		wsprintf(Buffer, TEXT("%c"), ch);
		WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);	// print quality
	}

	WriteConsole(hStdOut, szInvertedWF, (DWORD) strlen(szInvertedWF), &Result, NULL);
	InvertedWaveform = 2;
	while (InvertedWaveform == 2)
	{
		ch = _getch();
		if (ch == '2') InvertedWaveform = 1;
		else if (ch == '1') InvertedWaveform = 0;
	}
	//else ExitOnError(3);
	wsprintf(Buffer, TEXT("%c\n"), ch);
	WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);	// print waveform

	WriteConsole(hStdOut, szConvert, (DWORD) strlen(szConvert), &Result, NULL);
	ConvertToMP3 = 2;
	while (ConvertToMP3 == 2)
	{
		ch = _getch();
		if (ch == 'Y' || ch == 'y') ConvertToMP3 = 1;
		else if (ch == 'N' || ch == 'n') ConvertToMP3 = 0;
	}
	//else ExitOnError(3);
	wsprintf(Buffer, TEXT("%c\n"), toupper(ch));
	WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);	// print convert y/n

	if (ConvertToMP3 == 1)
	{
		WriteConsole(hStdOut, szMP3Kbps, (DWORD) strlen(szMP3Kbps), &Result, NULL);
		MP3Kbps = 512;
		while (MP3Kbps == 512)
		{
			ch = _getch();
			if (ch == '3') MP3Kbps = 320;
			else if (ch == '2') MP3Kbps = 256;
			else if (ch == '1') MP3Kbps = 192;
		}
		//else ExitOnError(3);
		wsprintf(Buffer, TEXT("%c\n"), ch);
		WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);	// print MP3 bitrate
	}

	if (MSXInterfaceSpeed == 3000)
	{
		ConvertRatio = 3;
		MSXInterfaceSpeed = 2400;
		if(InvertedWaveform == 0) InvertedWaveform = 1;
		else InvertedWaveform = 0;
		ConvertFlag = 1;
		SetConsoleTextAttribute(hStdOut, 0x0e);
		WriteConsole(hStdOut, szTopSpeedSel, (DWORD) strlen(szTopSpeedSel), &Result, NULL);	// print 3000 baud message
		SetConsoleTextAttribute(hStdOut, 0x0f);
	}
	else if (MSXInterfaceSpeed == 2400)
	{
		MSXInterfaceSpeed = 1200;
		ConvertRatio = 2;
		ConvertFlag = 1;
	}
	else ConvertRatio = 4;
		
	// Make primary preparations
	PrimarySetup();


	// Encode or decode file(s)
	if (EncodeDecode == 1 || EncodeDecode == 3) EncodeSingleFile();
	else if (EncodeDecode == 2) EncodeMultipleFiles(argc);

	// Convert to MP3 if possible
	if (ConvertToMP3 == 1) ConvertOutputToMP3();

	// Print final messages
	SetConsoleTextAttribute(hStdOut, 0x0a);
	if (ConvertToMP3 == 0) WriteConsole(hStdOut, szEncSuccMsg1, (DWORD) strlen(szEncSuccMsg1), &Result, NULL);
	else WriteConsole(hStdOut, szEncSuccMsg2, (DWORD) strlen(szEncSuccMsg2), &Result, NULL);
	SetConsoleTextAttribute(hStdOut, 0x0f);

	// Cleanup
	CleanUp();

	// Final message
	SetConsoleTextAttribute(hStdOut, 0x07);
	WriteConsole(hStdOut, szFinalMsg, (DWORD) strlen(szFinalMsg), &Result, NULL);
	SetConsoleTextAttribute(hStdOut, 0x0f);

	return 0;
}

// Encode single file to WAV sound file routine ---------------------------------

void EncodeSingleFile(void)
{
	// Open, check size, allocate memory and read input file
	if (fopen_s(&InputFileHandle, InputFileName, "rb")) ExitOnError(6);
	if (fseek(InputFileHandle, 0, SEEK_END)) ExitOnError(7);
	InputFileLength = (size_t)ftell(InputFileHandle);
	if (fseek(InputFileHandle, 0, SEEK_SET)) ExitOnError(7);
	if (InputFileLength < (long)MINENCINPUTFILELENGTH || InputFileLength > (long)MAXSENCINPUTFILELENGTH) ExitOnError(23);
	InputMemPointer = (unsigned char*) malloc(InputFileLength + 10);
	if (!InputMemPointer) ExitOnError(5);
	ZeroMemory(InputMemPointer, InputFileLength + 10);
	BytesRead = fread(InputMemPointer, sizeof(unsigned char), InputFileLength, InputFileHandle);
	if (BytesRead != InputFileLength) ExitOnError(9);

	// Check program type and make necessary adjustments
	if ((unsigned char)InputMemPointer[0] == 0xff && InputFileLength <= 25000L)
	{
		FileType = 1;			// tokenized basic program
		ExtraBytes = 7;
		FileOffset = 1;
		MoreExtraBytes = 0;
	}
	else if ((unsigned char)InputMemPointer[0] == 0xfe && InputFileLength <= 17000L)
	{
		FileType = 2;			// BIN program
		FileOffset = 7;

		// Preloader size calculation
		MoreExtraBytes = (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength) + (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength) +
     	(DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength) +
		(DWORD)(((DWORD)sizeof(BinFileHeaderID) + (DWORD)CASFILENAMELENGTH) * (DWORD)SampleScale * (DWORD)sizeof(BitStream) * (DWORD)BitEncodingLength);
		ExtraBytes = sizeof(BINLoader) + sizeof(BINROMPreLoader) + 6;
	}
	else if (InputFileLength >= 33000L && InputFileLength <= 50000L && (((unsigned char)InputMemPointer[0] == 'A' && (unsigned char)InputMemPointer[1] == 'B') || ((unsigned char)InputMemPointer[0x4000] == 'A' && (unsigned char)InputMemPointer[0x4001] == 'B')))
	{
		if (InputFileLength > 46000L && (unsigned char)InputMemPointer[3] >= 0x40 && (unsigned char)InputMemPointer[0x4000] != 'A' && (unsigned char)InputMemPointer[0x4001] != 'B') ExitOnError(8);

		FileType = 3;			// ROM image
		FileOffset = 0;
		ExtraBytes = 0;
		MoreExtraBytes = 0;

		// Preloader size calculation
		MoreExtraBytes = (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength) + (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength) +
     	(DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength) +
		(DWORD)(((DWORD)sizeof(BinFileHeaderID) + (DWORD)CASFILENAMELENGTH) * (DWORD)SampleScale * (DWORD)sizeof(BitStream) * (DWORD)BitEncodingLength);
		ExtraBytes = sizeof(BINROMPreLoader) + 6;

		// Extra areas for second and third part of ROM image calculation
		if (EncodeDecode == 1)
		{
			MoreExtraBytes = MoreExtraBytes + ((DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength) + (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength) +
	     	(DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength) +
			(DWORD)(((DWORD)sizeof(BinFileHeaderID) + (DWORD)CASFILENAMELENGTH - (DWORD)FileOffset) * (DWORD)SampleScale * (DWORD)sizeof(BitStream) * (DWORD)BitEncodingLength)) * 2;
			ExtraBytes = ExtraBytes + sizeof(ROM49KLoader1) + sizeof(ROM49KLoader2) + sizeof(ROM49KLoader3) + 12;
		}
		else if (EncodeDecode == 3)
		{
			MoreExtraBytes = MoreExtraBytes + ((DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength) + (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength) +
	     	(DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength) +
			(DWORD)(((DWORD)sizeof(BinFileHeaderID) + (DWORD)CASFILENAMELENGTH - (DWORD)FileOffset) * (DWORD)SampleScale * (DWORD)sizeof(BitStream) * (DWORD)BitEncodingLength)) * 2;
			ExtraBytes = ExtraBytes + sizeof(ROM49KLoader1) + sizeof(ROM49KLoader2) + sizeof(ROM49KLoader3Rst) + 12;
		}
	}
	else if ((unsigned char)InputMemPointer[0] == 'A' && (unsigned char)InputMemPointer[1] == 'B' && InputFileLength < 33000L)
	{
		FileType = 3;			// ROM image
		FileOffset = 0;
		ExtraBytes = 0;
		MoreExtraBytes = 0;

		// Preloader size calculation
		MoreExtraBytes = (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength) + (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength) +
     	(DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength) +
		(DWORD)(((DWORD)sizeof(BinFileHeaderID) + (DWORD)CASFILENAMELENGTH) * (DWORD)SampleScale * (DWORD)sizeof(BitStream) * (DWORD)BitEncodingLength);
		ExtraBytes = sizeof(BINROMPreLoader) + 6;

		if (InputFileLength <= 17000L)
		{
			if (EncodeDecode == 1)
				ExtraBytes = ExtraBytes + sizeof(ROMLoader) + 6;
			else if (EncodeDecode == 3)
				ExtraBytes = ExtraBytes + sizeof(ROMLoaderRst) + 6;
		}
		else if (InputFileLength > 17000L && InputFileLength < 33000L)
		{
			// Extra areas for second part of ROM image calculation
			if (EncodeDecode == 1)
			{
				MoreExtraBytes = MoreExtraBytes + (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength) + (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength) +
	     		(DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength) +
				(DWORD)(((DWORD)sizeof(BinFileHeaderID) + (DWORD)CASFILENAMELENGTH - (DWORD)FileOffset) * (DWORD)SampleScale * (DWORD)sizeof(BitStream) * (DWORD)BitEncodingLength);
				ExtraBytes = ExtraBytes + sizeof(ROM32KLoader1) + sizeof(ROM32KLoader2) + 12;
			}
			else if (EncodeDecode == 3)
			{
				MoreExtraBytes = MoreExtraBytes + (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength) + (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength) +
	     		(DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength) +
				(DWORD)(((DWORD)sizeof(BinFileHeaderID) + (DWORD)CASFILENAMELENGTH - (DWORD)FileOffset) * (DWORD)SampleScale * (DWORD)sizeof(BitStream) * (DWORD)BitEncodingLength);
				ExtraBytes = ExtraBytes + sizeof(ROM32KLoader1) + sizeof(ROM32KLoader2Rst) + 12;
			}
		}
		else ExitOnError(23);
	}
	else if (InputFileLength < 50000L)
	{
		for (i = 0; i < InputFileLength; i++)
		{
			if ((unsigned char)InputMemPointer[i] >= ' ' || (unsigned char)InputMemPointer[i] <= 2 || (unsigned char)InputMemPointer[i] == 0xa || (unsigned char)InputMemPointer[i] == 0xd || (unsigned char)InputMemPointer[i] == 0x1a) continue;
			else ExitOnError(10);
		}

		// Check for correct CR+LF
		for (i = 1; i < InputFileLength; i++)
			if ((unsigned char)InputMemPointer[i] == 0xa && (unsigned char)InputMemPointer[i-1] != 0xd) ExitOnError(24);

		FileType = 4;			// ASCII basic file
		ExtraBytes = 256;
		FileOffset = 0;
		MoreExtraBytes = 0;
   	}
	else ExitOnError(23);

	// Check if a valid ROM
	if (FileType == 3)
	{
		if (InputFileLength >= 33000L && InputFileLength < 50000L)
		{
			if (((unsigned char)InputMemPointer[0] != 'A' && (unsigned char)InputMemPointer[1] != 'B') && ((unsigned char)InputMemPointer[0x4000] != 'A' && (unsigned char)InputMemPointer[0x4001] != 'B'))
				ExitOnError(23);
		}
		else
		{
			ch = (unsigned char)InputMemPointer[3];
			ch = ch & 0xf0;
			if (InputFileLength > MAXSENCINPUTFILELENGTH && ch >= 0xD0) ExitOnError(14);
		}
	}

	// Fill WAV header
	WavHeader.WavHeaderID[0] = 'R';
	WavHeader.WavHeaderID[1] = 'I';
	WavHeader.WavHeaderID[2] = 'F';
	WavHeader.WavHeaderID[3] = 'F';
	WavHeader.MMFileType[0] = 'W';
	WavHeader.MMFileType[1] = 'A';
	WavHeader.MMFileType[2] = 'V';
	WavHeader.MMFileType[3] = 'E';
	WavHeader.FormatID[0] = 'f';
	WavHeader.FormatID[1] = 'm';
	WavHeader.FormatID[2] = 't';
	WavHeader.FormatID[3] = ' ';
	WavHeader.DataID[0] = 'd';
	WavHeader.DataID[1] = 'a';
	WavHeader.DataID[2] = 't';
	WavHeader.DataID[3] = 'a';
	WavHeader.BlkAllign = (WORD) 1;
	WavHeader.FormatTag = (WORD) 1;				// PCM
	WavHeader.NumChannels = (WORD) 1;			// mono
	WavHeader.BitsPerSample = (WORD) 8;			// 8 bit
	WavHeader.SamplesPerSec = (DWORD)WavSampleRate;
	WavHeader.BytesPerSec = (DWORD)((DWORD)WavSampleRate * (DWORD)(WavHeader.BitsPerSample / 8) * (DWORD)WavHeader.NumChannels);
	WavHeader.FormatSize = (DWORD)(sizeof(WavHeader.FormatTag) + sizeof(WavHeader.NumChannels) + sizeof(WavHeader.BitsPerSample) +
   	sizeof(WavHeader.SamplesPerSec) + sizeof(WavHeader.BytesPerSec) + sizeof(WavHeader.BlkAllign));
	WavHeader.PureSampleLength = 0;
	if (FileType == 1) FileHeaderIDLength = sizeof(BasicFileHeaderID);
	else if (FileType == 2 || FileType == 3) FileHeaderIDLength = sizeof(BinFileHeaderID);
	if (FileType == 4) FileHeaderIDLength = sizeof(DataFileHeaderID);
	if (FileType != 4)
	{
		WavHeader.PureSampleLength = (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength + (DWORD)WavSampleRate * (DWORD)MiddlePauseLength + (DWORD)WavSampleRate * (DWORD)FinalPauseLength) +	// Length of pauses
		(DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength) +	// Length of long and short headers
		(DWORD)(((DWORD)FileHeaderIDLength + (DWORD)CASFILENAMELENGTH + (DWORD)InputFileLength + (DWORD)ExtraBytes - (DWORD)FileOffset) * (DWORD)SampleScale * (DWORD)sizeof(BitStream) * (DWORD)BitEncodingLength) +	// Length of data areas
		MoreExtraBytes;  // Additional bytes
	}
	else
	{
		b = 1;
		if (InputFileLength > 256)
		{
			InputFileLengthTmp = InputFileLength;
			for (b = 2; ; b++)
	      	{
				InputFileLengthTmp = InputFileLengthTmp - 256;
				if (InputFileLengthTmp <= 256) break;
			}
		}
		WavHeader.PureSampleLength = (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength + (DWORD)WavSampleRate * (DWORD)MiddlePauseLength + (DWORD)WavSampleRate * (DWORD)FinalPauseLength) +	// Length of pauses
		(DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength * (DWORD)b) +	// Length of long and all short headers
		(DWORD)(((DWORD)FileHeaderIDLength + (DWORD)CASFILENAMELENGTH + (DWORD)InputFileLength + (DWORD)ExtraBytes - (DWORD)FileOffset) * (DWORD)SampleScale * (DWORD)sizeof(BitStream) * (DWORD)BitEncodingLength) +	// Length of data areas
		MoreExtraBytes;
	}
	WavHeader.SampleLength = (DWORD)((DWORD)WavHeader.PureSampleLength + (DWORD)WAVHEADEROFFSETVALUE);

	// Check if file already exists
	if (!fopen_s(&OutputFileHandle, OutputFileName, "rb"))
	{
		SetConsoleTextAttribute(hStdOut, 0x0e);
		WriteConsole(hStdOut, szOverwrite, (DWORD) strlen(szOverwrite), &Result, NULL);
		SetConsoleTextAttribute(hStdOut, 0x0f);
		ch = 'A';
		while (toupper(ch) != 'Y')
		{
			ch = _getch();
			if (toupper(ch) == 'N') ExitOnError(11);
		}
		wsprintf(Buffer, TEXT("%c\n"), toupper(ch));
		WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);	// print selection
		fclose(OutputFileHandle);
		OutputFileHandle = 0;
	}
	wsprintf(Buffer, szCreateOutFile, OutputFileName);
	WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);

	// Print file format
	wsprintf(Buffer, TEXT("\n  [ %s ]: File format - %s\n"), InputFileName, FileFormat[FileType - 1]);
	SetConsoleTextAttribute(hStdOut, 0x08);
	WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);	// print input format
	SetConsoleTextAttribute(hStdOut, 0x0f);

	// Write output file header (WAV)
	if (fopen_s(&OutputFileHandle, OutputFileName, "wb")) ExitOnError(12);
	BytesWritten = fwrite(&WavHeader, sizeof(unsigned char), sizeof(WavHeader), OutputFileHandle);
	if (BytesWritten != sizeof(WavHeader)) ExitOnError(13);

	// Prepare file name for tape header
	for (i = 0; i < CASFILENAMELENGTH; i++)
	{
		if (InputFileName[i] != '.' && InputFileName[i]) NameBuffer[i] = InputFileName[i];
		else break;
	}
	for ( ; i < CASFILENAMELENGTH; i++) NameBuffer[i] = ' ';	// Fill the rest with spaces

	// Encode file name into ROM loader and BIN/ROM preloader
	for (i = 0; i < CASFILENAMELENGTH; i++)
	{
		(unsigned char)ROM32KLoader1[i + 0x15] = (unsigned char)NameBuffer[i];	// 32k ROM loader 1
		(unsigned char)ROM49KLoader1[i + 0x15] = (unsigned char)NameBuffer[i];	// 49k ROM loader 1
		(unsigned char)ROM49KLoader2[i + 0x15] = (unsigned char)NameBuffer[i];	// 49k ROM loader 2
		(unsigned char)BINROMPreLoader[i + 0x0e] = (unsigned char)NameBuffer[i];	// Preloader for BIN/ROM files
	}
	if ((FileType == 2 || FileType == 3) && InputFileLength < 17000L)
	{
		for (i = 0; i < 6; i++) (unsigned char)BINROMPreLoader[i + 0x15] = ' ';	// Erase 'part 1' from command line in case only one file is encoded
	}

	// Encode preloader for BIN/ROM files
	if (FileType == 2 || FileType == 3)
	{
		// Encode first pause
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Create and encode long header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}
	
		// Encode preloader header data
   		for (i = 0; i < sizeof(BinFileHeaderID); i++) if (WriteDataByte(BinFileHeaderID[i])) ExitOnError(13);	// BIN/ROM ID
		
		// Encode file name
		for (i = 0; i < CASFILENAMELENGTH; i++)
		{
			if (WriteDataByte(NameBuffer[i])) ExitOnError(13);	// File name
		}

		// Encode second pause
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Create and encode short header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		// Set binary start addresses for preloader
		AddressBuffer[0] = 0;
		AddressBuffer[1] = 0x90;
		a = (size_t)((size_t)sizeof(BINROMPreLoader) + (size_t)0x9000 - 1);
		AddressBuffer[2] = (char)a;
		AddressBuffer[3] = (char)(a >> 8);
		AddressBuffer[4] = 0;
		AddressBuffer[5] = 0x90;

		// Encode 6 bytes of addresses
		for (i = 0; i < sizeof(AddressBuffer); i++)
		{
			ch = (unsigned char)AddressBuffer[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode preloader data
		for (i = 0; i < sizeof(BINROMPreLoader); i++)
		{
			ch = (unsigned char)BINROMPreLoader[i];	// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}
	}

	// Encode first pause (or actually pause between files if preloader was used)
	for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength); j++)
	{
		if (WriteServiceByte(SilenceByte)) ExitOnError(13);
	}

	// Create and encode long header
	for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength / (DWORD)BitEncodingLength); j++)
	{
		if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
	}

	// Encode file header data
	if (FileType == 1)
	{
   		for (i = 0; i < sizeof(BasicFileHeaderID); i++) if (WriteDataByte(BasicFileHeaderID[i])) ExitOnError(13);	// Basic ID
	}
	else if (FileType == 2 || FileType == 3)
	{
   		for (i = 0; i < sizeof(BinFileHeaderID); i++) if (WriteDataByte(BinFileHeaderID[i])) ExitOnError(13);	// BIN/ROM ID
	}
	else
	{
   		for (i = 0; i < sizeof(DataFileHeaderID); i++) if (WriteDataByte(DataFileHeaderID[i])) ExitOnError(13);	// Data ID
	}

	// Encode file name
	for (i = 0; i < CASFILENAMELENGTH; i++)
	{
		if (WriteDataByte(NameBuffer[i])) ExitOnError(13);	// File name
	}

	// Encode second pause
	for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength); j++)
	{
		if (WriteServiceByte(SilenceByte)) ExitOnError(13);
	}

	// Encode tokenized BAS file
	if (FileType == 1)
	{
		// Create and encode short header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		// Encode whole program data
		for (i = FileOffset; i < InputFileLength; i++)
		{
			ch = (unsigned char)InputMemPointer[i];	// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}
	}
	
	// Encode BIN files
	else if (FileType == 2)
	{
		// Calculate CRC of the whole BIN file excluding the header
		RomCRC = 0;
		RomBegin = (unsigned short)((unsigned char)InputMemPointer[2] * 0x100 + (unsigned char)InputMemPointer[1]);
		RomEnd = (unsigned short)((unsigned char)InputMemPointer[4] * 0x100 + (unsigned char)InputMemPointer[3]);
		RomStart = (unsigned short)((unsigned char)InputMemPointer[6] * 0x100 + (unsigned char)InputMemPointer[5]);
		if (RomEnd <= RomBegin || RomStart >= RomEnd || RomStart < RomBegin) ExitOnError(25);
		for (i = FileOffset; i < (RomEnd-RomBegin) + FileOffset; i++) RomCRC = RomCRC + InputMemPointer[i];

		// Copy BIN start addresses and CRC to loader
		for (j = 0; j < 6; j++) BINLoader[j + 3] = InputMemPointer[j + 1];
		BINLoader[9] = RomCRC;

		// Create and encode short header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		// Set binary start addresses
		AddressBuffer[0] = 0;
		AddressBuffer[1] = 0x82;
		a = (size_t)((size_t)sizeof(BINLoader) + (size_t)InputFileLength + (size_t)0x8200 - FileOffset - 1);
		AddressBuffer[2] = (char)a;
		AddressBuffer[3] = (char)(a >> 8);
		AddressBuffer[4] = 0;
		AddressBuffer[5] = 0x82;

		// Encode 6 bytes of addresses
		for (i = 0; i < sizeof(AddressBuffer); i++)
		{
			ch = (unsigned char)AddressBuffer[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode BIN loader
		for (i = 0; i < sizeof(BINLoader); i++)
		{
			ch = (unsigned char)BINLoader[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode whole BIN file
		for (i = FileOffset; i < InputFileLength; i++)
		{
			ch = (unsigned char)InputMemPointer[i];	// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}
	}

	// Encode 8k and 16k ROM files
	else if (FileType == 3 && InputFileLength <= 17000L)	// ROM file
	{
		// Calculate CRC of the whole ROM
		RomCRC = 0;
		for (i = 0; i < InputFileLength; i++) RomCRC = RomCRC + InputMemPointer[i];

		// Create and encode short header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		ch = (unsigned char)InputMemPointer[3];
		if ((unsigned char)InputMemPointer[3] >= (unsigned char)0x80) ch = (unsigned char)0x80;
		else if ((unsigned char)InputMemPointer[3] < (unsigned char)0x40) ch = 0;
		else ch = (unsigned char)0x40;
	
		// Set ROM loader addresses
		ch = ch & 0xf0;
		a = (size_t)ch;
		a = a << 8;

		if (EncodeDecode == 1)
		{
			ROMLoader[3] = 0;
			ROMLoader[4] = (char)(a >> 8);
			b = a + InputFileLength;
			ROMLoader[5] = (char)b;
			ROMLoader[6] = (char)(b >> 8);
			ROMLoader[7] = (char)InputMemPointer[2];
			ROMLoader[8] = (char)InputMemPointer[3];
			ROMLoader[9] = RomCRC;
			a = (size_t)((size_t)sizeof(ROMLoader) + (size_t)InputFileLength + (size_t)0x9000 - 1);
		}
		else if (EncodeDecode == 3)
		{
			ROMLoaderRst[3] = 0;
			ROMLoaderRst[4] = (char)(a >> 8);
			b = a + InputFileLength;
			ROMLoaderRst[5] = (char)b;
			ROMLoaderRst[6] = (char)(b >> 8);
			ROMLoaderRst[7] = (char)InputMemPointer[2];
			ROMLoaderRst[8] = (char)InputMemPointer[3];
			ROMLoaderRst[9] = RomCRC;
			a = (size_t)((size_t)sizeof(ROMLoaderRst) + (size_t)InputFileLength + (size_t)0x9000 - 1);
		}

		// Set binary start addresses
		AddressBuffer[0] = 0;
		AddressBuffer[1] = 0x90;
		AddressBuffer[2] = (char)a;
		AddressBuffer[3] = (char)(a >> 8);
		AddressBuffer[4] = 0;
		AddressBuffer[5] = 0x90;

		// Encode 6 bytes of addresses
		for (i = 0; i < sizeof(AddressBuffer); i++)
		{
			ch = (unsigned char)AddressBuffer[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode binary ROM loader
		if (EncodeDecode == 1)
		{
			for (i = 0; i < sizeof(ROMLoader); i++)
			{
				ch = (unsigned char)ROMLoader[i];		// Retrieve data byte
				if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
			}
		}
		else if (EncodeDecode == 3)
		{
			for (i = 0; i < sizeof(ROMLoaderRst); i++)
			{
				ch = (unsigned char)ROMLoaderRst[i];		// Retrieve data byte
				if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
			}
		}

		// Encode whole ROM data
		for (i = FileOffset; i < InputFileLength; i++)
		{
			ch = (unsigned char)InputMemPointer[i];	// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}
	}

	// Encode 32k ROM files
	else if (FileType == 3 && InputFileLength > 17000L && InputFileLength < 33000L)	// ROM file
	{
		// Calculate CRC of the first half of ROM
		RomCRC = 0;
		for (i = 0; i < (size_t)16384; i++) RomCRC = RomCRC + InputMemPointer[i];

		// Create and encode short header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

     	// Set ROM loader addresses
		ch = (unsigned char)InputMemPointer[3];
		if ((unsigned char)InputMemPointer[3] >= (unsigned char)0x40) ch = (unsigned char)0x40;
		else if ((unsigned char)InputMemPointer[3] < (unsigned char)0x40) ch = 0;

		ch = ch & 0xf0;
		a = (size_t)ch;
		a = a << 8;
		ROM32KLoader1[3] = 0;
		ROM32KLoader1[4] = (char)(a >> 8);
		b = a + (size_t)16384;
		ROM32KLoader1[5] = (char)b;
		ROM32KLoader1[6] = (char)(b >> 8);
		ROM32KLoader1[7] = (char)InputMemPointer[2];
		ROM32KLoader1[8] = (char)InputMemPointer[3];
		ROM32KLoader1[9] = RomCRC;

		// Set binary start addresses
		AddressBuffer[0] = 0;
		AddressBuffer[1] = 0x90;
		a = (size_t)((size_t)sizeof(ROM32KLoader1) + (size_t)16384 + (size_t)0x9000 - 1);
		AddressBuffer[2] = (char)a;
		AddressBuffer[3] = (char)(a >> 8);
		AddressBuffer[4] = 0;
		AddressBuffer[5] = 0x90;

		// Encode 6 bytes of addresses
		for (i = 0; i < sizeof(AddressBuffer); i++)
		{
			ch = (unsigned char)AddressBuffer[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode first binary 32k ROM loader
		for (i = 0; i < sizeof(ROM32KLoader1); i++)
		{
			ch = (unsigned char)ROM32KLoader1[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode half of 32k ROM data
		for (i = FileOffset; i < (size_t)16384; i++)
		{
			ch = (unsigned char)InputMemPointer[i];	// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode pause between files
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Calculate CRC of second half of ROM
		RomCRC = 0;
		for (i = (size_t)16384; i < InputFileLength; i++) RomCRC = RomCRC + InputMemPointer[i];

		// Create and encode long header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		// Encode binary header and second part of 32k ROM name
		for (i = 0; i < sizeof(BinFileHeaderID); i++) if (WriteDataByte(BinFileHeaderID[i])) ExitOnError(13);	// BIN/ROM ID
		for (i = 0; i < CASFILENAMELENGTH; i++) if (WriteDataByte(NameBuffer[i])) ExitOnError(13);	// File name

		// Encode second pause
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Create and encode short header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		ch = (unsigned char)InputMemPointer[3];
		if ((unsigned char)InputMemPointer[3] >= (unsigned char)0x40) ch = (unsigned char)0x40;
		else if ((unsigned char)InputMemPointer[3] < (unsigned char)0x40) ch = 0;

		// Set ROM loader addresses
		ch = ch & 0xf0;
		a = (size_t)ch;
		a = a << 8;
		a = a  + (size_t)16384;
		if (EncodeDecode == 1)
		{
			ROM32KLoader2[3] = 0;
			ROM32KLoader2[4] = (char)(a >> 8);
			b = a + InputFileLength - (size_t)16384;
			ROM32KLoader2[5] = (char)b;
			ROM32KLoader2[6] = (char)(b >> 8);
			ROM32KLoader2[7] = (char)InputMemPointer[2];
			ROM32KLoader2[8] = (char)InputMemPointer[3];
			ROM32KLoader2[9] = RomCRC;
		}
		else if (EncodeDecode == 3)
		{
			ROM32KLoader2Rst[3] = 0;
			ROM32KLoader2Rst[4] = (char)(a >> 8);
			b = a + InputFileLength - (size_t)16384;
			ROM32KLoader2Rst[5] = (char)b;
			ROM32KLoader2Rst[6] = (char)(b >> 8);
			ROM32KLoader2Rst[7] = (char)InputMemPointer[2];
			ROM32KLoader2Rst[8] = (char)InputMemPointer[3];
			ROM32KLoader2Rst[9] = RomCRC;
		}

		// Set binary start addresses
		AddressBuffer[0] = 0;
		AddressBuffer[1] = 0x90;
		if (EncodeDecode == 1) a = (size_t)((size_t)sizeof(ROM32KLoader2) + (size_t)InputFileLength - (size_t)16384 + (size_t)0x9000 - 1);
		else if (EncodeDecode == 3)  a = (size_t)((size_t)sizeof(ROM32KLoader2Rst) + (size_t)InputFileLength - (size_t)16384 + (size_t)0x9000 - 1);
		AddressBuffer[2] = (char)a;
		AddressBuffer[3] = (char)(a >> 8);
		AddressBuffer[4] = 0;
		AddressBuffer[5] = 0x90;

		// Encode 6 bytes of addresses
		for (i = 0; i < sizeof(AddressBuffer); i++)
		{
			ch = (unsigned char)AddressBuffer[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode second binary 32k ROM loader
		if (EncodeDecode == 1)
		{
			for (i = 0; i < sizeof(ROM32KLoader2); i++)
			{
				ch = (unsigned char)ROM32KLoader2[i];		// Retrieve data byte
				if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
			}
		}
		else if (EncodeDecode == 3)
		{
			for (i = 0; i < sizeof(ROM32KLoader2Rst); i++)
			{
				ch = (unsigned char)ROM32KLoader2Rst[i];		// Retrieve data byte
				if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
			}
		}

		// Encode second half of 32k ROM data
		for (i = FileOffset + (size_t)16384; i < InputFileLength; i++)
		{
			ch = (unsigned char)InputMemPointer[i];	// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}
	}

	// Encode 49k ROM files
	else if (FileType == 3 && InputFileLength >= 33000L && InputFileLength < 50000L)	// ROM file
	{
		// Calculate CRC of the first 1/3 of ROM
		RomCRC = 0;
		for (i = 0; i < (size_t)16384; i++) RomCRC = RomCRC + InputMemPointer[i];

		// Create and encode short header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

     	// Set ROM loader addresses
		ch = 0;
		ch = ch & 0xf0;
		a = (size_t)ch;
		a = a << 8;
		ROM49KLoader1[3] = 0;
		ROM49KLoader1[4] = (char)(a >> 8);
		b = a + (size_t)16384;
		ROM49KLoader1[5] = (char)b;
		ROM49KLoader1[6] = (char)(b >> 8);
		if ((unsigned char)InputMemPointer[0] == 'A' && (unsigned char)InputMemPointer[1] == 'B')
		{
			ROM49KLoader1[7] = (char)InputMemPointer[2];
			ROM49KLoader1[8] = (char)InputMemPointer[3];
		}
		else if ((unsigned char)InputMemPointer[0x4000] == 'A' && (unsigned char)InputMemPointer[0x4001] == 'B')
		{
			ROM49KLoader1[7] = (char)InputMemPointer[0x4002];
			ROM49KLoader1[8] = (char)InputMemPointer[0x4003];
		}
		ROM49KLoader1[9] = RomCRC;

		// Set binary start addresses
		AddressBuffer[0] = 0;
		AddressBuffer[1] = 0x90;
		a = (size_t)((size_t)sizeof(ROM49KLoader1) + (size_t)16384 + (size_t)0x9000 - 1);
		AddressBuffer[2] = (char)a;
		AddressBuffer[3] = (char)(a >> 8);
		AddressBuffer[4] = 0;
		AddressBuffer[5] = 0x90;

		// Encode 6 bytes of addresses
		for (i = 0; i < sizeof(AddressBuffer); i++)
		{
			ch = (unsigned char)AddressBuffer[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode first binary 49k ROM loader
		for (i = 0; i < sizeof(ROM49KLoader1); i++)
		{
			ch = (unsigned char)ROM49KLoader1[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode 1/3 of 49k ROM data
		for (i = FileOffset; i < (size_t)16384; i++)
		{
			ch = (unsigned char)InputMemPointer[i];	// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode pause between files
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Calculate CRC of 2/3rd part of ROM
		RomCRC = 0;
		for (i = (size_t)16384; i < (size_t)32768; i++) RomCRC = RomCRC + InputMemPointer[i];

		// Create and encode long header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		// Encode binary header and 2/3rd part of 49k ROM name
		for (i = 0; i < sizeof(BinFileHeaderID); i++) if (WriteDataByte(BinFileHeaderID[i])) ExitOnError(13);	// BIN/ROM ID
		for (i = 0; i < CASFILENAMELENGTH; i++) if (WriteDataByte(NameBuffer[i])) ExitOnError(13);	// File name

		// Encode second pause
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Create and encode short header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		// Set ROM loader addresses
		ch = 0;
		ch = ch & 0xf0;
		a = (size_t)ch;
		a = a << 8;
		a = a  + (size_t)16384;
		ROM49KLoader2[3] = 0;
		ROM49KLoader2[4] = (char)(a >> 8);
		b = a + (size_t)32768 - (size_t)16384;
		ROM49KLoader2[5] = (char)b;
		ROM49KLoader2[6] = (char)(b >> 8);
		if ((unsigned char)InputMemPointer[0] == 'A' && (unsigned char)InputMemPointer[1] == 'B')
		{
			ROM49KLoader2[7] = (char)InputMemPointer[2];
			ROM49KLoader2[8] = (char)InputMemPointer[3];
		}
		else if ((unsigned char)InputMemPointer[0x4000] == 'A' && (unsigned char)InputMemPointer[0x4001] == 'B')
		{
			ROM49KLoader2[7] = (char)InputMemPointer[0x4002];
			ROM49KLoader2[8] = (char)InputMemPointer[0x4003];
		}
		ROM49KLoader2[9] = RomCRC;

		// Set binary start addresses
		AddressBuffer[0] = 0;
		AddressBuffer[1] = 0x90;
		a = (size_t)((size_t)sizeof(ROM49KLoader2) + (size_t)32768 - (size_t)16384 + (size_t)0x9000 - 1);
		AddressBuffer[2] = (char)a;
		AddressBuffer[3] = (char)(a >> 8);
		AddressBuffer[4] = 0;
		AddressBuffer[5] = 0x90;

		// Encode 6 bytes of addresses
		for (i = 0; i < sizeof(AddressBuffer); i++)
		{
			ch = (unsigned char)AddressBuffer[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode second binary 49k ROM loader
		for (i = 0; i < sizeof(ROM49KLoader2); i++)
		{
			ch = (unsigned char)ROM49KLoader2[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode 2/3rd of 49k ROM data
		for (i = FileOffset + (size_t)16384; i < (size_t)32768; i++)
		{
			ch = (unsigned char)InputMemPointer[i];	// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode pause between files
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Calculate CRC of 3/3rd part of ROM
		RomCRC = 0;
		for (i = (size_t)32768; i < InputFileLength; i++) RomCRC = RomCRC + InputMemPointer[i];

		// Create and encode long header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		// Encode binary header and 3/3rd part of 49k ROM name
		for (i = 0; i < sizeof(BinFileHeaderID); i++) if (WriteDataByte(BinFileHeaderID[i])) ExitOnError(13);	// BIN/ROM ID
		for (i = 0; i < CASFILENAMELENGTH; i++) if (WriteDataByte(NameBuffer[i])) ExitOnError(13);	// File name

		// Encode second pause
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Create and encode short header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		// Set ROM loader addresses
		ch = 0;
		ch = ch & 0xf0;
		a = (size_t)ch;
		a = a << 8;
		a = a  + (size_t)32768;
		if (EncodeDecode == 1)
		{
			ROM49KLoader3[3] = 0;
			ROM49KLoader3[4] = (char)(a >> 8);
			b = a + InputFileLength - (size_t)32768;
			ROM49KLoader3[5] = (char)b;
			ROM49KLoader3[6] = (char)(b >> 8);
			if ((unsigned char)InputMemPointer[0] == 'A' && (unsigned char)InputMemPointer[1] == 'B')
			{
				ROM49KLoader3[7] = (char)InputMemPointer[2];
				ROM49KLoader3[8] = (char)InputMemPointer[3];
			}
			else if ((unsigned char)InputMemPointer[0x4000] == 'A' && (unsigned char)InputMemPointer[0x4001] == 'B')
			{
				ROM49KLoader3[7] = (char)InputMemPointer[0x4002];
				ROM49KLoader3[8] = (char)InputMemPointer[0x4003];
			}
			ROM49KLoader3[9] = RomCRC;
		}
		else if (EncodeDecode == 3)
		{
			ROM49KLoader3Rst[3] = 0;
			ROM49KLoader3Rst[4] = (char)(a >> 8);
			b = a + InputFileLength - (size_t)32768;
			ROM49KLoader3Rst[5] = (char)b;
			ROM49KLoader3Rst[6] = (char)(b >> 8);
			if ((unsigned char)InputMemPointer[0] == 'A' && (unsigned char)InputMemPointer[1] == 'B')
			{
				ROM49KLoader3Rst[7] = (char)InputMemPointer[2];
				ROM49KLoader3Rst[8] = (char)InputMemPointer[3];
			}
			else if ((unsigned char)InputMemPointer[0x4000] == 'A' && (unsigned char)InputMemPointer[0x4001] == 'B')
			{
				ROM49KLoader3Rst[7] = (char)InputMemPointer[0x4002];
				ROM49KLoader3Rst[8] = (char)InputMemPointer[0x4003];
			}
			ROM49KLoader3Rst[9] = RomCRC;
		}

		// Set binary start addresses
		AddressBuffer[0] = 0;
		AddressBuffer[1] = 0x90;
		if (EncodeDecode == 1) a = (size_t)((size_t)sizeof(ROM49KLoader3) + (size_t)InputFileLength - (size_t)32768 + (size_t)0x9000 - 1);
		else if (EncodeDecode == 3)  a = (size_t)((size_t)sizeof(ROM49KLoader3Rst) + (size_t)InputFileLength - (size_t)32768 + (size_t)0x9000 - 1);
		AddressBuffer[2] = (char)a;
		AddressBuffer[3] = (char)(a >> 8);
		AddressBuffer[4] = 0;
		AddressBuffer[5] = 0x90;

		// Encode 6 bytes of addresses
		for (i = 0; i < sizeof(AddressBuffer); i++)
		{
			ch = (unsigned char)AddressBuffer[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode third binary 49k ROM loader
		if (EncodeDecode == 1)
		{
			for (i = 0; i < sizeof(ROM49KLoader3); i++)
			{
				ch = (unsigned char)ROM49KLoader3[i];		// Retrieve data byte
				if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
			}
		}
		else if (EncodeDecode == 3)
		{
			for (i = 0; i < sizeof(ROM49KLoader3Rst); i++)
			{
				ch = (unsigned char)ROM49KLoader3Rst[i];		// Retrieve data byte
				if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
			}
		}

		// Encode 3/3rd part of 49k ROM data
		for (i = FileOffset + (size_t)32768; i < InputFileLength; i++)
		{
			ch = (unsigned char)InputMemPointer[i];	// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}
	}

	// Encode detokenized basic file
	else if (FileType == 4)
	{
		// Check for correct CR+LF
		for (i = 1; i < InputFileLength; i++)
			if ((unsigned char)InputMemPointer[i] == 0xa && (unsigned char)InputMemPointer[i-1] != 0xd) ExitOnError(24);

		if (InputFileLength <= (size_t)256) b = (size_t)InputFileLength;
			else b = 256;

		// Encode 256 bytes + header chunks of data
		for (i = FileOffset; ; i = i + 256)
		{
			// Create and encode short header
			for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
			{
				if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
			}

			// Encode 256 (or less) bytes
			for (a = 0; a < b; a++)
			{
				ch = (unsigned char)InputMemPointer[i + a];	// Retrieve data byte
				if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
			}

			// Check if whole file is written
			if (InputFileLength <= 256) break;
			InputFileLength = InputFileLength - 256;
			if (InputFileLength <= (size_t)256) b = (size_t)InputFileLength;
		}
	}


	// Encode finalize areas
	if (FileType == 1)
	{
		for (i = 0; i < 7; i++) if (WriteDataByte(0)) ExitOnError(13);	// Encode 7 zeros at the end of basic program
	}
	else if (FileType == 4)
	{
		for (i = 0; i < 256; i++) if (WriteDataByte(0x1a)) ExitOnError(13);	// Encode 256 bytes at the end of ascii program
	}


	// Encode final pause
	for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)FinalPauseLength + LENGTHCORRECTION); j++)
	{
		if (WriteServiceByte(SilenceByte)) ExitOnError(13);
	}

	// Add copyright message to WAV file
	fwrite(&szWavCopyrMsg, sizeof(unsigned char), sizeof(szWavCopyrMsg), OutputFileHandle);

	// Convert to 16 bit, stereo, 44 kHz
	if (ConvertFlag == 1)
	{
		if (ConvertToCDQuality() != 0) ExitOnError(18);
	}
}


// Encode multiple input files to WAV sound file routine -----------------------

void EncodeMultipleFiles(int k)
{
	struct {
		char FileType;
		size_t ExtraBytes;
		size_t MoreExtraBytes;
		size_t FileOffset;
		unsigned long FileLength;
	} FileQueue[22];
	int l;

	// Collect information about all input files
	if (k > 22) k = 22;
	for (l = 2; l < k; l++)
	{
		// Open, check size, allocate memory and read 10 bytes of input file
		if (fopen_s(&InputFileHandle, FileNamePointer[l], "rb")) ExitOnError(6);
		if (fseek(InputFileHandle, 0, SEEK_END)) ExitOnError(7);
		InputFileLength = (size_t)ftell(InputFileHandle);
		if (fseek(InputFileHandle, 0, SEEK_SET)) ExitOnError(7);
		if (InputFileLength < (long)MINENCINPUTFILELENGTH || InputFileLength > (long)MAXMENCINPUTFILELENGTH) ExitOnError(23);
		InputMemPointer = (unsigned char*) malloc(10);
		if (!InputMemPointer) ExitOnError(5);
		BytesRead = fread(InputMemPointer, sizeof(unsigned char), 10, InputFileHandle);
		if (BytesRead != 10) ExitOnError(9);
		if ((unsigned char)InputMemPointer[0] == 'A' && (unsigned char)InputMemPointer[1] == 'B') ExitOnError(15);
		else if ((unsigned char)InputMemPointer[0] == 0xfe)
		{
			FileQueue[l-2].FileType = 2; // BIN program
			if (FileQueue[0].FileType != 1 && FileQueue[0].FileType != 4)	// no basic program is loaded first
			{
				FileQueue[l-2].ExtraBytes = sizeof(MultFileLoader);
				FileQueue[l-2].FileOffset = 7;
				FileQueue[l-2].MoreExtraBytes = 1;
			}
			else
			{
				FileQueue[l-2].ExtraBytes = 0;
				FileQueue[l-2].FileOffset = 1;
				FileQueue[l-2].MoreExtraBytes = 1;
			}
		}
		else
		{
			if ((unsigned char)InputMemPointer[0] == 0xff)
			{
				FileQueue[l-2].FileType = 1; // tokenized Basic program
				FileQueue[l-2].ExtraBytes = 7;
				FileQueue[l-2].FileOffset = 1;
				FileQueue[l-2].MoreExtraBytes = 1;
			}
			else
			{
				FileQueue[l-2].FileType = 4;	// detokenized Basic program
				FileQueue[l-2].ExtraBytes = 256;
				FileQueue[l-2].FileOffset = 0;
				FileQueue[l-2].MoreExtraBytes = 1;
				if (InputFileLength > 256) FileQueue[l-2].MoreExtraBytes = InputFileLength / 256 + 1; // calculate extra bytes (extra short headers) for detokenized program
			}
			if (FileQueue[0].FileType != 1 && FileQueue[0].FileType != 4) ExitOnError(17);
		}

		// Cleanup before the next file is processed
		FileQueue[l-2].FileLength = (unsigned long)InputFileLength;	 // Save length
		fclose(InputFileHandle);
		free(InputMemPointer);
		InputMemPointer = 0;
		InputFileHandle = 0;
	}

	// Add preloader size correction in case only BIN/ROM files are loaded
	if (FileQueue[0].FileType == 2 || FileQueue[0].FileType == 3)
		FileQueue[0].ExtraBytes = FileQueue[0].ExtraBytes + (DWORD)sizeof(BinFileHeaderID) + (DWORD)CASFILENAMELENGTH +	(DWORD)sizeof(BINROMPreLoader) + (DWORD)6;

   // Fill WAV header
	WavHeader.WavHeaderID[0] = 'R';
	WavHeader.WavHeaderID[1] = 'I';
	WavHeader.WavHeaderID[2] = 'F';
	WavHeader.WavHeaderID[3] = 'F';
	WavHeader.MMFileType[0] = 'W';
	WavHeader.MMFileType[1] = 'A';
	WavHeader.MMFileType[2] = 'V';
	WavHeader.MMFileType[3] = 'E';
	WavHeader.FormatID[0] = 'f';
	WavHeader.FormatID[1] = 'm';
	WavHeader.FormatID[2] = 't';
	WavHeader.FormatID[3] = ' ';
	WavHeader.DataID[0] = 'd';
	WavHeader.DataID[1] = 'a';
	WavHeader.DataID[2] = 't';
	WavHeader.DataID[3] = 'a';
	WavHeader.BlkAllign = (WORD) 1;
	WavHeader.FormatTag = (WORD) 1;	// PCM
	WavHeader.NumChannels = (WORD) 1; // mono always
	WavHeader.BitsPerSample = (WORD) 8;	// 8 bit always
	WavHeader.SamplesPerSec = (DWORD)WavSampleRate;
	WavHeader.BytesPerSec = (DWORD)((DWORD)WavSampleRate * (DWORD)(WavHeader.BitsPerSample / 8) * (DWORD)WavHeader.NumChannels);
	WavHeader.FormatSize = (DWORD)(sizeof(WavHeader.FormatTag) + sizeof(WavHeader.NumChannels) + sizeof(WavHeader.BitsPerSample) +
   	sizeof(WavHeader.SamplesPerSec) + sizeof(WavHeader.BytesPerSec) + sizeof(WavHeader.BlkAllign));
	WavHeader.PureSampleLength = 0;
	for (l = 2; l < k; l++)
	{
		if (FileQueue[l-2].FileType == 1) FileHeaderIDLength = sizeof(BasicFileHeaderID);
		else if (FileQueue[l-2].FileType == 4) FileHeaderIDLength = sizeof(DataFileHeaderID);
		else FileHeaderIDLength = sizeof(BinFileHeaderID);

		WavHeader.PureSampleLength = WavHeader.PureSampleLength + (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength + (DWORD)WavSampleRate * (DWORD)MiddlePauseLength) +	// Length of pauses
		(DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength * (DWORD)FileQueue[l-2].MoreExtraBytes) +	// Length of long and short headers
		(DWORD)(((DWORD)FileHeaderIDLength + (DWORD)CASFILENAMELENGTH + (DWORD)FileQueue[l-2].FileLength + (DWORD)FileQueue[l-2].ExtraBytes - (DWORD)FileQueue[l-2].FileOffset) * (DWORD)SampleScale * (DWORD)sizeof(BitStream) * (DWORD)BitEncodingLength);	// Length of data areas
	}

	// Add preloader pauses and headers
	if (FileQueue[0].FileType == 2 || FileQueue[0].FileType == 3)
	{
		WavHeader.PureSampleLength = WavHeader.PureSampleLength + (DWORD)WavSampleRate * (DWORD)FirstPauseLength + (DWORD)WavSampleRate * (DWORD)MiddlePauseLength +
     	(DWORD)WavSampleRate * (DWORD)LongHeaderLength + (DWORD)WavSampleRate * (DWORD)ShortHeaderLength;
	}

	// Add final pause
	WavHeader.PureSampleLength = WavHeader.PureSampleLength + (DWORD)((DWORD)WavSampleRate * (DWORD)FinalPauseLength);
	WavHeader.SampleLength = (DWORD)((DWORD)WavHeader.PureSampleLength + (DWORD)WAVHEADEROFFSETVALUE);

	// Check if file already exists
	if (!fopen_s(&OutputFileHandle, OutputFileName, "rb"))
	{
		SetConsoleTextAttribute(hStdOut, 0x0e);
		WriteConsole(hStdOut, szOverwrWarning, (DWORD) strlen(szOverwrWarning), &Result, NULL);
		SetConsoleTextAttribute(hStdOut, 0x0f);
		ch = 'A';
		while (toupper(ch) != 'Y')
		{
			ch = _getch();
			if (toupper(ch) == 'N') ExitOnError(11);
		}
		wsprintf(Buffer, TEXT("%c\n"), toupper(ch));
		WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);	// print selection
		fclose(OutputFileHandle);
		OutputFileHandle = 0;
	}
	wsprintf(Buffer, szCreateOutFile, OutputFileName);
	WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);

	// Write output file header (WAV)
	if (fopen_s(&OutputFileHandle, OutputFileName, "wb")) ExitOnError(12);
	BytesWritten = fwrite(&WavHeader, sizeof(unsigned char), sizeof(WavHeader), OutputFileHandle);
	if (BytesWritten != sizeof(WavHeader)) ExitOnError(13);


	// Encode preloader if only BIN files are loaded
	if (FileQueue[0].FileType == 2 || FileQueue[0].FileType == 3)
	{
		// Prepare file name for tape header and preloader
		for (i = 0; i < CASFILENAMELENGTH; i++)
		{
			if (InputFileName[i] != '.' && InputFileName[i]) NameBuffer[i] = InputFileName[i];
			else break;
		}
		for ( ; i < CASFILENAMELENGTH; i++) NameBuffer[i] = ' ';
		for (i = 0; i < CASFILENAMELENGTH; i++)
		{
			(unsigned char)BINROMPreLoader[i + 0x0e] = (unsigned char)NameBuffer[i];	// Patch preloader for BIN/ROM files
		}

		// Encode first pause
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Create and encode long header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}
	
		// Encode preloader header data
   		for (i = 0; i < sizeof(BinFileHeaderID); i++) if (WriteDataByte(BinFileHeaderID[i])) ExitOnError(13);	// BIN/ROM ID
		
		// Encode file name
		for (i = 0; i < CASFILENAMELENGTH; i++)
		{
			if (WriteDataByte(NameBuffer[i])) ExitOnError(13);	// File name
		}

		// Encode second pause
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Create and encode short header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		// Set binary start addresses for preloader
		AddressBuffer[0] = 0;
		AddressBuffer[1] = 0x90;
		a = (size_t)((size_t)sizeof(BINROMPreLoader) + (size_t)0x9000 - 1);
		AddressBuffer[2] = (char)a;
		AddressBuffer[3] = (char)(a >> 8);
		AddressBuffer[4] = 0;
		AddressBuffer[5] = 0x90;

		// Encode 6 bytes of addresses
		for (i = 0; i < sizeof(AddressBuffer); i++)
		{
			ch = (unsigned char)AddressBuffer[i];		// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}

		// Encode preloader data
		for (i = 0; i < sizeof(BINROMPreLoader); i++)
		{
			ch = (unsigned char)BINROMPreLoader[i];	// Retrieve data byte
			if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
		}
	}

	// Encode all files
	for (l = 2; l < k; l++)
	{
		// Print file name and format
		wsprintf(Buffer, TEXT("\n  [ %s ]: File format - %s"), FileNamePointer[l], FileFormat[FileQueue[l-2].FileType - 1]);
		SetConsoleTextAttribute(hStdOut, 0x08);
		WriteConsole(hStdOut, Buffer, (DWORD) strlen(Buffer), &Result, NULL);
		SetConsoleTextAttribute(hStdOut, 0x0f);

		// Encode first pause
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)FirstPauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Create and encode long header
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)LongHeaderLength / (DWORD)BitEncodingLength); j++)
		{
			if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
		}

		// Encode file header data
		if (FileQueue[l-2].FileType == 1)
		{
            for (i = 0; i < sizeof(BasicFileHeaderID); i++) if (WriteDataByte(BasicFileHeaderID[i])) ExitOnError(13);	// Basic ID
		}
		else if (FileQueue[l-2].FileType == 4)
		{
			for (i = 0; i < sizeof(DataFileHeaderID); i++) if (WriteDataByte(DataFileHeaderID[i])) ExitOnError(13);	// Data ID
		}
		else
		{
			for (i = 0; i < sizeof(BinFileHeaderID); i++) if (WriteDataByte(BinFileHeaderID[i])) ExitOnError(13);	// BIN/ROM ID
		}

		// Prepare file name for tape header
		for (i = 0; i < CASFILENAMELENGTH; i++)
		{
			if (InputFileName[i] != '.' && InputFileName[i]) NameBuffer[i] = InputFileName[i];
			else break;
		}
		for ( ; i < CASFILENAMELENGTH; i++) NameBuffer[i] = ' ';

		// Increment part number in the loader
		if ((unsigned char)MultFileLoader[0x22] != ' ') (unsigned char)MultFileLoader[0x22] = (unsigned char)MultFileLoader[0x22] + 1;	// increment part number 10+
		else if ((unsigned char)MultFileLoader[0x21] == '9')
		{
			(unsigned char)MultFileLoader[0x21] = '1';
			(unsigned char)MultFileLoader[0x22] = '0';	// part number = 10
		}
		else (unsigned char)MultFileLoader[0x21] = (unsigned char)MultFileLoader[0x21] + 1;	// increment part number

		// Encode file name
		for (i = 0; i < CASFILENAMELENGTH; i++)
		{
			if (WriteDataByte(NameBuffer[i])) ExitOnError(13);	// File name
			(unsigned char)MultFileLoader[i + 0x15] = (unsigned char)NameBuffer[i];	// copy name to loader
		}

	   	// Encode second pause
		for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)MiddlePauseLength); j++)
		{
			if (WriteServiceByte(SilenceByte)) ExitOnError(13);
		}

		// Open, check size, allocate memory and read input file
		if (fopen_s(&InputFileHandle, FileNamePointer[l], "rb")) ExitOnError(6);
		if (fseek(InputFileHandle, 0, SEEK_END)) ExitOnError(7);
		InputFileLength = (size_t)ftell(InputFileHandle);
		if (fseek(InputFileHandle, 0, SEEK_SET)) ExitOnError(7);
		if (InputFileLength < (long)MINENCINPUTFILELENGTH || InputFileLength > (long)MAXMENCINPUTFILELENGTH) ExitOnError(23);
		InputMemPointer = (unsigned char*) malloc(InputFileLength);
		if (!InputMemPointer) ExitOnError(5);
   		BytesRead = fread(InputMemPointer, sizeof(unsigned char), InputFileLength, InputFileHandle);
		if (BytesRead != InputFileLength) ExitOnError(9);

		// Encode detokenized basic file
		if (FileQueue[l-2].FileType == 4)
		{
			// Check for correct CR+LF
			for (i = 1; i < InputFileLength; i++)
				if ((unsigned char)InputMemPointer[i] == 0xa && (unsigned char)InputMemPointer[i-1] != 0xd) ExitOnError(24);

			if (FileQueue[l-2].FileLength <= (size_t)256) b = (size_t)FileQueue[l-2].FileLength;
			else b = 256;

			// Encode 256 bytes + header chunks of data
			for (i = FileQueue[l-2].FileOffset; ; i = i + 256)
			{
				// Create and encode short header
				for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
				{
					if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
				}

				// Encode 256 (or less) bytes
				for (a = 0; a < b; a++)
				{
					ch = (unsigned char)InputMemPointer[i + a];	// Retrieve data byte
					if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
				}

				// Check if whole file is written
				if (FileQueue[l-2].FileLength <= 256) break;
				FileQueue[l-2].FileLength = FileQueue[l-2].FileLength - 256;
				if (FileQueue[l-2].FileLength <= (size_t)256) b = (size_t)FileQueue[l-2].FileLength;
			}
		}

		// Encode tokenized basic file
		else if (FileQueue[l-2].FileType == 1)
		{
			// Create and encode short header
			for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
			{
				if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
			}

			// Encode whole program data
			for (i = 1; i < FileQueue[l-2].FileLength; i++)
			{
				ch = (unsigned char)InputMemPointer[i];	// Retrieve data byte
				if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
			}
		}

		// Encode binary file
		else
		{
			// Create and encode short header
			for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)ShortHeaderLength / (DWORD)BitEncodingLength); j++)
			{
				if (WriteHeaderByte(SetBit[SpeedScale])) ExitOnError(13);
			}

			// Calculate CRC of the whole BIN file excluding the header
			RomCRC = 0;
			RomBegin = (unsigned short)((unsigned char)InputMemPointer[2] * 0x100 + (unsigned char)InputMemPointer[1]);
			RomEnd = (unsigned short)((unsigned char)InputMemPointer[4] * 0x100 + (unsigned char)InputMemPointer[3]);
			RomStart = (unsigned short)((unsigned char)InputMemPointer[6] * 0x100 + (unsigned char)InputMemPointer[5]);
			if (RomEnd <= RomBegin || RomStart >= RomEnd || RomStart < RomBegin) ExitOnError(25);
			for (i = FileQueue[l-2].FileOffset; i < (RomEnd-RomBegin) + FileQueue[l-2].FileOffset; i++) RomCRC = RomCRC + InputMemPointer[i];

			// Copy BIN start addresses and CRC to loader
			for (j = 0; j < 6; j++) MultFileLoader[j + 3] = InputMemPointer[j + 1];
			MultFileLoader[9] = RomCRC;
			if (l == (k - 1)) for (j = 0; j < 0x55; j++) MultFileLoader[j + 0xe2] = '\0';	// patch loader if last file is output
																							// !!! WARNING !!! when modifying the multi file loader, make sure you patch at the correct address and with the correct ammount!

			if (FileQueue[0].FileType == 2 || FileQueue[0].FileType == 3)	// no basic program is loaded first
			{
				// Set binary start addresses
				AddressBuffer[0] = 0;
				AddressBuffer[1] = 0x82;
				a = (size_t)((size_t)sizeof(MultFileLoader) + (size_t)FileQueue[l-2].FileLength + (size_t)0x8200 - FileQueue[l-2].FileOffset - 1);
				AddressBuffer[2] = (char)a;
				AddressBuffer[3] = (char)(a >> 8);
				AddressBuffer[4] = 0;
				AddressBuffer[5] = 0x82;

				// Encode 6 bytes of addresses
				for (i = 0; i < sizeof(AddressBuffer); i++)
				{
					ch = (unsigned char)AddressBuffer[i];		// Retrieve data byte
					if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
				}

				// Encode binary ROM loader
				for (i = 0; i < sizeof(MultFileLoader); i++)
				{
					ch = (unsigned char)MultFileLoader[i];		// Retrieve data byte
					if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
				}
			}

			// Encode whole BIN file from specified offset
			for (i = FileQueue[l-2].FileOffset; i < FileQueue[l-2].FileLength; i++)
			{
				ch = (unsigned char)InputMemPointer[i];	// Retrieve data byte
				if (WriteDataByte(ch)) ExitOnError(13);	// Encode data byte
			}
		}

		// Encode finalize areas
		if (FileQueue[l-2].FileType == 1)
		{
			for (i = 0; i < 7; i++) if (WriteDataByte(0)) ExitOnError(13);	// Encode 7 zeros at the end of Basic program
		}
		else if (FileQueue[l-2].FileType == 4)
		{
			for (i = 0; i < 256; i++) if (WriteDataByte(0x1a)) ExitOnError(13);	// Encode 256 bytes at the end of date/ascii file
		}

		// Cleanup before the next file is processed
		fclose(InputFileHandle);
		free(InputMemPointer);
		InputMemPointer = 0;
		InputFileHandle = 0;
	}

	// Encode final pause
	for (j = 0; j < (DWORD)((DWORD)WavSampleRate * (DWORD)FinalPauseLength + LENGTHCORRECTION); j++)
	{
		if (WriteServiceByte(SilenceByte)) ExitOnError(13);
	}

	// Add copyright message to WAV file
	fwrite(&szWavCopyrMsg, sizeof(unsigned char), sizeof(szWavCopyrMsg), OutputFileHandle);

	WriteConsole(hStdOut, "\n", (DWORD) strlen("\n"), &Result, NULL);

	// Convert to 16 bit, stereo, 44 kHz
	if (ConvertFlag == 1)
	{
		if (ConvertToCDQuality() != 0) ExitOnError(18);
	}
}



// Supplementary routines ------------------------------------------------------
// -----------------------------------------------------------------------------
// Abnormal termination routine ------------------------------------------------
void ExitOnError(int ErrorNum)
{
	// Clean up
	CleanUp();

	// Print error message and exit
	if (!ErrorNum) exit(255);
	WriteConsole(hStdOut, "\n\n", (DWORD) strlen("\n\n"), &Result, NULL);
	SetConsoleTextAttribute(hStdOut, 0x0c);
	WriteConsole(hStdOut, ErrorList[ErrorNum-1], (DWORD) strlen(ErrorList[ErrorNum-1]), &Result, NULL);
	SetConsoleTextAttribute(hStdOut, 0x0f);
	exit(ErrorNum);
}



// Clean up routine ------------------------------------------------------------
void CleanUp(void)
{
	if (InputMemPointer)
	{
		free(InputMemPointer);
		InputMemPointer = 0;
	}
	if (InputMemPointer16)
	{
		free(InputMemPointer16);
		InputMemPointer16 = 0;
	}
	if (OutputMemPointer)
	{
		free(OutputMemPointer);
		OutputMemPointer = 0;
	}
	if (InputFileHandle)
	{
		fclose(InputFileHandle);
		InputFileHandle = 0;
	}
	if (OutputFileHandle)
	{
		fclose(OutputFileHandle);
		OutputFileHandle = 0;
	}
	if (TmpFileHandle)
	{
		fclose(TmpFileHandle);
		TmpFileHandle = 0;
	}
	if (BatFileHandle)
	{
		fclose(BatFileHandle);
		BatFileHandle = 0;
	}
}

// Make primary preparations
void PrimarySetup(void)
{
	int i;

	// Initialize variables
	LongHeaderLength = 7;			// was 20/3;
	ShortHeaderLength = 3;			// was 5/3;
	FirstPauseLength = 2;			// Seconds
	MiddlePauseLength = 1;			// Seconds
	FinalPauseLength = 1;			// Seconds
	SilenceByte = SILENCE;
	WavSampleRate = 11025;

	// Prepare scales and bit patterns according to volume
	SampleScale = WavSampleRate / 11025;   // sample scale for different sampling rates
	SpeedScale = MSXInterfaceSpeed / 1200 - 1;	// speed scale for MSX interface
	if (!SpeedScale) BitEncodingLength = 10;
	else BitEncodingLength = 5;
	
	for (i = 0; i < BitEncodingLength; i++)
	{
		if (ZeroBit[SpeedScale][i] == (unsigned char)LOWAMPLITUDE) ZeroBit[SpeedScale][i] = (unsigned char)(LOWAMPLITUDE + (MAXSIGNALLEVEL - SignalLevel));
		else if (ZeroBit[SpeedScale][i] == (unsigned char)HIGHAMPLITUDE) ZeroBit[SpeedScale][i] = (unsigned char)(HIGHAMPLITUDE - (MAXSIGNALLEVEL - SignalLevel));
		if (SetBit[SpeedScale][i] == (unsigned char)LOWAMPLITUDE) SetBit[SpeedScale][i] = (unsigned char)(LOWAMPLITUDE + (MAXSIGNALLEVEL - SignalLevel));
		else if (SetBit[SpeedScale][i] == (unsigned char)HIGHAMPLITUDE) SetBit[SpeedScale][i] = (unsigned char)(HIGHAMPLITUDE - (MAXSIGNALLEVEL - SignalLevel));
	}

	// Allocate memory for output buffer
	OutputMemPointer = (unsigned char*) malloc((size_t)(BitEncodingLength * sizeof(BitStream)));
	if (!OutputMemPointer) ExitOnError(4);
}

// Encode and write data byte to WAV file --------------------------------------
int WriteDataByte(unsigned char ch)
{
	int i, j;
	char BitMask = 1;

	BitStream[0] =	(unsigned char)STARTBIT;
	BitStream[9] =	(unsigned char)STOPBIT;	// Fill start and stopbits
	BitStream[10] =	(unsigned char)STOPBIT;
	for (i = 1; i < 9; i++)
	{
		if (ch & BitMask) BitStream[i] = 1;	// Fill bit stream
		else BitStream[i] = 0;
		BitMask = BitMask << 1;
	}
	for (i = 0; i < sizeof(BitStream); i++)
	{
		if ((unsigned char)BitStream[i] == 0) for (j = 0; j < BitEncodingLength; j++) (unsigned char)OutputMemPointer[i * BitEncodingLength + j] = ZeroBit[SpeedScale][j];
		else for (j = 0; j < BitEncodingLength; j++) (unsigned char)OutputMemPointer[i * BitEncodingLength + j] = SetBit[SpeedScale][j];
	}

	// Save one encoded byte to file in 8 bit, mono
	OutputFileLength = (size_t)(BitEncodingLength * sizeof(BitStream));
	BytesWritten = fwrite(OutputMemPointer, 1, OutputFileLength, OutputFileHandle);
	if (BytesWritten != OutputFileLength) return 1;
	return 0;
}


// Encode and write service byte to WAV file --------------------------------------
int WriteServiceByte(unsigned char ch)
{
	*Buffer = ch;
	*(Buffer + 1) = '\0';

	// Save one encoded byte to file in 8 bit, mono
	BytesWritten = fwrite(Buffer, 1, 1, OutputFileHandle);
	if (BytesWritten != 1) return 1;
	return 0;
}


// Encode and write header byte to WAV file --------------------------------------
int WriteHeaderByte(unsigned char* Buffer)
{
	// Save one encoded byte to file in 8 bit, mono
	BytesWritten = fwrite(Buffer, 1, BitEncodingLength, OutputFileHandle);
	if (BytesWritten != BitEncodingLength) return 1;
	return 0;
}


// Convert WAV file to 16 bit, stereo, 44 kHz -------------------------------------
int ConvertToCDQuality(void)
{
	size_t	i;
	unsigned char ch, j;
	unsigned short ch1;

	// Close all files and free memory blocks
	CleanUp();

	// Prepare temporary name
	WriteConsole(hStdOut, szConvertToCD, (DWORD) strlen(szConvertToCD), &Result, NULL);
	strcpy_s(TmpFileName, 30, OutputFileName);
	strcat_s(TmpFileName, 30, ".tmp");
	
	// Open and read file
	if (fopen_s(&InputFileHandle, OutputFileName, "rb")) ExitOnError(18); // WAV file to convert
	if (fseek(InputFileHandle, 0, SEEK_END)) ExitOnError(18);
	InputFileLength = (size_t)ftell(InputFileHandle);
	if (fseek(InputFileHandle, 0, SEEK_SET)) ExitOnError(18);
	InputMemPointer = (unsigned char*) malloc(InputFileLength);
	if (!InputMemPointer) ExitOnError(18);

	// Read header
	BytesRead = fread(&WavHeader, sizeof(unsigned char), sizeof(WavHeader), InputFileHandle);
	if (BytesRead != sizeof(WavHeader)) ExitOnError(18);

	// Read the rest of file
	BytesRead = fread(InputMemPointer, sizeof(unsigned char), InputFileLength - sizeof(WavHeader), InputFileHandle);
	if (BytesRead != InputFileLength - sizeof(WavHeader)) ExitOnError(18);
	OutputFileLength = (InputFileLength - sizeof(WavHeader) - sizeof(szWavCopyrMsg));

	// Create new WAV file
	if (fopen_s(&OutputFileHandle, TmpFileName, "wb")) ExitOnError(18);

	// Correct header
	WavHeader.BlkAllign = 4;
	WavHeader.FormatTag = (WORD) 1;				// PCM
	WavHeader.NumChannels = (WORD) 2;			// Stereo
	WavHeader.BitsPerSample = (WORD) 16;		// 16 bit
	WavHeader.SamplesPerSec = (DWORD)44100;		// 44 kHz
	WavHeader.BytesPerSec = WavHeader.BytesPerSec * ConvertRatio * 2 * 2;	// 44 kHz, stereo, 2 channels	
	WavHeader.PureSampleLength = WavHeader.PureSampleLength * ConvertRatio * 2 * 2;
	WavHeader.SampleLength = WavHeader.SampleLength * ConvertRatio * 2 * 2;

	// Write new header
	BytesWritten = fwrite(&WavHeader, sizeof(unsigned char), sizeof(WavHeader), OutputFileHandle);
	if (BytesWritten != sizeof(WavHeader)) ExitOnError(18);

	// Convert to 16 bit, stereo, 44 kHz
	for (i = 0; i < OutputFileLength; i++)
	{
		ch = (unsigned char)InputMemPointer[i];
		if (InvertedWaveform == 1)				// 32767 (0x7FFF) - highest value,  -32768 (0x8000) - lowest value
		{
			if (ch == 0x80) ch1 = 0;			// inverted, for iPOD and CD
			else if (ch < 0x80) ch1 = 0x7FFF;
			else ch1 = 0x8000;
		}
		else
		{
			if (ch == 0x80) ch1 = 0;			// normal, for Creative
			else if (ch < 0x80) ch1 = 0x8000;
			else ch1 = 0x7FFF;
		}

		for (j = 0; j < ConvertRatio; j++)
		{
			BytesWritten = fwrite(&ch1, 1, 2, OutputFileHandle); // channel 1
			if (BytesWritten != 2) ExitOnError(18);
			BytesWritten = fwrite(&ch1, 1, 2, OutputFileHandle); // channel 2
			if (BytesWritten != 2) ExitOnError(18);
		}
	}

	// Add copyright message to WAV file
	fwrite(&szWavCopyrMsg, sizeof(unsigned char), sizeof(szWavCopyrMsg), OutputFileHandle);

	// Close all files and free memory blocks
	CleanUp();

	if (!DeleteFile(OutputFileName)) ExitOnError(18);
	if (!MoveFile(TmpFileName, OutputFileName)) ExitOnError(18);

	return 0;
}


// Covnert created WAV file to MP3 if possible -------------------------------------
void ConvertOutputToMP3(void)
{
	// Close all files and free memory blocks
	CleanUp();

	// Check that both lame.exe and lame_enc.dll are available
	if (fopen_s(&InputFileHandle, szLameEXE, "rb"))
	{
		SetConsoleTextAttribute(hStdOut, 0x0e);
		WriteConsole(hStdOut, szNoLameFiles, (DWORD) strlen(szNoLameFiles), &Result, NULL);	// print warning - no MP3 encoder
		SetConsoleTextAttribute(hStdOut, 0x0f);
		return;
	}
	fclose(InputFileHandle);
	if (fopen_s(&InputFileHandle, szLameDLL, "rb"))
	{
		SetConsoleTextAttribute(hStdOut, 0x0e);
		WriteConsole(hStdOut, szNoLameFiles, (DWORD) strlen(szNoLameFiles), &Result, NULL);	// print warning - no MP3 encoder
		SetConsoleTextAttribute(hStdOut, 0x0f);
		return;
	}
	fclose(InputFileHandle);

	// If low quality file was created, preserve it and convert to CD quality
	if (ConvertFlag == 0)
	{
		strcpy_s(TmpFileName, 30, OutputFileName);
		strcat_s(TmpFileName, 30, ".bak");
		if (!CopyFile(OutputFileName, TmpFileName, FALSE)) ExitOnError(22);
		if (ConvertToCDQuality() != 0)
		{
			DeleteFile(OutputFileName);
			MoveFile(TmpFileName, OutputFileName);
			ExitOnError(22);
		}
	}

	// Prepare for MP3 conversion
	WriteConsole(hStdOut, szCreateMP3, (DWORD) strlen(szCreateMP3), &Result, NULL);	// print converting message
	for (i = 0; i < 12; i++)
	{
		if (OutputFileName[i] == '.' || !OutputFileName[i]) break;
		TmpFileName[i] = OutputFileName[i];
	}
	TmpFileName[i] = 0;
	i = 0;
	if (TmpFileName[0] >= 'a')
	{
		TmpFileName[0] = toupper(TmpFileName[0]);	// uppercase first letter
		i = 1;
	}

	// Prepare command line for Lame encoder
	if (ConvertRatio == 3) j = 3000;
	else if (ConvertRatio == 2) j = 2400;
	else j = MSXInterfaceSpeed;

	wsprintf(Buffer, BatCmdLine, MP3Kbps, TmpFileName, j, MP3Kbps, TmpFileName, TmpFileName);	// fill buffer with data for batch file
	if (i == 1) TmpFileName[0] = tolower(TmpFileName[0]);	// lowercase first letter
	strcpy_s(MP3FileName, 30, TmpFileName);
	strcat_s(MP3FileName, 30, ".mp3");
	DeleteFile(MP3FileName);
	strcpy_s(BatFileName, 30, TmpFileName);
	strcat_s(BatFileName, 30, ".bat");
	DeleteFile(BatFileName);
	
	// Create batch file for conversion
	if (fopen_s(&BatFileHandle, BatFileName, "wb"))
	{
		if (ConvertFlag == 0)
		{
			strcpy_s(TmpFileName, 30, OutputFileName);
			strcat_s(TmpFileName, 30, ".bak");
			DeleteFile(OutputFileName);
			MoveFile(TmpFileName, OutputFileName);
		}
		ExitOnError(22);
	}
	BytesWritten = fwrite(Buffer, sizeof(unsigned char), sizeof(Buffer), BatFileHandle);
	if (BytesWritten != sizeof(Buffer))
	{
		if (ConvertFlag == 0)
		{
			strcpy_s(TmpFileName, 30, OutputFileName);
			strcat_s(TmpFileName, 30, ".bak");
			DeleteFile(OutputFileName);
			MoveFile(TmpFileName, OutputFileName);
		}
		ExitOnError(22);
	}
	fclose(BatFileHandle);

	// Run batch file to convert to MP3
	ShellExecute(NULL, "open", BatFileName, NULL, NULL, SW_HIDE);
	
	// Wait for the operation to complete
	Sleep(3000);
	WriteConsole(hStdOut, " ", (DWORD) 1, &Result, NULL);
	while (fopen_s(&TmpFileHandle, MP3FileName, "r+b"))
	{
			Sleep(1000);
			WriteConsole(hStdOut, ".", (DWORD) 1, &Result, NULL);
	}
	fclose(TmpFileHandle);
	WriteConsole(hStdOut, "\n ", (DWORD) sizeof("\n "), &Result, NULL);
	DeleteFile(BatFileName);

	// Rename old low quality file back
	if (ConvertFlag == 0)
	{
		strcpy_s(TmpFileName, 30, OutputFileName);
		strcat_s(TmpFileName, 30, ".bak");
		DeleteFile(OutputFileName);
		MoveFile(TmpFileName, OutputFileName);
	}
	return;
}

