#include "syscall.h"


int main()
{
	int pingId, pongId;
    PrintString("Start\n");
    pingId = Exec("ping");
    pongId = Exec("pong");
    Join(pingId);
    Join(pongId);
    PrintString("Finish!");
    Exit(0);
}
