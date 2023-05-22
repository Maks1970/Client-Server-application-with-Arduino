#include <windows.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include "ARdu.h"

HANDLE hCOMPort = INVALID_HANDLE_VALUE;
OVERLAPPED ovlprd;     //асинхронное чтение
OVERLAPPED ovlpwr;     //асинхронная запись
//=============================================================================
//.............................. объявления потоков ...........................
//=============================================================================
HANDLE hreader = INVALID_HANDLE_VALUE;	//дескриптор потока чтения из порта
HANDLE hwriter = INVALID_HANDLE_VALUE;	//дескриптор потока записи в порт
DWORD WINAPI ReadThread(LPVOID);
DWORD WINAPI WriteThread(LPVOID);
void COMTerminate();  //прибить потоки но порт открыт

unsigned char bufrd[8], bufwr[1];
LPCTSTR sPortName = L"COM2";


using namespace std;
void sendStop(bool& i);
extern bool check;

//---------------------------------------------------------------------------
void COMTerminate(void) {
	if (hwriter != INVALID_HANDLE_VALUE) {
		TerminateThread(hwriter, 0);
		CloseHandle(ovlpwr.hEvent);	//нужно закрыть объект-событие
		CloseHandle(hwriter);
	}
	if (hreader != INVALID_HANDLE_VALUE) {
		TerminateThread(hreader, 0);
		CloseHandle(ovlprd.hEvent);	//нужно закрыть объект-событие
		CloseHandle(hreader);
	}
	hreader = INVALID_HANDLE_VALUE;
	hwriter = INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------------
void COMPortStartThreads(void) {
	//очистить принимающий буфер порта
	PurgeComm(hCOMPort, PURGE_RXCLEAR);
	//создаём поток чтения, который сразу начнёт выполняться
	hreader = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	//создаём поток записи в остановленном состоянии
	hwriter = CreateThread(NULL, 0, WriteThread, NULL, CREATE_SUSPENDED, NULL);
}
//---------------------------------------------------------------------------
//главная функция потока, выполняет передачу байтов из буфера в COM-порт
DWORD WINAPI WriteThread(LPVOID) {
	DWORD temp, signal;
	//создать событие
	ovlpwr.hEvent = CreateEvent(NULL, true, true, NULL);
	while (1) {
		//записать байты в порт (перекрываемая операция!)
		WriteFile(hCOMPort, bufwr, 1, &temp, &ovlpwr);
		//приостановить поток, пока не завершится перекрываемая операция WriteFile
		signal = WaitForSingleObject(ovlpwr.hEvent, INFINITE);
		//если операция завершилась успешно
		if ((signal == WAIT_OBJECT_0) && (GetOverlappedResult(hCOMPort, &ovlpwr, &temp, true))) {
		}
		else {
		}
		SuspendThread(hwriter);
	}
}
//---------------------------------------------------------------------------
//главная функция потока, реализует приём байтов из COM-порта
DWORD WINAPI ReadThread(LPVOID) {
	COMSTAT comstat;		//структура текущего состояния порта
	DWORD btr, temp, mask, signal;
	//создать сигнальный объект-событие для асинхронных операций
	ovlprd.hEvent = CreateEvent(NULL, true, true, NULL);
	//установить маску на срабатывание по событию приёма байта в порт
	SetCommMask(hCOMPort, EV_RXCHAR);
	//пока поток не будет прерван, выполняем цикл
	while (1) {
		//ожидать события приёма байта
		WaitCommEvent(hCOMPort, &mask, &ovlprd);
		//приостановить поток до прихода байта
		signal = WaitForSingleObject(ovlprd.hEvent, INFINITE);
		//если событие прихода байта произошло
		if (signal == WAIT_OBJECT_0) {
			//проверяем, успешно ли завершилась перекрываемая операция WaitCommEvent
			if (GetOverlappedResult(hCOMPort, &ovlprd, &temp, true)) {
				//если произошло именно событие прихода байта
				if ((mask & EV_RXCHAR) != 0) {
					//нужно заполнить структуру COMSTAT
					ClearCommError(hCOMPort, &temp, &comstat);
					//и получить из неё количество принятых байтов
					btr = comstat.cbInQue;
					//если действительно есть байты для чтения
					if (btr) {
						//прочитать байты из порта в буфер программы
						ReadFile(hCOMPort, bufrd, btr, &temp, &ovlprd);
						//обробка зчитаного байта
						ReadCOM(check);
						//очистить буфер (чтобы данные не накладывались друг на друга)
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
	DCB dcb;                //структура для общей инициализации порта DCB
	COMMTIMEOUTS timeouts;  //структура для установки таймаутов
	//открыть порт, для асинхронных операций указать флаг FILE_FLAG_OVERLAPPED
	hCOMPort = CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	//если ошибка открытия порта
	if (hCOMPort == INVALID_HANDLE_VALUE) {
		wcout << "error CreateFile(sPortName) " << sPortName;
		return false;
	}
	//инициализация порта
	dcb.DCBlength = sizeof(DCB);
	//считать структуру DCB из порта
	if (!GetCommState(hCOMPort, &dcb)) {
		CloseHandle(hCOMPort);                  //закрыть порт
		hCOMPort = INVALID_HANDLE_VALUE;
		wcout << "error GetCommState(hCOMPort) " << sPortName;
		return false;
	}
	//инициализация структуры DCB
	dcb.BaudRate = CBR_9600;				 //задаём скорость передачи 115200 бод
	dcb.fBinary = TRUE;                     //включаем двоичный режим обмена
	dcb.fOutxCtsFlow = FALSE;               //выключаем режим слежения за сигналом CTS
	dcb.fOutxDsrFlow = FALSE;               //выключаем режим слежения за сигналом DSR
	dcb.fDtrControl = DTR_CONTROL_DISABLE;  //отключаем использование линии DTR
	dcb.fDsrSensitivity = FALSE;            //отключаем восприимчивость драйвера к состоянию линии DSR
	dcb.fNull = FALSE;                      //разрешить приём нулевых байтов
	dcb.fRtsControl = RTS_CONTROL_DISABLE;  //отключаем использование линии RTS
	dcb.fAbortOnError = FALSE;              //отключаем остановку всех операций чтения/записи при ошибке
	dcb.ByteSize = 8;                       //задаём 8 бит в байте
	dcb.Parity = 0;                         //отключаем проверку чётности
	dcb.StopBits = 0;                       //задаём один стоп-бит
	//загрузить структуру DCB в порт
	if (!SetCommState(hCOMPort, &dcb)) {
		CloseHandle(hCOMPort);                  //закрыть порт
		hCOMPort = INVALID_HANDLE_VALUE;
		wcout << "error SetCommState(hCOMPort) " << sPortName;
		return false;
	}
	//установить таймауты
	timeouts.ReadIntervalTimeout = 10;	 	   //таймаут между двумя символами
	timeouts.ReadTotalTimeoutMultiplier = 0;  //общий таймаут операции чтения
	timeouts.ReadTotalTimeoutConstant = 0;    //константа для общего таймаута операции чтения
	timeouts.WriteTotalTimeoutMultiplier = 0; //общий таймаут операции записи
	timeouts.WriteTotalTimeoutConstant = 0;   //константа для общего таймаута операции записи
	//записать структуру таймаутов в порт
	if (!SetCommTimeouts(hCOMPort, &timeouts)) {
		CloseHandle(hCOMPort);                  //закрыть порт
		hCOMPort = INVALID_HANDLE_VALUE;
		wcout << "error SetCommTimeouts(hCOMPort) " << sPortName;
		return false;
	}
	//установить размеры очередей приёма и передачи
	SetupComm(hCOMPort, 16, 16);
	//очистить принимающий буфер порта
	PurgeComm(hCOMPort, PURGE_RXCLEAR);
	wcout << "connected COM port " << sPortName;
	return true;
}
//---------------------------------------------------------------------------
void sendStop(bool& i) {
	bufwr[0] = 0xCC;  // строка для передачи
	if (hwriter != INVALID_HANDLE_VALUE) {
		ResumeThread(hwriter);
	}
	i = false;
}
//---------------------------------------------------------------------------
void sendStart(bool& i) {
	bufwr[0] = 0xEE;  // строка для передачи
	if (hwriter != INVALID_HANDLE_VALUE) {
		ResumeThread(hwriter);
	}
	cout << "Started!" << endl;
	i = true;
}
//---------------------------------------------------------------------------
void sendT(int t) {
	bufwr[0] = (unsigned char)t;  // строка для передачи
	if (hwriter != INVALID_HANDLE_VALUE) {
		ResumeThread(hwriter);
	}
	cout << "Control T=" << std::to_string(t) << endl;
}
//---------------------------------------------------------------------------


//35    0x33 0x35 0x0A      0x23