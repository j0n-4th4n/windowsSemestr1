#ifndef PSLIST_H
#   define PSLIST_H

#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <winternl.h> 

#define ThreadBasicInformation 0
#define ThreadTimes 1
#define ThreadQuerySetWin32StartAddress 9

typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PVOID TebBaseAddress;
    CLIENT_ID ClientId;
    KAFFINITY AffinityMask;
    KPRIORITY Priority;
    LONG BasePriority;
} THREAD_BASIC_INFORMATION;

// Define the function pointer type for NtQueryInformationThread
typedef NTSTATUS(NTAPI* NtQueryInformationThread_t)(
    HANDLE ThreadHandle,
    THREADINFOCLASS ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength
);

typedef struct {
    bool showThread;
    char *process;
    int processId;
} Args;



/**
 * utils.c
 */
// Function to retrieve the name of a process
void GetProcessName(HANDLE hProcess, char *processName, DWORD size);
// Function to count the number of threads associated with a process
DWORD GetThreadCount(DWORD processID);
// Function to retrieve the private memory usage of a process
SIZE_T GetPrivateMemoryUsage(HANDLE hProcess);
// Helper function to convert FILETIME to ULONGLONG
ULONGLONG ConvertFileTimeToULONGLONG(const FILETIME *ft);
// Function to calculate the elapsed time since process creation
double GetElapsedTime(FILETIME creationTime);
// Function to calculate the total CPU time (Kernel + User)
double GetTotalCPUTime(FILETIME kernelTime, FILETIME userTime);
// Function to print detailed information about a process
void PrintProcessInfo(PROCESSENTRY32 pe32);
void FormatTime(double timeInSeconds, char *formattedTime);
HANDLE getSnapshot();
bool is_number(char *str);
const char* TranslateThreadExitStatus(NTSTATUS status);
const char* GetErrorMessage(DWORD dwError);


/** 
 * init.c
 */
void usage();
bool EnableDebugPrivilege();
bool parse_args(int argc, char **argv, Args *args);

/**
 * thread.c
 */
void PrintThreadInfo(THREADENTRY32 te32, const char* processName);
void listThreads(HANDLE hSnapshot, PROCESSENTRY32 pe32);



#endif // PSLIST_H
