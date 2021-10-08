#include "syscall.h"

int main()
{
	int n,i,j,temp;
	char direction,swapped;
	int a[100];
	PrintString("Nhap so luong phan tu cua mang can sap xep: ");
	n = ReadNum();
	PrintString("Nhap chieu sap xep (0: tu nho den lon; 1: tu lon den nho): ");
	direction = ReadChar();

	for (i=0; i<n; ++i)
	{
		PrintString("A[");
		PrintNum(i);
		PrintString("] = ");
		a[i] = ReadNum();
	}

	for (i=0; i<n-1; ++i)
	{
		swapped=0;
		for (j=0; j<n-1-i; ++j)
		{
			if (direction==48)
			{
				if (a[j]>a[j+1])
				{
					temp=a[j];
					a[j]=a[j+1];
					a[j+1]=temp;
					swapped=1;
				}
			}
			else
			{
				if (a[j]<a[j+1])
				{
					temp=a[j];
					a[j]=a[j+1];
					a[j+1]=temp;
					swapped=1;
				}
			}
		}
		
		if (swapped==0) break;
	}
	
	PrintString("Mang sau khi sap xep: ");

	for (i=0; i<n; ++i)
	{
		PrintNum(a[i]);
		PrintChar(' ');
	}
	PrintChar(' ');
	Halt();
}
