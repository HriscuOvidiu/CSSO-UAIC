// Program2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <windows.h>

using namespace std;

struct Pair {
	int initial_number;
	int final_number;

	Pair(int initial_number, int final_number) {
		this->initial_number = initial_number;
		this->final_number = final_number;
	}
};

BOOL enable_privilege(HANDLE hToken, LPCTSTR lpszPrivilege)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(
		NULL,
		lpszPrivilege,
		&luid))
	{
		printf("LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		printf("AdjustTokenPrivileges error: %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		printf("The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}

int main()
{
	int counter = 1;

	HANDLE token;
	HANDLE request_number_event = OpenEvent(EVENT_ALL_ACCESS, FALSE, "request_number");
	HANDLE new_pair_event = OpenEvent(EVENT_ALL_ACCESS, FALSE, "new_pair");

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
	{
		printf("Cannot get process token: %d", GetLastError());
		exit(1);
	}

	if (!enable_privilege(token, SE_DEBUG_NAME))
	{
		exit(1);
	}

	HANDLE hData = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "data");
	if (hData == NULL) {
		printf("Cannot create file mapping. Error code: %d", GetLastError());
		return 0;
	}

	SetEvent(request_number_event);

	while (1)
	{
		WaitForSingleObject(new_pair_event, INFINITE);
		ResetEvent(new_pair_event);

		Pair *data = (Pair*)MapViewOfFile(hData, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		SetEvent(request_number_event);

		cout << counter++ << ". 2 * " << data->initial_number << " == " << data->final_number << " : ";
		cout << ((data->initial_number * 2 == data->final_number) ? "correct" : "incorrect") << '\n';
	}
}
