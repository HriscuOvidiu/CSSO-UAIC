#include "pch.h"
#include <windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <map>

using namespace std;

#define NUMBER_OF_PROCESSES 212
#define PAD_VAL 4

struct ProcessData {
	string pid;
	string ppid;
	char exe_name[50];
};

struct ProcessNode {
	ProcessData process_data;
	vector<ProcessNode> children;

	ProcessNode(ProcessData p) 
	{
		process_data = p;
	}

	ProcessNode(ProcessData p, vector<ProcessNode> c)
	{
		process_data = p;
		children = c;
	}
};

BOOL enable_privilege( HANDLE hToken, LPCTSTR lpszPrivilege)
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

int get_number_of_descendants(ProcessNode t)
{
	int num = t.children.size();
	
	for (auto c : t.children)
	{
		num += get_number_of_descendants(c);
	}

	return num;
}

bool execute_option_one(vector<ProcessNode> trees, string name)
{
	bool found = false;
	for (auto t : trees)
	{
		if (strcmp(t.process_data.exe_name, name.c_str()) == 0)
		{
			int num = get_number_of_descendants(t);

			cout << "Process id: " << t.process_data.pid << " cu " << num << " descendenti\n";

			found = true;

			execute_option_one(t.children, name);
		}
		else
		{
			bool r = execute_option_one(t.children, name);
			found = found || r;
		}
	}
	
	return found;
}

void execute_option_two(vector<ProcessNode> &trees, string pid)
{
	for (auto &t : trees)
	{
		if (t.process_data.pid == pid)
		{
			DWORD dpid = atoi(t.process_data.pid.c_str());
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, dpid);

			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
			else
			{
				printf("Failed to open process %s: %d", t.process_data.pid.c_str(), GetLastError());
			}

			return;
		}

		execute_option_two(t.children, pid);
	}
}

void add_children_to_tree(ProcessNode &tree, map<string, vector<ProcessNode>> &m)
{
	auto it = m.find(tree.process_data.pid);
	if (it!= m.end())
	{
		tree.children = it->second;
		m.erase(it);
		for (auto &child : tree.children)
		{
			add_children_to_tree(child, m);
		}
	}
}

bool is_root_process(ProcessData p, vector<ProcessData> vect)
{
	for (auto q : vect)
	{
		if (p.ppid == q.pid)
		{
			return false;
		}
	}

	return true;
}

vector<ProcessNode> process_trees(vector<ProcessData> vect)
{
	vector<ProcessNode> trees;
	map<string, vector<ProcessNode>> m;
	int size = vect.size();
	for (auto p : vect)
	{
		if (p.pid == p.ppid || is_root_process(p, vect))
		{
			ProcessNode pn(p);
			trees.push_back(pn);
		}
		else
		{
			if (m.find(p.ppid) == m.end())
			{
				vector<ProcessNode> v;
				m[p.ppid] = v;
			}
			auto v = m[p.ppid];
			v.push_back(ProcessNode(p));
			m[p.ppid] = v;
		}
	}

	for (auto &t : trees)
	{
		add_children_to_tree(t, m);
	}

	return trees;
}

void print_trees(vector<ProcessNode> trees, int padding = 0)
{
	int nr = 1;
	string padding_string="";
	for (int i = 0; i < padding; i++)
	{
		padding_string += ' ';
	}
	for (auto t : trees)
	{
		if (padding == 0)
		{
			cout << "[arbore nr." << nr++ << "]\n";
		}
		cout << padding_string << t.process_data.exe_name << " " << t.process_data.pid << "\n";

		print_trees(t.children, padding + PAD_VAL);
	}
}

void print_intro()
{
	cout << "Selectati una din optiunile de mai jos" << '\n';
	cout << "1. Se citeste un nume de proces de la tastatura si se afiseaza pentru fiecare proces cu acel nume existent pe sistem pid"
		"- ul acestuia si numarul de descendenti." << "\n";
	cout << "2. Se citeste pid-ul unui proces si se vor inchide toate procesele din subarborele care are radacina in acel PID." << '\n';
}

int main()
{
	HANDLE token;
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

	ProcessData process_arr[NUMBER_OF_PROCESSES];
	ProcessData* result = (ProcessData*)MapViewOfFile(hData, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	CopyMemory(process_arr, result, sizeof(ProcessData) * NUMBER_OF_PROCESSES);

	UnmapViewOfFile(hData);
	CloseHandle(hData);

	vector<ProcessData> process_vect(process_arr, process_arr + NUMBER_OF_PROCESSES);
	vector<ProcessNode> trees = process_trees(process_vect);
	
	print_trees(trees);
	cout << "\n\n";

	int num;

	while (1)
	{
		print_intro();
		
		string input;
		input.reserve(400);
		bool r;
		cin >> num;
		cin.ignore();

		switch (num)
		{
			case 0:
				exit(0);

			case 1:
				cout << "Introduceti un nume de proces\n";
				getline(cin, input);
				r = execute_option_one(trees, input);
				if (!r)
				{
					cout << "Nu am gasit nici un proces cu acest nume.\n";
				}
				break;

			case 2:
				cout << "Introduceti un pid\n";
				getline(cin, input);
				execute_option_two(trees, input);
				break;

			default:
				cout << "Selectati doar 1 sau 2" << '\n';
				break;
			}
	}
}