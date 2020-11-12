//
// Created by sdong on 2020/11/11.
//


#include <windows.h>
#include <atlstr.h>
#include <iostream>

using namespace std;
/*
 * http://members.ee.net/brey/Serial.pdf
 *
 * https://docs.microsoft.com/en-us/previous-versions/ms810467(v=msdn.10)?redirectedfrom=MSDN
 *
 *
 * https://doc.qt.io/qt-5/qserialport.html#baudRate-prop
 **/
#if 0
bool WriteComPort(CString PortSpecifier, CString data)
{
    DCB dcb;
    DWORD byteswritten;
    HANDLE hPort = CreateFile(

            PortSpecifier,
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
    );
    if (!GetCommState(hPort,&dcb))
        return false;
    dcb.BaudRate = CBR_9600; //9600 Baud
    dcb.ByteSize = 8; //8 data bits
    dcb.Parity = NOPARITY; //no parity
    dcb.StopBits = ONESTOPBIT; //1 stop
    if (!SetCommState(hPort,&dcb))
        return false;
    bool retVal = WriteFile(hPort,data,1,&byteswritten,NULL);
    CloseHandle(hPort); //close the handle
    return retVal;
}

int ReadByte(CString PortSpecifier)
{
    DCB dcb;
    int retVal;
    BYTE Byte;
    DWORD dwBytesTransferred;
    DWORD dwCommModemStatus;
    HANDLE hPort = CreateFile(

            PortSpecifier,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
    );
    if (!GetCommState(hPort,&dcb))
        return 0x100;
    dcb.BaudRate = CBR_9600; //9600 Baud
    dcb.ByteSize = 8; //8 data bits
    dcb.Parity = NOPARITY; //no parity
    dcb.StopBits = ONESTOPBIT; //1 stop
    if (!SetCommState(hPort,&dcb))
        return 0x100;
    SetCommMask (hPort, EV_RXCHAR | EV_ERR); //receive character event
    WaitCommEvent (hPort, &dwCommModemStatus, 0); //wait for character
    if (dwCommModemStatus & EV_RXCHAR)
        ReadFile (hPort, &Byte, 1, &dwBytesTransferred, 0); //read 1
    else if (dwCommModemStatus & EV_ERR)
        retVal = 0x101;
    retVal = Byte;
    CloseHandle(hPort);
    return retVal;
}


int main(){
    if(WriteComPort("COM4", "helllo")){
        cout << "ok!" << endl;
    }else{
        cout << "failed!" << endl;
    }

    return 0;
}
#endif


//https://docs.microsoft.com/en-us/previous-versions/ff802693(v=msdn.10)?redirectedfrom=MSDN
struct WinSerial{
    HANDLE hComm;

    void open(){
        hComm;
        hComm = CreateFile( "COM4",
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            0,
                            OPEN_EXISTING,
                            FILE_FLAG_OVERLAPPED,
                            0);
        if (hComm == INVALID_HANDLE_VALUE){
            // error opening port; abort
            cout << "can't open" << endl;
            return;
        }

    }
#define READ_TIMEOUT      500      // milliseconds

    void read(uint8_t* lpBuf, int len){
        DWORD dwRead;
        BOOL fWaitingOnRead = TRUE;//FALSE;
        OVERLAPPED osReader = {0};

        // Create the overlapped event. Must be closed before exiting
        // to avoid a handle leak.
        osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        if (osReader.hEvent == NULL){
            // Error creating overlapped event; abort.

        }

        if (!fWaitingOnRead) {
            // Issue read operation.
            if (!ReadFile(hComm, lpBuf, len, &dwRead, &osReader)) {
                if (GetLastError() != ERROR_IO_PENDING){
                    // read not delayed?
                    cout << "failed" << endl;

                }
                else{
                    // Error in communications; report it.
                    cout << "failed" << endl;

                }
                fWaitingOnRead = TRUE;
            }
            else {
                // read completed immediately
//                HandleASuccessfulRead(lpBuf, dwRead);
                cout << "done" << endl;
            }
        }

        DWORD dwRes;

        if (fWaitingOnRead) {
            dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
            switch(dwRes)
            {
                // Read completed.
                case WAIT_OBJECT_0:
                    if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE)){
                        // Error in communications; report it.

                    }
                    else{
                        // Read completed successfully.
                        cout << "failed" << endl;
                    }


                    //  Reset flag so that another opertion can be issued.
                    fWaitingOnRead = FALSE;
                    break;

                case WAIT_TIMEOUT:
                    // Operation isn't complete yet. fWaitingOnRead flag isn't
                    // changed since I'll loop back around, and I don't want
                    // to issue another read until the first one finishes.
                    //
                    // This is a good time to do some background work.
                    cout << "timeout" << endl;
                    break;

                default:
                    // Error in the WaitForSingleObject; abort.
                    // This indicates a problem with the OVERLAPPED structure's
                    // event handle.
                    break;
            }
        }
    }

    BOOL write(char * lpBuf, DWORD dwToWrite)
    {
        OVERLAPPED osWrite = {0};
        DWORD dwWritten;
        DWORD dwRes;
        BOOL fRes;

        // Create this write operation's OVERLAPPED structure's hEvent.
        osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (osWrite.hEvent == NULL)
            // error creating overlapped event handle
            return FALSE;

        // Issue write.
        if (!WriteFile(hComm, lpBuf, dwToWrite, &dwWritten, &osWrite)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                cout << "error" << endl;
                // WriteFile failed, but isn't delayed. Report error and abort.
                fRes = FALSE;
            }
            else{
                // Write is pending.
                dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);

                switch(dwRes)
                {
                    // OVERLAPPED structure's event has been signaled.
                    case WAIT_OBJECT_0:
                        if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, FALSE))
                            fRes = FALSE;
                        else
                            // Write operation completed successfully.
                            fRes = TRUE;
                        break;

                    default:
                        // An error has occurred in WaitForSingleObject.
                        // This usually indicates a problem with the
                        // OVERLAPPED structure's event handle.
                        fRes = FALSE;
                        break;
                }
            }
        }
        else{

            // WriteFile completed immediately.
            fRes = TRUE;
        }

        CloseHandle(osWrite.hEvent);
        return fRes;
    }

};

uint8_t recv_buf[2];

int main (void){

    WinSerial s;

    s.open();


    for(;;){
        s.write("foobar", 1);
        s.read(recv_buf, 2);
    }

    return 0;
}
