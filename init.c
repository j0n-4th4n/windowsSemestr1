#include "pslist.h"


void usage() {
    printf("Usage: pslist.exe [-d] [name|pid]\n");
    printf("Options:\n");
    printf("  -d                Show thread details\n");
    printf("\n");
    printf("name       The name of the process to filter (optional).\n");
    printf("pid        The process's pid (optional).\n");
}



bool EnableDebugPrivilege() 
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return FALSE;
    }
    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
    tp.PrivilegeCount = 10;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
    CloseHandle(hToken);
    return (GetLastError() == ERROR_SUCCESS);
}



bool parse_args(int argc, char **argv, Args *args)
{
    args->showThread = FALSE;
    args->processId = -1;
    args->process = NULL;
    
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || argc > 3)) 
    {
        usage();
        return (0);
    }

    int i = 1;
    while ( argv[i] )
    {
        if (strcmp(argv[i], "-d") == 0)
            args->showThread = TRUE;
        else
        {
            if ( is_number(argv[i]) )
                args->processId = atoi(argv[i]);
            else
                args->process = argv[i];
        }
        i++;
    }
    return (1);
}
