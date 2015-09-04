#include <windows.h> 
#include <stdio.h> 

#define NUM_THREADS 5 

DWORD __stdcall PrintHello(LPVOID threadid)
{
	long tid = (long)threadid;

	//for (int i = 0; i < 5; i++)
	printf("Hello World! It's me, thread #%ld!\n", tid);
	ExitThread(NULL);
}

int main(int argc, char *argv[])
{
	HANDLE threads[NUM_THREADS];

	for (unsigned int t = 0; t < NUM_THREADS; t++)
	{
		printf("In main: creating thread %ld\n", t);
		threads[t] = CreateThread(NULL, 0, PrintHello, (LPVOID)t, 0, NULL);
		if (threads[t] == NULL)
		{
			printf("ERROR. Return code from CreateThread() is %d\n", GetLastError());
			exit(-1);
		}
	}
	ExitThread(NULL);
}
