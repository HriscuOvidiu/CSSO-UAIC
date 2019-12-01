// Program.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>

using namespace std;

struct ProcessData {
	string pid;
	string ppid;
	char exe_name[50];
};

int main()
{
	vector<ProcessData> process_vect;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcess == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot failed.err = %d \n", GetLastError());
		return(-1);
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcess, &pe32))
	{
		printf("Process32First failed. err = %d \n", GetLastError());
		CloseHandle(hProcess); 
		return(-1);
	}
	do
	{
		ProcessData process_data;
		process_data.pid = to_string(pe32.th32ProcessID);
		process_data.ppid = to_string(pe32.th32ParentProcessID);
		strcpy(process_data.exe_name, pe32.szExeFile);

		process_vect.push_back(process_data);
		
		printf("Process [%d]: %s \n", pe32.th32ProcessID, pe32.szExeFile);
	} while (Process32Next(hProcess, &pe32));
	CloseHandle(hProcess);

	HANDLE hData = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(ProcessData) * process_vect.size(), "data");
	if (hData == NULL) {
		printf("Cannot create file mapping. Error code: %d", GetLastError());
		return 0;
	}

	ProcessData** pData = (ProcessData**)MapViewOfFile(hData, FILE_MAP_WRITE, 0, 0, 0);
	if (pData == NULL) {
		printf("Cannot get pointer to file mapping. Error code: %d", GetLastError());
		CloseHandle(hData);
		return 0;
	}

	CopyMemory(pData, process_vect.data(), sizeof(ProcessData) * process_vect.size());
	printf("size: %d\n", process_vect.size());

	UnmapViewOfFile(hData);

	CloseHandle(hData);

	return 0;
}