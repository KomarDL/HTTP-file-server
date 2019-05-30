#pragma once

#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>

#define PATH_FOR_SERVER_ROOT "./Root"
#define PATH_DELIMITER "/"
#define MSG_DIR_IS_EMPTY "Empty folder"

void CreateRoot();
void PutFile(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff, int ReciveRes);
void GetFile(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff);
void WriteInFile(FILE *Out, char *Buff, int Len);
void DelFile(char *PathArr[], int ArrLen, SOCKET Sock);
void HeadFile(char *PathArr[], int ArrLen, SOCKET Sock);