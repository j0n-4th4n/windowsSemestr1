#include "pslist.h"


const char* GetThreadStateFromTimes(HANDLE hThread) {
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetThreadTimes(hThread, &creationTime, &exitTime, &kernelTime, &userTime)) {
        if (CompareFileTime(&kernelTime, &userTime) == 0) {
            return "WAITING";
        }
    }
    return "RUNNING";
}


const char* GetThreadState(HANDLE hThread) {
    HMODULE hNtDll = LoadLibraryA("ntdll.dll");
    if (!hNtDll) {
        printf("Failed to load ntdll.dll.\n");
        return NULL;
    }

    NtQueryInformationThread_t NtQueryInformationThread = 
        (NtQueryInformationThread_t)GetProcAddress(hNtDll, "NtQueryInformationThread");
    if (!NtQueryInformationThread) {
        printf("Failed to retrieve NtQueryInformationThread function address.\n");
        FreeLibrary(hNtDll);
        return NULL;
    }

    THREAD_BASIC_INFORMATION tbi;
    NTSTATUS status = NtQueryInformationThread(hThread, ThreadBasicInformation, &tbi, sizeof(tbi), NULL);
    if (status == 0) { // STATUS_SUCCESS
        printf("Thread Exit Status: 0x%08X\n", tbi.ExitStatus);
        printf("Thread Base Priority: %d\n", tbi.BasePriority);
    } else {
        printf("Failed to get thread information. NTSTATUS: 0x%08X\n", status);
    }

    FreeLibrary(hNtDll);

    return (TranslateThreadExitStatus(status));
}


PVOID GetThreadEntryPoint(HANDLE hThread) 
{
    HMODULE hNtDll = LoadLibraryA("ntdll.dll");
    if (!hNtDll) 
    {
        printf("Failed to load ntdll.dll.\n");
        return NULL;
    }

    NtQueryInformationThread_t NtQueryInformationThread = (NtQueryInformationThread_t)GetProcAddress(hNtDll, "NtQueryInformationThread");
    if (!NtQueryInformationThread) 
    {
        printf("Failed to retrieve NtQueryInformationThread function address.\n");
        FreeLibrary(hNtDll);
        return NULL;
    }

    // Use the function pointer to call NtQueryInformationThread
    PVOID startAddress = NULL;
    NTSTATUS status = NtQueryInformationThread(
        hThread,
        ThreadQuerySetWin32StartAddress,
        &startAddress,
        sizeof(startAddress),
        NULL
    );

    FreeLibrary(hNtDll);

    if (status) 
    { 
        printf("Failed to get thread entry point. NTSTATUS: 0x%08lX\n", status);
        return NULL;
    }

    return startAddress;
}



void PrintThreadInfo(THREADENTRY32 te32, const char* processName) {
    HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
    if (hThread == NULL) {
        printf("%5u %3u %9s %9s %9s %11s %13s\n",
               te32.th32ThreadID, te32.tpBasePri, "-", "-", "-", "-", "-");
        return;
    }

    PVOID entry = GetThreadEntryPoint(hThread);
    DWORD dwError = GetLastError();
    const char *state = GetThreadStateFromTimes(hThread);

    char userTime[20] = "N/A";
    char kernelTime[20] = "N/A";

    // Retrieve thread times
    FILETIME creationTime, exitTime, kernelTimeFT, userTimeFT;
    if (GetThreadTimes(hThread, &creationTime, &exitTime, &kernelTimeFT, &userTimeFT)) {
        SYSTEMTIME kernelTimeST, userTimeST;
        FileTimeToSystemTime(&kernelTimeFT, &kernelTimeST);
        FileTimeToSystemTime(&userTimeFT, &userTimeST);
        sprintf(userTime, "%02d:%02d:%02d", userTimeST.wHour, userTimeST.wMinute, userTimeST.wSecond);
        sprintf(kernelTime, "%02d:%02d:%02d", kernelTimeST.wHour, kernelTimeST.wMinute, kernelTimeST.wSecond);
    }

    printf("%5u %4u  0x%p  %10lu %9s %11s %13s\n",
           te32.th32ThreadID, te32.tpBasePri, entry, dwError, state, userTime, kernelTime);

    CloseHandle(hThread);
}



void listThreads(HANDLE hSnapshot, PROCESSENTRY32 pe32)
{
    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hSnapshot, &te32)) {
        printf("Failed to get first thread.\n");
        CloseHandle(hSnapshot);
        return;
    }

    printf("%s %d:\n", pe32.szExeFile, pe32.th32ProcessID);
    printf("%5s %4s %11s  %10s %9s %11s %13s\n",
        "Tid", "Pri", "entrypoint", "Last error", "State", "User Time", "Kernel Time");

    do {
        if (pe32.th32ProcessID == te32.th32OwnerProcessID)
            PrintThreadInfo(te32, pe32.szExeFile);
    } while (Thread32Next(hSnapshot, &te32));
    printf("\n");
}
