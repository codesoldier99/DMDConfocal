
#pragma once

bool HLoadBmp( LPBYTE* pBuffer,int& IWidth, int& IHeight, int& wBitsPerPixel, LPCTSTR lpszFileName );
bool HSaveBmp( LPBYTE pBuffer,int IWidth, int IHeight, int wBitsPerPixel, LPCTSTR lpszFileName );
void ChangeFormate( unsigned char* scr,unsigned char* dest );
CString HGetTimeString();