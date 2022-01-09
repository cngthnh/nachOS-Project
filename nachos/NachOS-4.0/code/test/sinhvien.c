#include"syscall.h"
#include"copyright.h"

void main()
{
	
	int f_Success; // Bien co dung de kiem tra thanh cong
	int si_input, si_output, si_sinhvien, si_result;	// Bien id cho file
	char c_readFile;	// Bien ki tu luu ki tu doc tu file
	//PrintNum(Wait("sinhvien"));
	si_sinhvien = Open("sinhvien.txt", 0);
	if(si_sinhvien == -1)
	{
		return;
	}

	while(1)
	{
		if(Read(&c_readFile, 1, si_sinhvien) < 1)
		{
			// Doc toi cuoi file
			break;
		}
		if(c_readFile != '\n')
		{
			PrintChar(c_readFile);
			PrintNum(2);
			//Write(&c_readFile, 1, si_sinhvien);			
		}
		else
		{
			break;
		}
	}
	Exit(0);
}