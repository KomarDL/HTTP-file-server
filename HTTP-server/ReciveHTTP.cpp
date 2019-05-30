#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include "ReciveHTTP.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "WorkWithURL.h"
#include "WorkWithFileSystem.h"

void ReciveRequestAndSendResponse(SOCKET ClientSock)
{
	int Res, RecvRes;
	bool RequestFound = false, EndOfData = false;
	timeval Delay = { 1, 0 };
	fd_set ForRead = { 1, ClientSock };
	char *HTTPBuff = (char *)calloc(HTTP_BUFFER_SIZE + 1, sizeof(char));

	Res = select(0, &ForRead, NULL, NULL, &Delay);
	if (Res == SOCKET_ERROR)
	{
		printf("select function failed with error = %d\n", WSAGetLastError());
		getchar();
		return;
	}

	RecvRes = recv(ClientSock, HTTPBuff, HTTP_BUFFER_SIZE, 0);
	printf_s("%s\n", HTTPBuff);

/*define request type*/
	TRequestType ReqType = GetRequestType(HTTPBuff);
	char *ResponseBuff = (char*)calloc(1, sizeof(char));
	int ResponseBuffLen = 0;
	if (ReqType == NotImplemented)
	{
	/*send response with "method not implemented" code*/
		send(ClientSock, NOT_IMPLEMENTED_RESPONSE, strlen(NOT_IMPLEMENTED_RESPONSE), 0);
	}
	else
	{
	/*parse url*/
		int ArrLen;
		char **PathArr = ParseURL(HTTPBuff, &ArrLen);
	/*do anything depending on the type of request*/
		ProcessRequest(PathArr, ArrLen, ReqType, ClientSock, HTTPBuff, RecvRes);
	}
			
	shutdown(ClientSock, SD_BOTH);
	closesocket(ClientSock);
	return;
}

void ReciveFile(char Path[], SOCKET Sock, char HTTPBuff[], int ReciveRes)
{
	int Res;
	
	int Len = strlen(HTTP_END_OF_LINE) * 2;
	char *EndOfHeader = (char*)calloc(Len + 1, sizeof(char));
	strcat_s(EndOfHeader, Len + 1, HTTP_END_OF_LINE);
	strcat_s(EndOfHeader, Len + 1, HTTP_END_OF_LINE);

	char *Tmp = strstr(HTTPBuff, EndOfHeader);
	Tmp += Len;
	Len = abs(Tmp - HTTPBuff);
	ReciveRes -= Len;
	FILE *Out;
	fopen_s(&Out, Path, "wb");

	WriteInFile(Out, Tmp, ReciveRes);

	timeval Delay = { 5, 0 };
	fd_set ForRead = { 1, Sock };
	int RecvRes = 0;

	do
	{
		Res = select(0, &ForRead, NULL, NULL, &Delay);
		if (Res == SOCKET_ERROR)
		{
			printf("select function failed with error = %d\n", WSAGetLastError());
			fclose(Out);
			return;
		}

		if (Res > 0)
		{
			RecvRes = recv(Sock, HTTPBuff, HTTP_BUFFER_SIZE, 0);
			if (RecvRes == SOCKET_ERROR)
			{
				printf("recv function failed with error = %d\n", WSAGetLastError());
				fclose(Out);
				return;
			}

			if (RecvRes > 0)
			{
				WriteInFile(Out, HTTPBuff, RecvRes);
			}
			ForRead.fd_count = 1;
			ForRead.fd_array[0] = Sock;
		}
	} while (RecvRes != 0 && Res != 0);

	fclose(Out);
}