#include "OperatingSystem.h"

#ifdef Windows


//#include <Windows.h>    // Includes the functions for serial communication via RS232
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RS232Comm.h"
#include "menu.h"
#include "queues.h"
#include "Frame.h"
#include "Multithreading.h"

short iBigBuf[SAMPLES_SEC * RECORD_TIME];
long lBigBufSize = SAMPLES_SEC * RECORD_TIME;

int SettingFlag;
int recordTime;

HANDLE RCom;										// Pointer to recieving COM port
HANDLE TCom;										// Pointer to sending COM port
int nComRate = 9600;								// Baud (Bit) rate in bits/second 
int nComBits = 8;									// Number of bits per frame
COMMTIMEOUTS timeout;
// com 0 com transmission
//#define COMPORT "COM4"
// Ensure that default character set is not Unicode
// Communication variables and parameters


// Initializes the port and sets the communication parameters
void initPortT() {										// Transmit only
	createPortFileSend();							// Initializes hCom to point to PORT4 (port 4 is used by USB-Serial adapter on my laptop)
	purgePort(TCom);									// Purges the COM port
	SetComParms(TCom);									// Uses the DCB structure to set up the COM port
	purgePort(TCom);
}

void initPortR() {										// Recieve only
	createPortFileRec();							// Initializes hCom to point to PORT4 (port 4 is used by USB-Serial adapter on my laptop)
	purgePort(RCom);									// Purges the COM port
	SetComParms(RCom);									// Uses the DCB structure to set up the COM port
	purgePort(RCom);
}

// Purge any outstanding requests on the serial port (initialize)
void purgePort(HANDLE hCom) {														// function for either port
	PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
}

// Output message to port (COM4)
void outputToPort(LPCVOID buf, DWORD szBuf) {										// Transmit only
	int i = 0;
	initPortT();
	DWORD NumberofBytesTransmitted;
	LPDWORD lpErrors = 0;
	LPCOMSTAT lpStat = 0;

	//extern TxRx* txrx;
	//std::thread tx = txrx->RS232TxThread(TIMEOUT, hCom, buf, szBuf, &NumberofBytesTransmitted);
	
	i = WriteFile(
		TCom,										// Write handle pointing to transmission port
		buf,										// Buffer size
		szBuf,										// Size of buffer
		&NumberofBytesTransmitted,					// Written number of bytes
		NULL
	);

	// Handle the timeout error
	if (i == 0) {
		printf("\nTx Port Failed: Error # 0x%x\n", GetLastError());
		ClearCommError(TCom, lpErrors, lpStat);		// Clears the device error flag to enable additional input and output operations. Retrieves information ofthe communications error.	
	}
	else {
		printf("\nSuccessful transmission, there were %ld bytes transmitted\n", NumberofBytesTransmitted);
	}
	//tx.join();
}

// Dont use in this file - This file only outputs not inputs

int inputFromPort(LPVOID buf, DWORD szBuf) {								// Recieve only function
	int i = 0;
	DWORD NumberofBytesRead;
	LPDWORD lpErrors = 0;
	LPCOMSTAT lpStat = 0;

	i = ReadFile(
		RCom,										// Read handle pointing to recieving COM port
		buf,										// Buffer size
		szBuf,  									// Size of buffer - Maximum number of bytes to read
		&NumberofBytesRead,
		NULL
	);
	// Handle the timeout error
	if (i == 0) {
		printf("\nRx Port Failed: Error # 0x%x\n", GetLastError());
		ClearCommError(RCom, lpErrors, lpStat);		// Clears the device error flag to enable additional input and output operations. Retrieves information ofthe communications error.
	}
	else {
		//printf("\nSuccessful reception!, There were %ld bytes read\n", NumberofBytesRead);
		return(NumberofBytesRead);
	}
	return 0;
}

// Sub functions called by above functions
/**************************************************************************************/

// Set the hCom HANDLE to point to a COM port, initialize for reading and writing, open the port and set securities
void createPortFileRec() {				// Recieve only
	// Call the CreateFile() function 
	RCom = CreateFile(
		COMPORTR,									// COM port number  --> If COM# is larger than 9 then use the following syntax--> "\\\\.\\COM10"
		GENERIC_READ | GENERIC_WRITE,				// Open for read and write
		NULL,										// No sharing allowed
		NULL,										// No security
		OPEN_EXISTING,								// Opens the existing com port
		FILE_ATTRIBUTE_NORMAL,						// Do not set any file attributes --> Use synchronous operation
		NULL										// No template
	);

	if (RCom == INVALID_HANDLE_VALUE) {
		printf("\nFatal Error 0x%x: Unable to open Rx\n", GetLastError());
	}
	else {
		//printf("\nCOM is now open\n");
	}
}
void createPortFileSend() {				// Transmit only
	// Call the CreateFile() function 
	TCom = CreateFile(
		COMPORTT,									// COM port number  --> If COM# is larger than 9 then use the following syntax--> "\\\\.\\COM10"
		GENERIC_READ | GENERIC_WRITE,				// Open for read and write
		NULL,										// No sharing allowed
		NULL,										// No security
		OPEN_EXISTING,								// Opens the existing com port
		FILE_ATTRIBUTE_NORMAL,						// Do not set any file attributes --> Use synchronous operation
		NULL										// No template
	);

	if (TCom == INVALID_HANDLE_VALUE) {
		printf("\nFatal Error 0x%x: Unable to open Tx\n", GetLastError());
	}
	else {
		printf("\nCOM is now open\n");
	}
}

// Defines and sets values for COM port.
static int SetComParms(HANDLE hCom) {				// For either port
	DCB dcb;										// Windows device control block
	// Clear DCB to start out clean, then get current settings
	memset(&dcb, 0, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	if (!GetCommState(hCom, &dcb))
		return(0);

	// Set our own parameters from Globals
	dcb.BaudRate = nComRate;						// Baud (bit) rate
	dcb.ByteSize = (BYTE)nComBits;					// Number of bits(8)
	dcb.Parity = 0;									// No parity	
	dcb.StopBits = ONESTOPBIT;						// One stop bit
	if (!SetCommState(hCom, &dcb))
		return(0);

	// Set communication timeouts (SEE COMMTIMEOUTS structure in MSDN) - want a fairly long timeout
	// This sets the time for how long the com port will be open to reception or transmission.
	memset((void*)& timeout, 0, sizeof(timeout));
	timeout.ReadIntervalTimeout = 500;				// Maximum time allowed to elapse before arival of next byte in milliseconds. If the interval between the arrival of any two bytes exceeds this amount, the ReadFile operation is completed and buffered data is returned
	timeout.ReadTotalTimeoutMultiplier = 1;			// The multiplier used to calculate the total time-out period for read operations in milliseconds. For each read operation this value is multiplied by the requested number of bytes to be read
	timeout.ReadTotalTimeoutConstant = 5000;		// A constant added to the calculation of the total time-out period. This constant is added to the resulting product of the ReadTotalTimeoutMultiplier and the number of bytes (above).
	SetCommTimeouts(hCom, &timeout);
	return(1);
}


#endif // Windows






int sidCount = 0;
/************************************SEND MESSAGE******************************************/
void send_message(int status) {												// Transmission only
	// Initialize the port
	char c;
	int count = 0;
	int flag = 0;
	char buf[sizeof(MssgTxt)];		//Text payload is 140 byes, 16 bytes for the header

	MessageCounter++;		// Gets initalized to zero every time the program starts
/************************************PERSONAL MESSAGE******************************************/
	if (status == 1) {
		char msgOut[140];
		// Sender Program -- comment out these 3 lines if receiver
		printf("Enter a string up to 140 characters to send another computer.\n");

		while ((c = getchar()) != '\n' && (c != EOF)) {

			if (count == 139) {
				msgOut[139] = '\0';
				flag = 1;
				break;
			}

			msgOut[count] = c;
			count++;
		}
		if (flag == 0) {
			msgOut[count] = '\0';
		}

		memcpy_s(buf, sizeof(buf), AppendMsg(msgOut), TXTBUFF);	// Append the header and payload to a char buffer to send;
		memcpy_s(cvt.convert, sizeof(cvt.convert), buf, sizeof(buf));
		//printf("\n%lx\n", cvt.FRAME.Head.lstartSequence);


		outputToPort(cvt.convert, sizeof(buf) + 1);		// Send string to port - include space for '\0' termination
		Sleep(1000);									// Allow time for signal propagation on cable 

		purgePort(TCom);									// Purge the port
		CloseHandle(TCom);								// Closes the handle pointing to the COM port

		//AddToQueue(msgOut);
		printf("\nPress any key to return to previous Menu\n");
		getchar();
		textMenu();

	}

	/************************************DEFAULT MESSAGE******************************************/
	else if (status == 2) {
		char msgOut[] = "Hi there person\n";

		memcpy_s(buf, sizeof(buf), AppendMsg(msgOut), TXTBUFF);	// Append the header and payload to a char buffer to send;
		memcpy_s(cvt.convert, sizeof(cvt.convert), buf, sizeof(buf));
		//printf("\n%lx\n", cvt.FRAME.Head.lstartSequence);

		outputToPort(cvt.convert, sizeof(buf) + 1);		// Send string to port - include space for '\0' termination
		Sleep(1000);									// Allow time for signal propagation on cable 

		purgePort(TCom);									// Purge the port
		CloseHandle(TCom);								// Closes the handle pointing to the COM port

		printf("\nPress any key to return to previous Menu\n");
		getchar();
		textMenu();
	}

	printf("\nPress any key to return to previous Menu\n");
	getchar();
	textMenu();
}

/************************************RANDOM MESSAGE******************************************/
void randMessage(char* msg) {												// Transmit only
	// Initialize the port
	int count = 0;
	int flag = 0;
	char buf[sizeof(MssgTxt)];		//Text payload is 140 byes, 16 bytes for the header

	memcpy_s(buf, sizeof(buf), AppendMsg(msg), TXTBUFF);	// Append the header and payload to a char buffer to send;
	memcpy_s(cvt.convert, sizeof(cvt.convert), buf, sizeof(buf));

	outputToPort(cvt.convert, sizeof(buf) + 1);		// Send string to port - include space for '\0' termination
	Sleep(500);									// Allow time for signal propagation on cable 

	purgePort(TCom);									// Purge the port
	CloseHandle(TCom);								// Closes the handle pointing to the COM port
	textMenu();
}



/************************************RECIEVE MESSAGE******************************************/
void recieveMessage() {									// Receive only

	pointer = (link)malloc(sizeof(Node) + sizeof(cvt.convert));
	char msgIn[TXTBUFF + 1];
	initPortR();										// Initialize the port

	// The input message is saved to msgIn

	int MsgBytes = inputFromPort(msgIn, (TXTBUFF + 1));	// Receive string from port
	char *Buff = (char*)malloc(cvt.FRAME.Head.luncompressedLen * sizeof(char));
	if (MsgBytes > 0) {								// if there is actually a message
		msgIn[MsgBytes - 1] = '\0';
		//This copys the whole buffer (header and message) into memory at cvt.convert the union
		memcpy_s(cvt.convert, sizeof(cvt.convert) + 2, msgIn, MsgBytes);
		
		
		RLE_Uncompress((unsigned char *)cvt.FRAME.Text.message, (unsigned char *)Buff, cvt.FRAME.Head.lcompressedLen);

		//printf("\nMessage Received: \n%s\n", Buff);	// Display message from port

		/*****STORE VALUES OF THE UNION INTO THE QUEUE*****/
		pointer->Data.sid = sidCount;		//Labels the number of messages in the queue
		strcpy_s(pointer->Data.message, cvt.FRAME.Head.lcompressedLen, cvt.FRAME.Text.message);
		//pointer->Data.lstartSequence = cvt.FRAME.Head.lstartSequence;
		//pointer->Data.lDataLength = cvt.FRAME.Head.luncompressedLen;
		//pointer->Data.bPattern = cvt.FRAME.Head.bPattern;
		//pointer->Data.ReceiveAddr = cvt.FRAME.Head.ReceiveAddr;
		//pointer->Data.flag = cvt.FRAME.Head.flag;
		Recieve.AddToQueue(pointer);

		msgFlag++;
		//printf("\nNew message!\n");
		sidCount++;
	}

	purgePort(RCom);									// Purge the port
	CloseHandle(RCom);						// Closes the handle pointing to the COM port
	//printf("\nPress any key to return to previous Menu\n");
	//getchar();
	//textMenu();
}


void SendAudio() {					//Transmit only

	char buf[sizeof(MssgAud)];

	if (SettingFlag == 1) {
		iBigBuf[SAMPLES_SEC * recordTime];
		lBigBufSize = SAMPLES_SEC * recordTime;
	}
	InitializePlayback();
	InitializeRecording();


	// start recording
	RecordBuffer(iBigBuf, lBigBufSize);
	CloseRecording();

	memcpy_s(buf, sizeof(buf), AppendAudio(iBigBuf), lBigBufSize);	// Append the header and payload to a char buffer to send;
	memcpy_s(cvtA.convert, sizeof(cvtA.convert), buf, sizeof(buf));

	PlayBuffer(cvtA.FRAMEAUD.Aud.audioBuf, lBigBufSize);
	ClosePlayback();

	printf("\nPress any key to send the recording\n");
	getchar();

	outputToPort(cvtA.convert, sizeof(buf) + 1);	// Send string to port - include space for '\0' termination
	Sleep(1000);									// Allow time for signal propagation on cable 

	purgePort(TCom);									// Purge the port
	CloseHandle(TCom);								// Closes the handle pointing to the COM port
	
	printf("\nPress any key to return to previous Menu\n");
	getchar();
}

void RecieveAudio() {						// Recieve only fn

	pointer = (link)malloc(sizeof(Node) + sizeof(cvtA.convert));
	char msgIn[48000];
	printf("\nPress any key to recieve a message - (make sure this programming is running before you send a message)\n");
	getchar();
	InitializePlayback();
	initPortR();										// Initialize the port

	
	int MsgBytes = inputFromPort(msgIn, (48001));	// Receive string from port

	memcpy_s(cvtA.convert, sizeof(cvtA.convert), msgIn, (48000));


	PlayBuffer(cvtA.FRAMEAUD.Aud.audioBuf, lBigBufSize);
	ClosePlayback();


	/*****STORE VALUES OF THE UNION INTO THE QUEUE*****/
	Recieve.AddToQueue(pointer);
	pointer->Data.sid = sidCount;		//Labels the number of messages in the queue
	//strcpy_s(pointer->Data.message, sizeof(cvt.FRAME.Text.message), cvt.FRAME.Text.message);
	pointer->Data.lDataLength = cvt.FRAME.Head.luncompressedLen;
	pointer->Data.ReceiveAddr = cvt.FRAME.Head.ReceiveAddr;
	pointer->Data.flag = cvt.FRAME.Head.flag;

	purgePort(RCom);									
	CloseHandle(RCom);						

	sidCount++;
	printf("\nPress any key to return to previous Menu\n");
	getchar();
	//textMenu();
}

void AudioSettings() {

	printf("Enter length of recording (s).\n");
	scanf_s("%d", &recordTime, 2);


	SettingFlag = 1;
	printf("\nYour Audio settings have been updated!\n");
	settings();
}
