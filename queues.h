/* Queues.h - Interface for Queue functionality
 *
 * Copyright 2019 Caleb Hoeksema, Gregory Huras, Laura Muhlbauerova
 *			October 15, 2019:	Add returnHead(), visit(), traverse(), and traverseR()
 */

#pragma once					// so it is only included once - could use ifndef instead
const int iLength = 140;		// Length of the array for storing the message

// header (gives the type of payload
// Payload - contains the message - text, audio


// Rename the structures
typedef struct node Node;
typedef struct item Item;
typedef Node* link;

struct item {
	char message[iLength];		// Message text
	short sid;				// Sender ID
	short rid;				// Receiver ID
	char priority;				// Priority of message
	short int seqNum;			// Message number (used in communications)
	

	char ReceiveAddr;			//not sure what to have here
	char flag;					//0 - text, no comp. 1 - text, comp. 2 - audio, no comp. 3 - audio, comp
	long lDataLength;			//sizeof the actual data of the message
};

struct node {
	link pNext;					// pointer to this type of structure (next node in linked list)
	Item Data;					// structure holding information
};


class Queue {
private:
	link Head, Tail;

public:
	void queue(void);
	int IsQueueEmpty(void);
	void AddToQueue(link);
	link DeQueue(void);
	link returnHead(void);
	
	void traverseR(link h, void (*visit)(link));
	int traverse(link h, void (*visit)(link));
};

void visit(link h);
char* AppendMsg(char* msg);

void QueueDiagnostics(void);
void PrintMessagesInQ(void);
void PrintContentsDiag(void);
static int MessageCounter = 0;

extern Queue Recieve;

// Function Prototypes
//void InitQueue(void);



// Recursive Queue functions
/*
link returnHead();
void visit(link h);
void traverseR(link h, void (*visit)(link));
void traverse(link h, void (*visit)(link));
*/