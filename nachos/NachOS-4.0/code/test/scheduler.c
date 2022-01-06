#include "syscall.h"


void main()
{
	int pingPID, pongPID;
	PrintString("Ping-Pong test starting...\n\n");
	pingPID = Exec("ping");
	pongPID = Exec("pong");
	Join(pingPID);
	Join(pongPID);
    PrintString("Ping-Pong test endding...\n\n");
    Halt();
}