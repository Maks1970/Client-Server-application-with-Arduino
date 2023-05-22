#include <windows.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include "ARdu.h"

HANDLE hCOMPort = INVALID_HANDLE_VALUE;
OVERLAPPED ovlprd;     //����������� ������
OVERLAPPED ovlpwr;     //����������� ������
//=============================================================================
//.............................. ���������� ������� ...........................
//=============================================================================
HANDLE hreader = INVALID_HANDLE_VALUE;	//���������� ������ ������ �� �����
HANDLE hwriter = INVALID_HANDLE_VALUE;	//���������� ������ ������ � ����
DWORD WINAPI ReadThread(LPVOID);
DWORD WINAPI WriteThread(LPVOID);
void COMTerminate();  //������� ������ �� ���� ������

unsigned char bufrd[8], bufwr[1];
LPCTSTR sPortName = L"COM2";


using namespace std;
void sendStop(bool& i);
extern bool check;

//---------------------------------------------------------------------------
void COMTerminate(void) {
	if (hwriter != INVALID_HANDLE_VALUE) {
		TerminateThread(hwriter, 0);
		CloseHandle(ovlpwr.hEvent);	//����� ������� ������-�������
		CloseHandle(hwriter);
	}
	if (hreader != INVALID_HANDLE_VALUE) {
		TerminateThread(hreader, 0);
		CloseHandle(ovlprd.hEvent);	//����� ������� ������-�������
		CloseHandle(hreader);
	}
	hreader = INVALID_HANDLE_VALUE;
	hwriter = INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------------
void COMPortStartThreads(void) {
	//�������� ����������� ����� �����
	PurgeComm(hCOMPort, PURGE_RXCLEAR);
	//������ ����� ������, ������� ����� ����� �����������
	hreader = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	//������ ����� ������ � ������������� ���������
	hwriter = CreateThread(NULL, 0, WriteThread, NULL, CREATE_SUSPENDED, NULL);
}
//---------------------------------------------------------------------------
//������� ������� ������, ��������� �������� ������ �� ������ � COM-����
DWORD WINAPI WriteThread(LPVOID) {
	DWORD temp, signal;
	//������� �������
	ovlpwr.hEvent = CreateEvent(NULL, true, true, NULL);
	while (1) {
		//�������� ����� � ���� (������������� ��������!)
		WriteFile(hCOMPort, bufwr, 1, &temp, &ovlpwr);
		//������������� �����, ���� �� ���������� ������������� �������� WriteFile
		signal = WaitForSingleObject(ovlpwr.hEvent, INFINITE);
		//���� �������� ����������� �������
		if ((signal == WAIT_OBJECT_0) && (GetOverlappedResult(hCOMPort, &ovlpwr, &temp, true))) {
		}
		else {
		}
		SuspendThread(hwriter);
	}
}
//---------------------------------------------------------------------------
//������� ������� ������, ��������� ���� ������ �� COM-�����
DWORD WINAPI ReadThread(LPVOID) {
	COMSTAT comstat;		//��������� �������� ��������� �����
	DWORD btr, temp, mask, signal;
	//������� ���������� ������-������� ��� ����������� ��������
	ovlprd.hEvent = CreateEvent(NULL, true, true, NULL);
	//���������� ����� �� ������������ �� ������� ����� ����� � ����
	SetCommMask(hCOMPort, EV_RXCHAR);
	//���� ����� �� ����� �������, ��������� ����
	while (1) {
		//������� ������� ����� �����
		WaitCommEvent(hCOMPort, &mask, &ovlprd);
		//������������� ����� �� ������� �����
		signal = WaitForSingleObject(ovlprd.hEvent, INFINITE);
		//���� ������� ������� ����� ���������
		if (signal == WAIT_OBJECT_0) {
			//���������, ������� �� ����������� ������������� �������� WaitCommEvent
			if (GetOverlappedResult(hCOMPort, &ovlprd, &temp, true)) {
				//���� ��������� ������ ������� ������� �����
				if ((mask & EV_RXCHAR) != 0) {
					//����� ��������� ��������� COMSTAT
					ClearCommError(hCOMPort, &temp, &comstat);
					//� �������� �� �� ���������� �������� ������
					btr = comstat.cbInQue;
					//���� ������������� ���� ����� ��� ������
					if (btr) {
						//��������� ����� �� ����� � ����� ���������
						ReadFile(hCOMPort, bufrd, btr, &temp, &ovlprd);
						//������� ��������� �����
						ReadCOM(check);
						//�������� ����� (����� ������ �� ������������� ���� �� �����)
						memset(bufrd, 0, 8);
					}
				}
			}
		}
	}
}
//---------------------------------------------------------------------------
void ReadCOM(bool& i) {
	//SYSTEMTIME s;
	//GetLocalTime(&s);
	SYSTEMTIME s;
	GetLocalTime(&s);
	if ((i == true) && (bufrd[0] < 128)) {
		cout << "T=" << std::to_string(bufrd[0]) << "\n";
		ofstream out("Datatem.txt", std::ios::app);
		if (out.is_open())
		{
			out << "T=" << std::to_string(bufrd[0]) << "\n";;
			out << " " << s.wHour << ":" << s.wMinute << " " << s.wDay << "." << s.wMonth << "." << s.wYear << endl;
			out.close();
		}
		

	}
}
//---------------------------------------------------------------------------
bool conCom() {
	DCB dcb;                //��������� ��� ����� ������������� ����� DCB
	COMMTIMEOUTS timeouts;  //��������� ��� ��������� ���������
	//������� ����, ��� ����������� �������� ������� ���� FILE_FLAG_OVERLAPPED
	hCOMPort = CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	//���� ������ �������� �����
	if (hCOMPort == INVALID_HANDLE_VALUE) {
		wcout << "error CreateFile(sPortName) " << sPortName;
		return false;
	}
	//������������� �����
	dcb.DCBlength = sizeof(DCB);
	//������� ��������� DCB �� �����
	if (!GetCommState(hCOMPort, &dcb)) {
		CloseHandle(hCOMPort);                  //������� ����
		hCOMPort = INVALID_HANDLE_VALUE;
		wcout << "error GetCommState(hCOMPort) " << sPortName;
		return false;
	}
	//������������� ��������� DCB
	dcb.BaudRate = CBR_9600;				 //����� �������� �������� 115200 ���
	dcb.fBinary = TRUE;                     //�������� �������� ����� ������
	dcb.fOutxCtsFlow = FALSE;               //��������� ����� �������� �� �������� CTS
	dcb.fOutxDsrFlow = FALSE;               //��������� ����� �������� �� �������� DSR
	dcb.fDtrControl = DTR_CONTROL_DISABLE;  //��������� ������������� ����� DTR
	dcb.fDsrSensitivity = FALSE;            //��������� ��������������� �������� � ��������� ����� DSR
	dcb.fNull = FALSE;                      //��������� ���� ������� ������
	dcb.fRtsControl = RTS_CONTROL_DISABLE;  //��������� ������������� ����� RTS
	dcb.fAbortOnError = FALSE;              //��������� ��������� ���� �������� ������/������ ��� ������
	dcb.ByteSize = 8;                       //����� 8 ��� � �����
	dcb.Parity = 0;                         //��������� �������� ��������
	dcb.StopBits = 0;                       //����� ���� ����-���
	//��������� ��������� DCB � ����
	if (!SetCommState(hCOMPort, &dcb)) {
		CloseHandle(hCOMPort);                  //������� ����
		hCOMPort = INVALID_HANDLE_VALUE;
		wcout << "error SetCommState(hCOMPort) " << sPortName;
		return false;
	}
	//���������� ��������
	timeouts.ReadIntervalTimeout = 10;	 	   //������� ����� ����� ���������
	timeouts.ReadTotalTimeoutMultiplier = 0;  //����� ������� �������� ������
	timeouts.ReadTotalTimeoutConstant = 0;    //��������� ��� ������ �������� �������� ������
	timeouts.WriteTotalTimeoutMultiplier = 0; //����� ������� �������� ������
	timeouts.WriteTotalTimeoutConstant = 0;   //��������� ��� ������ �������� �������� ������
	//�������� ��������� ��������� � ����
	if (!SetCommTimeouts(hCOMPort, &timeouts)) {
		CloseHandle(hCOMPort);                  //������� ����
		hCOMPort = INVALID_HANDLE_VALUE;
		wcout << "error SetCommTimeouts(hCOMPort) " << sPortName;
		return false;
	}
	//���������� ������� �������� ����� � ��������
	SetupComm(hCOMPort, 16, 16);
	//�������� ����������� ����� �����
	PurgeComm(hCOMPort, PURGE_RXCLEAR);
	wcout << "connected COM port " << sPortName;
	return true;
}
//---------------------------------------------------------------------------
void sendStop(bool& i) {
	bufwr[0] = 0xCC;  // ������ ��� ��������
	if (hwriter != INVALID_HANDLE_VALUE) {
		ResumeThread(hwriter);
	}
	i = false;
}
//---------------------------------------------------------------------------
void sendStart(bool& i) {
	bufwr[0] = 0xEE;  // ������ ��� ��������
	if (hwriter != INVALID_HANDLE_VALUE) {
		ResumeThread(hwriter);
	}
	cout << "Started!" << endl;
	i = true;
}
//---------------------------------------------------------------------------
void sendT(int t) {
	bufwr[0] = (unsigned char)t;  // ������ ��� ��������
	if (hwriter != INVALID_HANDLE_VALUE) {
		ResumeThread(hwriter);
	}
	cout << "Control T=" << std::to_string(t) << endl;
}
//---------------------------------------------------------------------------


//35    0x33 0x35 0x0A      0x23