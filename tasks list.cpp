#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>

using namespace std;

struct ProcessInfo {
    DWORD processID;
    string processName;
};

DWORD getExplorerPID() {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        cerr << "Failed to take a process snapshot!" << endl;
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe32)) {
        do {
            if (string(pe32.szExeFile) == "explorer.exe") {
                CloseHandle(hSnap);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(hSnap, &pe32));
    }

    CloseHandle(hSnap);
    return 0;
}

void listUserTasks(vector<ProcessInfo>& processList) {
    DWORD explorerPID = getExplorerPID();
    if (explorerPID == 0) {
        cerr << "Failed to find Explorer.exe process!" << endl;
        return;
    }

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        cerr << "Failed to take a process snapshot!" << endl;
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe32)) {
        int serial = 1;
        do {
            if (pe32.th32ParentProcessID == explorerPID) {
                processList.push_back({pe32.th32ProcessID, pe32.szExeFile});
                cout << serial++ << ". " << pe32.szExeFile << " [PID: " << pe32.th32ProcessID << "]" << endl;
            }
        } while (Process32Next(hSnap, &pe32));
    } else {
        cerr << "Failed to retrieve process information!" << endl;
    }

    CloseHandle(hSnap);
}

void closeTask(const ProcessInfo& process) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, process.processID);
    if (hProcess == NULL) {
        DWORD error = GetLastError();
        cerr << "Failed to open process for termination: " << process.processName
             << " (Error Code: " << error << ")" << endl;
        return;
    }

    if (TerminateProcess(hProcess, 0)) {
        cout << "Successfully terminated: " << process.processName << endl;
    } else {
        DWORD error = GetLastError();
        cerr << "Failed to terminate: " << process.processName
             << " (Error Code: " << error << ")" << endl;
    }

    CloseHandle(hProcess);
}

int main() {
    while (true) {
        vector<ProcessInfo> processList;
        cout << "\n=== User-Opened Tasks ===" << endl;
        listUserTasks(processList);

        cout << "\nEnter the serial number of the task to close (0 to exit): ";
        int choice;
        cin >> choice;

        if (choice == 0) {
            cout << "Exiting program..." << endl;
            break;
        }

        if (choice < 1 || choice > processList.size()) {
            cerr << "Invalid choice!" << endl;
            continue;
        }

        closeTask(processList[choice - 1]);
    }

    return 0;
}
