/*	testSound.cpp - Client for testing sound recording and playback for Windows
 *
 * Copyright 2019 Caleb Hoeksema
 *
 * History:
 *		November 01, 2019: File created
 */
 /*

 #include <stdio.h>
 #include <windows.h>					// Contains WAVEFORMATEX structure
 #include <mmsystem.h>
 #include <math.h>
 #include "sound.h"*/


 /*	Filename: main.cpp
 Author: Michael Galle
 Date: 2019
 Details: Testing mainline for Windows sound API
 */
#include "OperatingSystem.h"


#include "sound.h"
#include <stdio.h>
#include <windows.h>
#include "Frame.h"
#include "queues.h"
#include "menu.h"


int	sound_interface() {

	char buf[sizeof(MssgAud)];
	
	short* iBigBufNew = (short*)malloc(lBigBufSize * sizeof(short));		// buffer used for reading recorded sound from file
	char save;
	char replay;
	char c;																// used to flush extra input
	FILE* f;


	InitializePlayback();
	InitializeRecording();


	// start recording
	RecordBuffer(iBigBuf, lBigBufSize);
	CloseRecording();

	memcpy_s(buf, sizeof(buf), AppendAudio(iBigBuf), lBigBufSize);	// Append the header and payload to a char buffer to send;
	memcpy_s(cvtA.convert, sizeof(cvtA.convert), buf, sizeof(buf));

	// playback recording 
	printf("\nPlaying recording from buffer\n");
	PlayBuffer((short*)cvtA.convert, lBigBufSize);
	ClosePlayback();

	// save audio recording  
	printf("Would you like to save your audio recording? (y/n): ");
	scanf_s("%c", &save, 1);
	while ((c = getchar()) != '\n' && c != EOF) {}								// Flush other input
	if ((save == 'y') || (save == 'Y')) {
		/* Open input file */
		fopen_s(&f, "C:\\Users\\Greg\\Rec\\rec.dat", "wb");
		if (!f) {
			printf("unable to open %s\n", "C:\\myfiles\\recording.dat");
			return 0;
		}
		printf("Writing to sound file ...\n");
		fwrite(iBigBuf, sizeof(short), lBigBufSize, f);
		fclose(f);
	}

	// replay audio recording from file -- read and store in buffer, then use playback() to play it
	printf("Would you like to replay the saved audio recording from the file? (y/n): ");
	scanf_s("%c", &replay, 1);
	while ((c = getchar()) != '\n' && c != EOF) {}								// Flush other input
	if ((replay == 'y') || (replay == 'Y')) {
		/* Open input file */
		fopen_s(&f, "C:\\Users\\Greg\\Rec\\rec.dat", "rb");
		if (!f) {
			printf("unable to open %s\n", "C:\\myfiles\\recording.dat");
			return 0;
		}
		printf("Reading from sound file ...\n");
		fread(iBigBufNew, sizeof(short), lBigBufSize, f);				// Record to new buffer iBigBufNew
		fclose(f);
	}
	InitializePlayback();
	printf("\nPlaying recording from saved file ...\n");
	PlayBuffer(iBigBufNew, lBigBufSize);
	ClosePlayback();

	printf("\n");
	system("pause");
	return(0);
}

