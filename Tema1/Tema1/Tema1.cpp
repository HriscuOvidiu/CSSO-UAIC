// Tema1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <stdio.h>
#include <Windows.h>

#define BASE_PATH "D:\\Coding\\Facultate\\RN"

void searchInPath(const char* directory_path = NULL);

int main()
{
	searchInPath();
}

void searchInPath(const char* directory_path)
{
	char* path = new char[200];
	strcpy(path, BASE_PATH);
	if (directory_path)
	{
		strcat(path, "\\");
		strcat(path, directory_path);
	}
	strcat(path, "\\*");

	WIN32_FIND_DATA find_data;
	HANDLE hDir = FindFirstFile(path, &find_data);

	while (FindNextFile(hDir, &find_data)) {
		if (strcmp((char*)find_data.cFileName, ".") != 0
		 && strcmp((char*)find_data.cFileName, "..") != 0)
		{
			if (find_data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				char* new_directory_path = new char[200];
				if (directory_path)
				{
					strcpy(new_directory_path, directory_path);
				}
				else
				{
					strcpy(new_directory_path, "");
				}

				strcat(new_directory_path, "\\");
				strcat(new_directory_path, find_data.cFileName);

				searchInPath(new_directory_path);
			}
			else
			{
				HKEY hKey;
				DWORD disposition = 0;

				char* key_path = new char[200];
				strcpy(key_path, "SOFTWARE\\CSSO");
				if (directory_path)
				{
					strcat(key_path, directory_path);
				}

				auto keyCreation = RegCreateKeyEx(HKEY_CURRENT_USER,
					key_path,
					0,
					NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_WRITE,
					NULL,
					&hKey,
					&disposition);

				if (keyCreation != ERROR_SUCCESS)
				{
					printf("Nu s-a putut creea cheia. Cod eroare: %d\n", GetLastError());
					return;
				}

				LARGE_INTEGER size;

				size.HighPart = find_data.nFileSizeHigh;
				size.LowPart = find_data.nFileSizeLow;

				RegSetValueEx(hKey,
							find_data.cFileName,
							0,
							REG_DWORD,
							(const BYTE*)&(size),
							sizeof(DWORD));

				RegCloseKey(hKey);
			}
		}
	}

	FindClose(hDir);
}
