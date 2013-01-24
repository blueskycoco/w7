//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
#include "SPIApp.h"
#include <stdlib.h>
#include <s3c6410_spi.h>


HINSTANCE g_hInst;

BOOL APIENTRY CALLBACK DlgProc
(
 HWND    hWnd,
 UINT    message,
 WPARAM  wParam,
 LPARAM  lParam
 );


#define DMA_CE60

//#define READ_TEST

unsigned char *g_buffer;

static BYTE pTestBuffer[] = {
 0x01,0x02,0x03 ,0x04 ,0x05 ,0x06 ,0x07 ,0x08 ,0x09 ,0x0a ,0x0b ,0x0c ,0x0d,0x0e,0x0f ,0x10 ,
 0x11,0x12,0x13 ,0x14 ,0x15 ,0x16 ,0x17 ,0x18 ,0x19 ,0x1A ,0x1B ,0x1C ,0x1D,0x1E,0x1F ,0x20 ,
 0x21,0x22,0x23 ,0x24 ,0x25 ,0x26 ,0x27 ,0x28 ,0x29 ,0x2a ,0x2b ,0x2c ,0x2d,0x2e,0x2f ,0x30 ,
 0x31,0x32,0x33 ,0x34 ,0x35 ,0x36 ,0x37 ,0x38 ,0x39 ,0x3A ,0x3B ,0x3C ,0x3D,0x3E,0x3F ,0x40 ,
 //64
};


#define TEST_BUFFER_SIZE sizeof(pTestBuffer)
#define TIME_MASTERREAD 100
#define MAX_STRING_SIZE 200

UINT    g_nReadCount=0;
HANDLE  g_hThreadAutoRead = NULL;
BOOL    g_ThreadStop = FALSE;
HWND    g_hwndList = NULL;
HANDLE  g_hSpi = INVALID_HANDLE_VALUE;


void CALLBACK ThreadWaitRead(LPVOID pVoid)
{
    int i=0;
    TCHAR szString[MAX_STRING_SIZE];
    DWORD   count = 0;
    bool success = true;

    while(!g_ThreadStop)
    {
        g_nReadCount++;

        memset(g_buffer,0x0,TEST_BUFFER_SIZE);
        
        ReadFile(g_hSpi, g_buffer, TEST_BUFFER_SIZE, &count, NULL);

#ifdef  READ_TEST
        if(count >0)
        {
            for(i=0; i<count; ++i)
                RETAILMSG(1,(TEXT("[SLAVEAP] APP Buffer : %d 0x%X \t"), i,g_buffer[i]));
        }
        else
        {   
            RETAILMSG(1, (TEXT("[SLAVEAP] ReadFile[%d] count=0"),g_nReadCount) );
        }
        RETAILMSG(1, (TEXT("\n")) );
#endif

        if (count > 0)
        {
            for(i=0; i<TEST_BUFFER_SIZE; ++i)
            {
                if(pTestBuffer[i] != g_buffer[i])
                {
                    success = false;
                    i++;
                    break;
                }
            }

            _stprintf_s(szString,_countof(szString), TEXT("Read(Count %d) : %s %d"), count, success ? TEXT("SUCCESS") : TEXT("FAILED"), i);
            SendMessage(g_hwndList, LB_INSERTSTRING, 0, (LPARAM )szString);
            
            DEBUGMSG(1, (TEXT("[SLAVEAP] ReadFile count= %d\n"),count) );

            //Read data and write back to master for data verified
            if(success)
            {
                WriteFile(g_hSpi, g_buffer, TEST_BUFFER_SIZE, &count, NULL);

                _stprintf_s(szString,_countof(szString), TEXT("Write(Count %d) : %s %d"), count, success ? TEXT("SUCCESS") : TEXT("FAILED"), i);
                SendMessage(g_hwndList, LB_INSERTSTRING, 0, (LPARAM )szString);
            }
        }
        Sleep(TIME_MASTERREAD);
    }
}


BOOL SPIAP_Open() 
{
    BOOL fRetVal = TRUE;
    TCHAR szString[MAX_STRING_SIZE];

    g_hSpi = CreateFile( L"SPI1:",
                             GENERIC_READ|GENERIC_WRITE,
                             FILE_SHARE_READ|FILE_SHARE_WRITE,
                             NULL, OPEN_EXISTING, 0, 0);
                             
    if ( g_hSpi == INVALID_HANDLE_VALUE ) 
    {
        _stprintf_s(szString,_countof(szString), TEXT("SPI FILE OPEN FAILED"));
        fRetVal = FALSE;
    }
    else 
    {
        _stprintf_s(szString,_countof(szString), TEXT("SPI FILE OPEN SUCCESS"));
        g_buffer = (unsigned char *) malloc(TEST_BUFFER_SIZE);
        if (NULL == g_buffer)
        {
            _stprintf_s(szString,_countof(szString), TEXT("SPI Buffer Alloc FAILED"));
            fRetVal = FALSE;
        }
    }
    
    SendMessage(g_hwndList, LB_INSERTSTRING, 0, (LPARAM )szString);
    return fRetVal;
}


BOOL SPIAP_Config() 
{
    BOOL fRetVal = TRUE;
    SET_CONFIG SetConfig;
    TCHAR szString[MAX_STRING_SIZE];

    SetConfig.bUseRxDMA = FALSE;
    SetConfig.bUseRxIntr = TRUE;
    SetConfig.bUseTxDMA = FALSE;
    SetConfig.bUseTxIntr = TRUE;
    SetConfig.dwMode = SPI_SLAVE_MODE;
    SetConfig.dwPrescaler = 0;

    if(DeviceIoControl(g_hSpi, SPI_IOCTL_SET_CONFIG, &SetConfig, sizeof(SetConfig), NULL, NULL, NULL, NULL)) 
    {
        _stprintf_s(szString, _countof(szString),TEXT("DEVICE IOCONTROL (SetConfig) SUCCESS"));

        if(DeviceIoControl(g_hSpi, SPI_IOCTL_START, NULL, NULL, NULL, NULL, NULL, NULL)) 
        {
            _stprintf_s(szString,_countof(szString), TEXT("DEVICE IOCONTROL (Start) SUCCESS"));
        }
        else 
        {
            _stprintf_s(szString, _countof(szString),TEXT("DEVICE IOCONTROL (Start) FAILED"));
            fRetVal = FALSE;
        }
    }
    else 
    {
        _stprintf_s(szString,_countof(szString), TEXT("DEVICE IOCONTROL (SetConfig) FAILED"));
        fRetVal = FALSE;
    }

    SendMessage(g_hwndList, LB_INSERTSTRING, 0, (LPARAM )szString);
    return fRetVal;
}


// Create the main window
HWND InitInstance(int nCmdShow)
{
    HWND hWnd;

    hWnd = CreateDialog( g_hInst, MAKEINTRESOURCE( IDD_SPIAPP2_DIALOG ), NULL, DlgProc );
    if (hWnd)
    {
        ShowWindow (hWnd, nCmdShow);
        UpdateWindow (hWnd);
    }
    
    return hWnd;
}


BOOL APIENTRY CALLBACK DlgProc
(
 HWND    hWnd,
 UINT    message,
 WPARAM  wParam,
 LPARAM  lParam
 )
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            g_hwndList = GetDlgItem(hWnd, IDC_LIST);
            
            if (SPIAP_Open() &&
                SPIAP_Config())
            {
                g_hThreadAutoRead= CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadWaitRead, NULL, 0, NULL);            
            }
        }
        return TRUE;
        break;
        
    case WM_COMMAND:
        {
            UINT    wCommand = LOWORD(wParam);
            switch (wCommand)
            {
            case IDOK:
            case IDCANCEL:
                g_ThreadStop = TRUE;
                EndDialog(hWnd,1);
                PostQuitMessage(1);
                break;
            }
        }
        break;
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR lpCmdLine,
                   int nCmdShow)
{
    MSG msg;
    HWND hWnd;
    
    g_hInst=hInstance;

    hWnd = InitInstance(nCmdShow);
    if (!hWnd)
    {
        RETAILMSG(1, (TEXT("Unable to Init Instance\r\n")));
        return FALSE;
    }
     
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (INVALID_HANDLE_VALUE != g_hSpi)
    {
        CloseHandle(g_hSpi);
    }

    return msg.wParam;

}



