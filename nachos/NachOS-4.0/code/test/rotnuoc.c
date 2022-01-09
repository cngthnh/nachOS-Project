#include "syscall.h"

void WriteInt(int fileId, int value);

int main()
{
    int i; //Bien dem cho vong for
	int time; //Bien mo phong thoi gian rot nuoc
    int fileOutput; //Bien luu id mo file output
	char st[2]; //Bien dung chuyen so sang chuoi
	int idProcess; //id cua tien trinh dang chay
    int pos;

    for (i = 0; i < 10; ++i)
    {
        Wait("rotnuoc");
        fileOutput = Open("output.txt", 0);
        idProcess = GetProcessID();
		pos = Seek(-1, fileOutput);
        Seek(pos, fileOutput);

		st[0] = 48+idProcess;
		st[1] = '\0';
        Write(st, 1, fileOutput);
		Write(" ", 1, fileOutput);
       
        Close(fileOutput);
        PrintString("Sinh vien ");
		PrintNum(idProcess);
		PrintString(" rot nuoc.\n");
        time = RandomNum() % 10000;
        while (time--)
            ;
        Signal("rotnuoc");
    }
    Exit(0);
}
