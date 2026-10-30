#include <MetroTRK.h>

BOOL usr_puts_serial(const char* msg) {
    BOOL connect_ = FALSE;
    char c;
    char buf[2];

    while (!connect_ && (c = *msg++) != '\0') {
        BOOL connected = GetTRKConnected();

        buf[0] = c;
        buf[1] = '\0';

        SetTRKConnected(FALSE);
        OSReport(buf);
        SetTRKConnected(connected);
        connect_ = FALSE;
    }

    return connect_;
}
