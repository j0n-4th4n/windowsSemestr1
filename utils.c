#include "pslist.h"

void GetProcessName(HANDLE hProcess, char *processName, DWORD size) {
    if (!GetModuleBaseName(hProcess, NULL, processName, size)) {
        strncpy(processName, "<unknown>", size);
    }
}

DWORD GetThreadCount(DWORD processID) {
    DWORD threadCount = 0;
    THREADENTRY32 te = { sizeof(THREADENTRY32) };
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == processID) {
                threadCount++;
            }
        } while (Thread32Next(hSnapshot, &te));
    }
    CloseHandle(hSnapshot);
    return threadCount;
}

SIZE_T GetPrivateMemoryUsage(HANDLE hProcess) {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.PrivateUsage;
    }
    return 0;
}

ULONGLONG ConvertFileTimeToULONGLONG(const FILETIME *ft) {
    return (((ULONGLONG)ft->dwHighDateTime << 32) | ft->dwLowDateTime);
}

double GetElapsedTime(FILETIME creationTime) {
    ULONGLONG creationTimeUL = ConvertFileTimeToULONGLONG(&creationTime);

    FILETIME currentTime;
    GetSystemTimeAsFileTime(&currentTime);
    ULONGLONG currentTimeUL = ConvertFileTimeToULONGLONG(&currentTime);

    return (double)(currentTimeUL - creationTimeUL) / 10000000.0;
}

double GetTotalCPUTime(FILETIME kernelTime, FILETIME userTime) {
    ULONGLONG totalKernel = ConvertFileTimeToULONGLONG(&kernelTime);
    ULONGLONG totalUser = ConvertFileTimeToULONGLONG(&userTime);
    return (double)(totalKernel + totalUser) / 10000000.0;
}


// Helper function to convert time in seconds to hh:mm:ss format
void FormatTime(double timeInSeconds, char *formattedTime) {
    int hours = (int)(timeInSeconds / 3600);
    int minutes = (int)((timeInSeconds - hours * 3600) / 60);
    double seconds = timeInSeconds - hours * 3600 - minutes * 60;

    snprintf(formattedTime, 20, "%02d:%02d:%06.3f", hours, minutes, seconds);
}

HANDLE getSnapshot() 
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("Failed to take process snapshot.\n");
        return NULL;
    }
    return hSnapshot;
}

bool is_number(char *str)
{
    while (*str)
    {
        if ( !isdigit(*str) )
            return (0);
        str++;
    }
    return (1);
}

const char* TranslateThreadExitStatus(NTSTATUS status) {
    switch (status) {
        case 0x00000000: return "RUNNING";
        case 0x000000C0: return "TERMINATED"; 
        case 0xC0000005: return "CRASHED: ACCESS VIOLATION";
        case 0xC000001D: return "CRASHED: ILLEGAL INSTRUCTION";
        case 0xC0000096: return "CRASHED: PRIVILEGED INSTRUCTION";
        case 0xC000013A: return "TERMINATED BY CTRL+C";
        case 0xC000010A: return "PROCESS TERMINATING";
        case 0x40000015: return "SUSPENDED";
        case 0x00000103: return "WAITING";
        default: return "UNKNOWN STATUS";
    }
}

const char* GetErrorMessage(DWORD dwError) {
    static char errorMsg[512];  // Static so it persists after the function returns

    // Format the error message
    DWORD result = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwError,
        0, // Default language
        errorMsg,
        sizeof(errorMsg),
        NULL
    );

    if (result == 0) {
        // If FormatMessage fails, return a default message with the error code
        sprintf(errorMsg, "Error code: %lu", dwError);
    }

    return errorMsg;
}
