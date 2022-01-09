#include "syscall.h"

int main() {
    int fileSuccess; //Kiem tra cac thao tac voi file co thanh cong khong
    int n; //Bien luu so luong sinh vien
	int i; //BIen dem cho cac vong lap
    int processId[5]; //Luu lai cac process id cua Exec
	char cRead; //Luu ki tu doc duoc tu file
    // Tao Semaphore kiem soat 1 voi nuoc
    if (CreateSemaphore("rotnuoc", 1) == -1) 
	{
        return 1;
    }

	//Mo file input de doc so luong sinh vien
    fileSuccess = Open("input.txt", 1);
	if(fileSuccess == -1)
	{
		return 1;
	}

	//Doc so luong sinh vien
	while(1)
	{
		Read(&cRead, 1, fileSuccess);
		if(cRead != '\n')
		{
			if(cRead >= '0' && cRead <= '9')
				n = n * 10 + (cRead - 48);
		}
		else
			break;
	}

	//Dong file input.txt
    Close(fileSuccess);

	//Tao file output.txt
    fileSuccess =  CreateFile("output.txt");
	if(fileSuccess == -1)
	{
		return 1;
	}

	//Goi chuong trinh rotnuoc chay tuong duong voi so sinh vien
    for (i = 0; i < n; ++i) {
        processId[i] = Exec("rotnuoc");
    }
    for (i = 0; i < n; ++i) {
        Join(processId[i]);
    }

	return 0;
}

