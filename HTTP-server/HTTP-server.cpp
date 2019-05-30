#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <string>
#include <vector>
#include <thread>
#include "ReciveHTTP.h"
#include "WorkWithFileSystem.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment (lib, "iphlpapi.lib")

#define WORKING_BUFFER_SIZE 15000
#define DEFAULT_PORT 8080

ULONG GetServerIP()
{
	ULONG BroadcastAddr = 0;
	ULONG Flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_INCLUDE_GATEWAYS;
	ULONG BuffSize = WORKING_BUFFER_SIZE;
	PIP_ADAPTER_ADDRESSES Adr = (PIP_ADAPTER_ADDRESSES)malloc(BuffSize), CurrAdr;

	ULONG RetVal = GetAdaptersAddresses(AF_INET, Flags, NULL, Adr, &BuffSize);
	CurrAdr = Adr;
	ULONG Result = 0;
	if (RetVal == NO_ERROR)
	{
		BOOL GatewayIPFounded = FALSE;
		while ((CurrAdr != NULL) || (GatewayIPFounded))
		{
			GatewayIPFounded = (CurrAdr->FirstGatewayAddress != NULL);
			if (GatewayIPFounded)
			{
				sockaddr_in *IPInAddr = (sockaddr_in*)CurrAdr->FirstUnicastAddress->Address.lpSockaddr;
				Result = IPInAddr->sin_addr.s_addr;
			}
			CurrAdr = CurrAdr->Next;
		}
	}
	else
	{
		LPVOID lpMsgBuf = NULL;
		printf("Call to GetAdaptersAddresses failed with error: %d\n", RetVal);
		if (RetVal == ERROR_NO_DATA)
			printf("\tNo addresses were found for the requested parameters\n");
		else
		{

			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, RetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),/* Default language*/(LPTSTR)&lpMsgBuf, 0, NULL))
			{
				printf("\tError: %s", (char*)lpMsgBuf);
			}
		}
	}

	free(Adr);
	return Result;
}

int main()
{
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	CreateRoot();

	int Res = 0;
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf_s("Error");
		exit(1);
	}

	ULONG ServerIP = GetServerIP();
	std::string IPStr;
	if (inet_ntop(AF_INET, &ServerIP, (PSTR)(IPStr.c_str()), 16) == NULL)
	{
		printf_s("inet_ntop failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		getchar();
		return 1;
	}
	else
	{
		printf_s("Server's IP = %s\nServer's port = %d\n\n\n", IPStr.c_str(), DEFAULT_PORT);
		IPStr.~basic_string();
	}

	SOCKET ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSock == INVALID_SOCKET)
	{
		printf("socket function failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		getchar();
		return 1;
	}

	sockaddr_in Adr;
	const int AdrLen = sizeof(Adr);
	Adr.sin_family = AF_INET;
	Adr.sin_port = htons((USHORT)DEFAULT_PORT);
	Adr.sin_addr.s_addr = ServerIP;

	Res = bind(ListenSock, (SOCKADDR*)&Adr, AdrLen);

	Res = listen(ListenSock, SOMAXCONN);
	if (Res == INVALID_SOCKET)
	{
		printf("listen function failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		getchar();
		return 1;
	}

	SOCKET Tmp;
	std::vector<SOCKET> SockVector;
	std::vector<std::thread> ThreadPool;
	size_t i = 0;
	while (true)
	{
		Tmp = accept(ListenSock, NULL, NULL);
		if (Tmp == INVALID_SOCKET)
		{
			printf("accept function failed with error = %d\n", WSAGetLastError());
			continue;
		}
		SockVector.push_back(Tmp);
		ThreadPool.push_back(std::thread(ReciveRequestAndSendResponse, SockVector[i]));
		ThreadPool[i++].detach();
	}

	WSACleanup();
	getchar();
	return 0;
	return 0;
}