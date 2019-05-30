#include <stdlib.h>
#include <string.h>
#include "HTTPRequest.h"
#include "ReciveHTTP.h"

void DefineRequestType(char Req[], PRequestType ReqType)
{
	int Res;
	Res = strcmp(Req, PUT_REQUEST);
	if (Res == 0)
	{
		(*ReqType) = Put;
		return;
	}

	Res = strcmp(Req, GET_REQUEST);
	if (Res == 0)
	{
		(*ReqType) = Get;
		return;
	}

	Res = strcmp(Req, DELETE_REQUEST);
	if (Res == 0)
	{
		(*ReqType) = Delete;
		return;
	}

	Res = strcmp(Req, HEAD_REQUEST);
	if (Res == 0)
	{
		(*ReqType) = Head;
		return;
	}
}

TRequestType GetRequestType(char *HTTPBuff)
{
	TRequestType Result = NotImplemented;
	char *Tmp, *Req;
	Tmp = strstr(HTTPBuff, HTTP_DELIMITER);
	if (Tmp != NULL)
	{
		int Length = abs(Tmp - HTTPBuff);
		Req = (char*)calloc(Length + 1, sizeof(char));
		strncpy_s(Req, Length + 1, HTTPBuff, Length);
		DefineRequestType(Req, &Result);
	}
	return Result;
}