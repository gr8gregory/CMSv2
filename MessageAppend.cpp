#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Frame.h"
#include "sound.h"
#include "RS232Comm.h"

#define _CRT_SECURE_NO_WARNINGS


char* AppendMsg(char* msg) {
	
	char ReceiveAddr = 0xf;
	short sid  = 0xaaa;							// Sender ID
	char flag = 0x0; 
	long luncompressedLen = strlen(msg) + 1;
	long lcompressedLen;
	payloadText text;
	char buff[142];
	printf("\n%d", luncompressedLen);
	lcompressedLen = RLE_Compress((unsigned char*)msg, (unsigned char*)buff, luncompressedLen);
	printf("\n%d", lcompressedLen);
	
	
	text.message = (char*)malloc((luncompressedLen + 3) * sizeof(char));

	memcpy_s(text.message, (luncompressedLen + 3), buff, lcompressedLen);

	//text.message[luncompressedLen - 1] = '\0';
	
	header FRAME = { ReceiveAddr, sid, flag, luncompressedLen, lcompressedLen };

	MssgTxt holder = { NULL, NULL };
	holder.Head = FRAME;
	holder.Text = text;
	
	cvt.FRAME = holder;

	printf("\n%d", (sizeof(cvt.FRAME.Head) + lcompressedLen));
	return cvt.convert;
}


void PrintQueue(link pn) {

	while (!Recieve.IsQueueEmpty()) {				// if there is no head's avaiable
		pn = Recieve.DeQueue();						//deque sends the current head to main and then moves head to next spot
		printf("\n sID of Node is: %d", pn->Data.sid);		//print what the current heads node value
		free(pn);					//free the head
	}
}

char* AppendAudio(short* Audio) {

	char ReceiveAddr = 0xf;
	short sid = 0xaaa;							// Sender ID
	char flag = 0x0;
	long lDataLength = lBigBufSize;

	header FRAMEAUD = { ReceiveAddr, sid, flag, lDataLength };
	
	payloadAudio audio;
	memcpy_s(audio.audioBuf, sizeof(audio.audioBuf), Audio, 48000);// this memcopy 

	MssgAud Holder = { NULL, NULL };
	Holder.Head = FRAMEAUD;
	Holder.Aud = audio;

	cvtA.FRAMEAUD = Holder;
	return cvtA.convert;
}