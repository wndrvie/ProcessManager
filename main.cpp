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
        return process;
    }
    
    process.handle = hProcess;
    
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))){
        process.memory = pmc.WorkingSetSize;
    } else{
        process.memory = -1;
    }
    
    return process;
}

void PrintProcessInform(Process proc){
    cout.width(9);
    cout << proc.id;
    cout.width(30); 
    cout << proc.name;
    cout.width(12);
    if(proc.memory != -1)
        cout << proc.memory;
    else
        cout << "no access";
}

void PrintIntro(){
    cout << " No";
    cout.width(9);
    cout << "PID";
    cout.width(30); 
    cout << "name.exe";
    cout.width(12);
    cout << "RAM" << endl << endl;
}

void PrintProcessArray(vector<Process> procArray){
    cout << "Processes: " << procArray.size() << endl << endl;
    
    PrintIntro();    
    
    for (int i = 0; i < procArray.size() - 3; i++){            
        cout.width(3);
        cout << i;
        PrintProcessInform(procArray.at(i));
        cout << endl;
    }

    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, BACKGROUND_RED | FOREGROUND_INTENSITY);
    for (int i = procArray.size() - 3; i < procArray.size(); i++){            
        cout.width(3);
        cout << i;
        PrintProcessInform(procArray.at(i));
        cout << endl;
        }
    SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | 
            FOREGROUND_BLUE | FOREGROUND_GREEN);
    
    cout << endl;
}

vector<Process> CreateCurrentProcessArray(){
    vector<Process> emptyVector;
    HANDLE hSnap;
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == NULL)
    {
        cout << "Error: processes snapshot hasn't been taken." << endl;
        return emptyVector;
    }

    PROCESSENTRY32 proc;
    proc.dwSize = sizeof(PROCESSENTRY32);
    vector<Process> procArray;

    if (Process32First(hSnap, &proc))
    {
        do{
            procArray.push_back(CreateProcessInfo(proc));
        }while (Process32Next(hSnap, &proc));
    }
    else{
        cout << "ProcessEntry32 iterator error. Code: " << GetLastError() <<  endl;
        return emptyVector;
    }
    
    sort(procArray.begin(), procArray.end(), comp);
    return procArray;
}

void AskUserChoice(){
    cout << "Anything else? Press 0 to terminate another process," << endl <<
                "1 to terminate top-3 of the heaviest processes," << endl <<
                "2 to exit the program." << endl;
}

int main(int argc, char* argv[])
{   
    //dialogue with user
    int checker = 0;
    while(checker != 2){
        switch(checker){
            case 0:
            {
                vector<Process> procArray = CreateCurrentProcessArray();
                PrintProcessArray(procArray);
                cout << endl << "Choose an index of a process to be terminated: ";
                int killer;
                cin >> killer;
                cout << endl;

                DWORD procTermStatus = 1;
                GetExitCodeProcess(procArray.at(killer).handle, &procTermStatus);
                if(TerminateProcess(procArray.at(killer).handle, procTermStatus))
                    cout << "The process was terminated successfully!" << endl;
                else
                    cout << "Error. Its code for you to check out: " << GetLastError() << endl;
                
                AskUserChoice();
                        
                cin >> checker;
                if(checker != 2)
                    system("cls");
                break;
            }

            case 1: 
            {
                vector<Process> procArray = CreateCurrentProcessArray();
                DWORD procTermStatus = 1;
                bool success = true;
                for(int i = 0; i < 3; i++){
                    GetExitCodeProcess(procArray.back().handle, &procTermStatus);
                    i++;
                    if(TerminateProcess(procArray.back().handle, procTermStatus)){
                        procArray.erase(procArray.end());
                    }
                    else{
                        success = false;
                        break;
                    }     
                }
                
                PrintProcessArray(procArray);
                if(success){
                    cout << "Top-3 processes were terminated successfully!" << endl;
                    AskUserChoice();
                }
                else{
                    cout << "Something went wrong." << endl;
                    AskUserChoice();
                }
                
                cin >> checker;
                if(checker != 2)
                    system("cls");
                break;
            }
        }
    }
    
    system("pause");
    return 0;
}
