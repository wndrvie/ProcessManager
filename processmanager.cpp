#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <psapi.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

struct Process{
    unsigned long int id; 
    string name;
    HANDLE handle;
    long int memory;
};

struct Comparator{
    bool operator()(const Process& pr1, const Process& pr2){
        if(pr1.memory < pr2.memory){
            return true;
        }
        else 
            return false;
    }
} comp;

class ProcessArray {
private:
    vector<Process> procArray;
    
public:
    bool createArray(){
        HANDLE hSnap;
        hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap == NULL){
            cout << "Error: processes snapshot hasn't been taken." << endl;    
            return false;
        }

        PROCESSENTRY32 proc;
        proc.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnap, &proc)){
            do{
                procArray.push_back(collectInfo(proc));
            }while (Process32Next(hSnap, &proc));
        }
        else{
            cout << "ProcessEntry32 iterator error. Code: " << GetLastError() <<  endl;
            return false;
        }

        sort(procArray.begin(), procArray.end(), comp);
        CloseHandle(hSnap);
        return true;
    }
    
    Process collectInfo(PROCESSENTRY32 proc)
    {
        HANDLE hProcess;
        PROCESS_MEMORY_COUNTERS pmc;
        Process process;
        process.name = proc.szExeFile;
        process.id = proc.th32ProcessID;

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
    
    void print(){
        cout << "Processes: " << procArray.size() << endl 
                << "Hit H for Help." << endl << endl;
    
        cout << " No";
        cout.width(9);
        cout << "PID";
        cout.width(30); 
        cout << "name.exe";
        cout.width(12);
        cout << "RAM" << endl << endl;   

        for (int i = 0; i < procArray.size() - 3; i++){            
            cout.width(3);
            cout << i;
            printProcess(procArray.at(i));
            cout << endl;
        }

        SetConsoleTextAttribute(hStdOut, BACKGROUND_RED | FOREGROUND_INTENSITY);
        for (int i = procArray.size() - 3; i < procArray.size(); i++){            
            cout.width(3);
            cout << i;
            printProcess(procArray.at(i));
            cout << endl;
            }
        
        SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | 
                FOREGROUND_BLUE | FOREGROUND_GREEN);

        cout << endl;
    }
    
    void printProcess(Process proc){
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
    
    bool terminateAt(int index){
        DWORD procTermStatus;
        GetExitCodeProcess(procArray.at(index).handle, &procTermStatus);
        DWORD out = TerminateProcess(procArray.at(index).handle, procTermStatus);
        if(out != 0){
            cout << "The process was terminated successfully!" << endl;
            return true;
        }
        else{
            cout << "Error: process cannot be terminated. Code: " << GetLastError() << endl;
            return false;
        }           
    }
    
    bool terminateAt(int begin, int end){
        if(begin > end){
            int k = end;
            end = begin;
            begin = k;
        }
        else{
            for(int i = begin; i <= end; i++){
                if(!terminateAt(i)){
                    cout << "Error: cannot terminate some of the chosen processes." << endl;
                    return false;
                }                    
            }
            return true;
        }             
    }
    
    ~ProcessArray(){
        for (int i = 0; i < procArray.size(); i++){            
            CloseHandle(procArray.at(i).handle);
        }
    }
};

int main(int argc, char* argv[])
{   
    //dialogue with user
    char choice = 'R';
    while(choice != 'E'){
        switch(choice){
            case 'T':
            {
                ProcessArray procArray;
                if(procArray.createArray()){
                    procArray.print(); 
                    cout << "Terminating a process." << endl
                            << "Index: ";
                    int index;
                    cin >> index;
                    procArray.terminateAt(index);
                    cout << endl << "Next action: ";
                    cin >> choice;
                    system("cls");
                    break;
                }
                cout <<  "Next action: ";
                cin >> choice;
                system("cls");
                break;
            }

            case 'S': 
            {
                ProcessArray procArray;
                if(procArray.createArray()){
                    procArray.print(); 
                    cout << "Terminating a set processes." << endl 
                            << "From index ";
                    int begin;
                    cin >> begin;
                    cout << "to index ";
                    int end;
                    cin >> end;
                    procArray.terminateAt(begin, end);
                    cout << endl << "Next action: ";
                    cin >> choice;
                    system("cls");
                    break;
                }
                cout <<  "Next action: ";
                cin >> choice;
                system("cls");
                break;
            }
            
            case 'H':
            {
                cout << "Welcome to ProcessManager!" << endl 
                        << "Shortkeys:" << endl 
                        << "T: terminate process with its index(shown at column No)" << endl
                        << "S: terminate a set of processes from index1 to index2"<< endl
                        << "R: refresh" << endl
                        << "E: exit the program" << endl 
                        << "Your choice: ";
                cin >> choice;
                cout << endl;
                break;
            }
            
            case 'R':
            {
                ProcessArray procArray;
                if(procArray.createArray()){
                    procArray.print(); 
                    cout << endl << "Next action: ";
                    cin >> choice;
                    system("cls");
                    break;
                }
                cout <<  "Next action: ";
                cin >> choice;
                system("cls");
                break;
            }
            
            default:
            {
                ProcessArray procArray;
                if(procArray.createArray()){
                    procArray.print();                    
                    cout << "Invalid request. Try once again: ";
                    cin >> choice;
                    system("cls");
                    break;
                }
                cout << "Invalid request. Try once again: ";
                cin >> choice;
                system("cls");
                break;
            }
        }
    }
    
    cout << "Bye! :)" << endl;
    system("pause");
    CloseHandle(hStdOut);
    return 0;
}
