#pragma once

#ifndef Unit2H
#define Unit2H

void ReadCOM(bool& i);
void sendStop(bool& i);
void sendT(int t);
//void conComST(int t);
//int Check;
bool conCom();
void sendStart(bool& i);

void COMPortStartThreads(void);
void COMTerminate(void);

#endif