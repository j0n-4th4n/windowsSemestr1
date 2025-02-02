#include "pslist.h"


void PrintProcessInfo(PROCESSENTRY32 pe32) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
    if (hProcess == NULL) {
        printf("%-20s %5u %3s %3s %5s %10s     %12s    %12s\n",
           pe32.szExeFile, pe32.th32ProcessID, "-", "-", "-", "-", "-", "-");
        return;
    }


    DWORD priority = GetPriorityClass(hProcess);
    if (priority == 0) {
        priority = -1;  // Indicate error
    }

    DWORD threadCount = pe32.cntThreads;

    DWORD handleCount = 0;
    if (!GetProcessHandleCount(hProcess, &handleCount)) {
        handleCount = 0;  // Handle error
    }

    SIZE_T privateMemory = 0;
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        privateMemory = pmc.WorkingSetSize / 1024; // Convert to KB
    }

    char cpuTime[20] = "N/A";
    char elapsedTime[20] = "N/A";

    FILETIME kernelTime, userTime, creationTime, exitTime;
    if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
        double d_cpuTime = GetTotalCPUTime(kernelTime, userTime);
        double d_elapsedTime = GetElapsedTime(creationTime);
        FormatTime(d_cpuTime, cpuTime);
        FormatTime(d_elapsedTime, elapsedTime);
    }

    printf("%-20s %5u %3u %3u %5u %10zu     %12s    %12s\n",
           pe32.szExeFile, pe32.th32ProcessID, priority, threadCount, handleCount,
           privateMemory, cpuTime, elapsedTime);

    CloseHandle(hProcess);
}



void listProcess(Args args) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = getSnapshot();

    if (!Process32First(hSnapshot, &pe32)) {
        printf("Failed to get first process.\n");
        CloseHandle(hSnapshot);
        return;
    }

    // Search for the process with the matching PID
    do {
        if (pe32.th32ProcessID == args.processId) 
        {
            if (args.showThread)
                listThreads(hSnapshot, pe32);
            else
                PrintProcessInfo(pe32);

            CloseHandle(hSnapshot);
            return;
        }
    } while (Process32Next(hSnapshot, &pe32));

    printf("Process with PID %d not found.\n", args.processId);
    CloseHandle(hSnapshot);
}



void listProcesses(Args args) {
    HANDLE hSnapshot = getSnapshot();
    if (hSnapshot == NULL) {
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        printf("Failed to get first process.\n");
        CloseHandle(hSnapshot);
        return;
    }

    do {
        if (args.process == NULL || _stricmp(pe32.szExeFile, args.process) == 0) 
        {
            if (args.showThread)
                listThreads(hSnapshot, pe32);
            else
                PrintProcessInfo(pe32);
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
}



int main(int argc, char **argv) 
{
    Args arguments;

    EnableDebugPrivilege();
    if ( !parse_args(argc, argv, &arguments) )
        return (1);


    if (!arguments.showThread)
        printf("%-20s %5s %3s %3s %5s %10s %16s %15s\n",
            "Name", "PID", "Pri", "Thd", "Hnd", "Priv", "CPU Time", "Elapsed Time");


    if ( arguments.processId > -1 )
        listProcess(arguments);
    else
        listProcesses(arguments);

    return 0;
}