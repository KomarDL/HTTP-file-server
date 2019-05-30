#pragma once

#include <WinSock2.h>
#define HTTP_BUFFER_SIZE 65536 

#define HTTP_END_OF_LINE "\r\n"
#define HTTP_DELIMITER " "

void ReciveRequestAndSendResponse(SOCKET ClientSock);
void ReciveFile(char Path[], SOCKET Sock, char HTTPBuff[], int ReciveRes);