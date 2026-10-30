#include <MetroTRK.h>

#include <revolution/OS/OSInterrupt.h>

void MWInitializeCriticalSection(void* criticalSection) {
}

void MWEnterCriticalSection(void* criticalSection) {
    BOOL interruptsDisabled;

    interruptsDisabled = OSDisableInterrupts();
    *(BOOL*)criticalSection = interruptsDisabled;
}

void MWExitCriticalSection(void* criticalSection) {
    OSRestoreInterrupts(*(BOOL*)criticalSection);
}
