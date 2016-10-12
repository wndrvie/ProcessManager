#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <psapi.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

struct Process{
    unsigned long int id; 
    string name;
    HANDLE handle;
    long int memory;
    DWORD errorName;
};

//pr1 < pr2 - true 
struct Comparator{
    bool operator()(const Process& pr1, const Process& pr2){
        if(pr1.memory < pr2.memory){
            return true;
        }
        else 
            return false;
    }
} comp;

Process CreateProcessInfo(PROCESSENTRY32 proc)
{
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;
    Process process;
    process.name = proc.szExeFile;
    process.id = proc.th32ProcessID;

    //
    hProcess = OpenProcess( PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION
            | PROCESS_VM_READ, FALSE, process.id );
    if (NULL == hProcess){
        process.handle = NULL;
        process.memory = -1;
        process.errorName = GetLastError();
        return process;
    }
    
    process.handle = hProcess;
    
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))){
        process.memory = pmc.WorkingSetSize;
        process.errorName = 0;
    } else{
        process.memory = -2;
        process.errorName = GetLastError();
    }
    
    return process;
}

void PrintInform(Process proc){
    cout.width(9);
    cout << proc.id;
    cout.width(30); 
    cout << proc.name;
    cout.width(12);
    cout << proc.memory << " " << proc.errorName << endl;
}

int main(int argc, char* argv[])
{   
    HANDLE hSnap;
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == NULL)
    {
        cout << "Snapshot hasn't been taken." << endl;
        return 0;
    }

    PROCESSENTRY32 proc;
    proc.dwSize = sizeof(PROCESSENTRY32);
    vector<Process> procArray;

    if (Process32First(hSnap, &proc))
    {
        do{
            procArray.push_back(CreateProcessInfo(proc));
        }while (Process32Next(hSnap, &proc));
        
        cout << "Processes: " << procArray.size() << endl << endl;
        cout << " No";
        cout.width(9);
        cout << "PID";
        cout.width(30); 
        cout << "name.exe";
        cout.width(12);
        cout << "RAM" << " ErrorCode" << endl << endl;
        
        sort(procArray.begin(), procArray.end(), comp);
        
        for (int i = 0; i < procArray.size(); i++)
        {
            cout.width(3);
            cout << i;
            PrintInform(procArray.at(i));
            
        }
    } else {
        cout << "ProcessEntry32 iter error. Code: " << GetLastError() <<  endl;
    }
    
    cout << endl << "Choose an index of a process to be terminated: ";
    int killer;
    cin >> killer;
    cout << endl;
    
    DWORD procTermStatus = 2;
    GetExitCodeProcess(procArray.at(killer).handle, &procTermStatus);
    bool success = TerminateProcess(procArray.at(killer).handle, procTermStatus);
    
    for(int i = 0; i < procArray.size(); i++){
        CloseHandle(procArray.at(i).handle);
    }
    
    if(success)
        cout << "The process was terminated successfully!" << endl;
    else
        cout << "Error. Code for you to check out: " << GetLastError() << endl;
    
    system("pause");
    CloseHandle(hSnap);
    return 0;
}