#pragma once

#include <winsock2.h>
#include "HTTPRequest.h"

#define OK_RESPONSE "HTTP/1.1 200 OK\r\n\r\n"
#define NOT_IMPLEMENTED_RESPONSE "HTTP/1.1 418 I'm a teapot\r\n\r\n" /*"HTTP/1.1 501 Not Implemented\r\n\r\n"*/
#define CREATED_RESPONSE "HTTP/1.1 201 Created\r\n\r\n"
#define BAD_RESPONSE "HTTP/1.1 400 Bad Request\r\n\r\n"
#define NOT_FOUND_RESPONSE "HTTP/1.1 404 Not Found\r\n\r\n"
#define FORBIDDEN_RESPONSE "HTTP/1.1 403 Forbidden\r\n\r\n"
#define INTERNAL_ERROR_RESPONSE "HTTP/1.1 500 Internal Server Error\r\n\r\n"
#define NOT_ALLOWED_RESPONSE "HTTP/1.1 405 Method Not Allowed\r\n\r\n"

void ProcessRequest(char *PathArr[], int ArrLen, TRequestType ReqType, SOCKET Sock, char *HTTPBuff, int ReciveRes);