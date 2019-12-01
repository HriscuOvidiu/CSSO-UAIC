// Tema3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <time.h>

#define MAX_VALUE 100
#define PROCESS_PATH "D:\\Coding\\Facultate\\CSSO\\Program2\\Debug\\Program2.exe"

using namespace std;

struct Pair {
	int initial_number;
	int final_number;

	Pair(int initial_number, int final_number) {
		this->initial_number = initial_number;
		this->final_number = final_number;
	}
};

int main()
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	HANDLE new_pair_event = CreateEvent(NULL, FALSE, FALSE, "new_pair");
	HANDLE request_number_event = CreateEvent(NULL, FALSE, FALSE, "request_number");

	srand(time(NULL));

	HANDLE hData = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024 * 1024, "data");
	if (hData == NULL) {
		printf("Cannot create file mapping. Error code: %d", GetLastError());
		return 0;
	}

	if (!CreateProcess(PROCESS_PATH, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		printf("Cannot create process.\n");
		return 0;
	}

	unsigned char* pData = (unsigned char*)MapViewOfFile(hData, FILE_MAP_WRITE, 0, 0, 0);
	if (pData == NULL) {
		printf("Cannot get pointer to file mapping. Error code: %d", GetLastError());
		CloseHandle(hData);
		return 0;
	}

	while (1)
	{
		WaitForSingleObject(request_number_event, INFINITE);
		ResetEvent(request_number_event);

		//Sleep(100);
		const int num = rand() % MAX_VALUE;
		const int result = num * 2;
		Pair *pair = new Pair(num, result);

		memcpy(pData, pair, sizeof(Pair));
		SetEvent(new_pair_event);
	}

	system("pause");
}
