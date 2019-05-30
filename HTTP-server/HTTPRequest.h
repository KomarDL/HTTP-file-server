#pragma once

typedef enum _RequestType
{
	NotImplemented,
	Put,
	Get,
	Delete,
	Head
} TRequestType, *PRequestType;

#define PUT_REQUEST "PUT"
#define GET_REQUEST "GET"
#define DELETE_REQUEST "DELETE"
#define HEAD_REQUEST "HEAD"
#define COPY_REQUEST "Copy-file-from"

TRequestType GetRequestType(char *HTTPBuff);


