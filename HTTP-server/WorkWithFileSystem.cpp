#include <stdlib.h>
#include "WorkWithFileSystem.h"
#include "ReciveHTTP.h"
#include <WinSock2.h>
#include <fileapi.h>
#include "HTTPResponse.h"
#include "WorkWithURL.h"

void CreateRoot()
{
	WIN32_FIND_DATAA Mock;
	HANDLE Res = FindFirstFileA(PATH_FOR_SERVER_ROOT, &Mock);
	if (Res == INVALID_HANDLE_VALUE)
	{
		CreateDirectoryA(PATH_FOR_SERVER_ROOT, NULL);
	}
}

void ConcatPath(char **Path, const char *NextName)
{
	int Len = strlen(*Path) + strlen(NextName) + strlen(PATH_DELIMITER);
	(*Path) = (char*)realloc((*Path), (Len + 1) * sizeof(char));
	strcat_s((*Path), Len + 1, PATH_DELIMITER);
	strcat_s((*Path), Len + 1, NextName);
}

BOOL ThisIsFile(char Name[])
{
	if ((strstr(Name, ".") != NULL) && (strstr(Name, "./") == NULL))
	{
		return TRUE;
	}
	return FALSE;
}

void CreateDir(char *PathArr[], int ArrLen, char **Path)
{
	HANDLE Res;
	WIN32_FIND_DATAA Mock;
	for (int i = 0; i < ArrLen - 1; i++)
	{
		ConcatPath(Path, PathArr[i]);
		Res = FindFirstFileA((*Path), &Mock);
		if (Res == INVALID_HANDLE_VALUE)
		{
			CreateDirectoryA((*Path), NULL);
		}
	}
}

void WriteInFile(FILE *Out, char *Buff, int Len)
{
	int Res;
	do
	{
		Res = fwrite(Buff, sizeof(char), Len, Out);
		Len -= Res;
		Buff += Res;
	} while (Len != 0);
}

void ReadDir(SOCKET Sock, char Path[])
{
	HANDLE search_file;
	WIN32_FIND_DATA Data;
	search_file = FindFirstFileA(Path, &Data);
	BOOL Empty = TRUE;
	while (FindNextFileA(search_file, &Data) != 0)
	{
		if (!strncmp(Data.cFileName, ".", 1) || !strncmp(Data.cFileName, "..", 2))
		{
			continue;
		}
		send(Sock, Data.cFileName, strlen(Data.cFileName), 0);
		send(Sock, HTTP_END_OF_LINE, strlen(HTTP_END_OF_LINE), 0);
		Empty = FALSE;
	}
	if (Empty)
	{
		send(Sock, MSG_DIR_IS_EMPTY, strlen(MSG_DIR_IS_EMPTY), 0);
		send(Sock, HTTP_END_OF_LINE, strlen(HTTP_END_OF_LINE), 0);
	}
	FindClose(search_file);
}

void FileRead(SOCKET Sock, char Path[], char *HTTPBuff)
{
	FILE *In;
	fopen_s(&In, Path, "rb");
	int ReadRes;
	do
	{
		ReadRes = fread_s(HTTPBuff, HTTP_BUFFER_SIZE + 1, sizeof(char), HTTP_BUFFER_SIZE, In);
		if (ReadRes != EOF)
		{
			int Tmp = ReadRes;
			char *TmpBuff = HTTPBuff;
			int SendRes;
			do
			{
				SendRes = send(Sock, TmpBuff, Tmp, 0);
				Tmp -= SendRes;
				TmpBuff += SendRes;
			} while (Tmp != 0);
		}
	} while (!feof(In));
	fclose(In);
}

void FileCopy(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff)
{
	char *Tmp = strstr(HTTPBuff, COPY_REQUEST);
	int DestLen;
	char **DestPathArr = ParseURL(Tmp, &DestLen);
	if (ThisIsFile(DestPathArr[DestLen - 1]) && ThisIsFile(PathArr[ArrLen - 1]))
	{
		int Len = strlen(PATH_FOR_SERVER_ROOT);
		char *Path = (char*)calloc(Len + 1, sizeof(char));
		memcpy_s(Path, Len + 1, PATH_FOR_SERVER_ROOT, Len);
		CreateDir(PathArr, ArrLen, &Path);
		ConcatPath(&Path, PathArr[ArrLen - 1]);

		int LenDest = strlen(PATH_FOR_SERVER_ROOT);
		char *PathDest = (char*)calloc(LenDest + 1, sizeof(char));
		memcpy_s(PathDest, Len + 1, PATH_FOR_SERVER_ROOT, Len);
		CreateDir(DestPathArr, DestLen, &PathDest);
		ConcatPath(&PathDest, DestPathArr[DestLen - 1]);

		if (strcmp(Path, PathDest) == 0)
		{
			send(Sock, BAD_RESPONSE, strlen(BAD_RESPONSE), 0);
			return;
		}

		HANDLE ResDest, Res;
		WIN32_FIND_DATAA DataDest, Data;

		 ResDest = FindFirstFileA(Path, &Data);
		 Res = FindFirstFileA(PathDest, &DataDest);

		if (Res == INVALID_HANDLE_VALUE)
		{
			send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			return;
		}
		else
		{
			if (CopyFileA(PathDest, Path, FALSE) != 0)
			{
				send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
				return;
			}
			else
			{
				send(Sock, INTERNAL_ERROR_RESPONSE, strlen(INTERNAL_ERROR_RESPONSE), 0);
				return;
			}
		}
	}
	else
	{
		send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
	}
}

void PutFile(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff, int ReciveRes)
{
	if (ArrLen == 0)
	{
		send(Sock, BAD_RESPONSE, strlen(BAD_RESPONSE), 0);
		return;
	}

	int Len = strlen(PATH_FOR_SERVER_ROOT);
	char *Path = (char*)calloc(Len + 1, sizeof(char));
	memcpy_s(Path, Len + 1, PATH_FOR_SERVER_ROOT, Len);

	CreateDir(PathArr, ArrLen, &Path);
	HANDLE Res;
	WIN32_FIND_DATAA Mock;
	ConcatPath(&Path, PathArr[ArrLen - 1]);
	Res = FindFirstFileA(Path, &Mock);
	if (ThisIsFile(PathArr[ArrLen - 1]))
	{
		if (strstr(HTTPBuff, COPY_REQUEST) != NULL) 
		{
			FileCopy(PathArr, ArrLen, Sock, HTTPBuff);

		}
		else
		{
			ReciveFile(Path, Sock, HTTPBuff, ReciveRes);
			if (Res == INVALID_HANDLE_VALUE)
			{
				/*response code will be 201 created*/
				send(Sock, CREATED_RESPONSE, strlen(CREATED_RESPONSE), 0);
			}
			else
			{
				/*response code will be 200 ok*/
				send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			}
		}
	}
	else
	{
		timeval Delay = { 1, 0 };
		fd_set ForRead = { 1, Sock };
		int RecvRes = 0;
		int Result;
		do
		{
			Result = select(0, &ForRead, NULL, NULL, &Delay);
			if (Result == SOCKET_ERROR)
			{
				printf("select function failed with error = %d\n", WSAGetLastError());
				return;
			}

			if (Result > 0)
			{
				RecvRes = recv(Sock, HTTPBuff, HTTP_BUFFER_SIZE, 0);
				if (RecvRes == SOCKET_ERROR)
				{
					printf("recv function failed with error = %d\n", WSAGetLastError());
					return;
				}
				ForRead.fd_count = 1;
				ForRead.fd_array[0] = Sock;
			}
		} while (RecvRes != 0 && Result != 0);

		if (Res == INVALID_HANDLE_VALUE)
		{
			CreateDirectoryA(Path, NULL);
			/*response code will be 201 created*/
			send(Sock, CREATED_RESPONSE, strlen(CREATED_RESPONSE), 0);
		}
		else
		{
			/*response code will be 200 ok*/
			send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
		}
	}
}

void CreateFullPath(char *PathArr[], int ArrLen, char **Path)
{
	for (int i = 0; i < ArrLen; i++)
	{
		ConcatPath(Path, PathArr[i]);
	}
}

void GetFile(char *PathArr[], int ArrLen, SOCKET Sock, char *HTTPBuff)
{
	if (ArrLen == 0)
	{
		send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
		int Len = strlen(PATH_FOR_SERVER_ROOT) + strlen(PATH_DELIMITER) + strlen("*");
		char *Tmp = (char*)calloc(Len + 1, sizeof(char));
		strcat_s(Tmp, Len + 1, PATH_FOR_SERVER_ROOT);
		strcat_s(Tmp, Len + 1, PATH_DELIMITER);
		strcat_s(Tmp, Len + 1, "*");
		ReadDir(Sock, Tmp);
		return;
	}

	int Len = strlen(PATH_FOR_SERVER_ROOT);
	char *Path = (char*)calloc(Len + 1, sizeof(char));
	memcpy_s(Path, Len + 1, PATH_FOR_SERVER_ROOT, Len);
	CreateFullPath(PathArr, ArrLen, &Path);
	
	HANDLE Res;
	WIN32_FIND_DATAA Mock;
	Res = FindFirstFileA(Path, &Mock);

	if (ThisIsFile(PathArr[ArrLen - 1]))
	{
		if (Res == INVALID_HANDLE_VALUE)
		{
			send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
		}
		else
		{
			send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			FileRead(Sock, Path, HTTPBuff);
		}
			
	}
	else
	{
		if (Res == INVALID_HANDLE_VALUE)
		{
			send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
		}
		else
		{
			send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			ConcatPath(&Path, "*");
			ReadDir(Sock, Path);
		}
			
	}
}

int DelDir(char *Path)
{
	HANDLE search_file;
	WIN32_FIND_DATA Data;
	search_file = FindFirstFileA(Path, &Data);
	int Result = 1;
	while (FindNextFileA(search_file, &Data) != 0)
	{
		if (!strncmp(Data.cFileName, ".", 1) || !strncmp(Data.cFileName, "..", 2))
		{
			continue;
		}
		
		if (Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			int Len = strlen(Path);
			char *TmpPath = (char*)calloc(Len + 1, sizeof(char));
			memcpy_s(TmpPath, Len + 1, Path, Len - 2);
			ConcatPath(&TmpPath, Data.cFileName);
			ConcatPath(&TmpPath, "*");
			Result = DelDir(TmpPath);
			if (Result == 0)
			{
				return 0;
			}
			Len = strlen(TmpPath) - 1;
			TmpPath = (char*)realloc(TmpPath, Len); /*to delete '/*' symbols*/
			TmpPath[Len - 1] = '\0';
			if (RemoveDirectoryA(TmpPath) == 0)
			{
				return 0;
			}
		}
		else
		{
			int Len = strlen(Path);
			char *TmpPath = (char*)calloc(Len + 1, sizeof(char));
			memcpy_s(TmpPath, Len + 1, Path, Len - 2);/*to delete '/*' symbols*/
			ConcatPath(&TmpPath, Data.cFileName);
			if (DeleteFileA(TmpPath) == 0)
			{
				return 0;
			}
		}
	}
	FindClose(search_file);
	return Result;
}

void DelFile(char *PathArr[], int ArrLen, SOCKET Sock)
{
	if (ArrLen == 0)
	{
		send(Sock, FORBIDDEN_RESPONSE, strlen(FORBIDDEN_RESPONSE), 0);
		return;
	}

	int Len = strlen(PATH_FOR_SERVER_ROOT);
	char *Path = (char*)calloc(Len + 1, sizeof(char));
	memcpy_s(Path, Len + 1, PATH_FOR_SERVER_ROOT, Len);

	CreateFullPath(PathArr, ArrLen, &Path);

	HANDLE search_file;
	WIN32_FIND_DATA Data;
	search_file = FindFirstFileA(Path, &Data);
	if (ThisIsFile(PathArr[ArrLen - 1]))
	{
		if (search_file == INVALID_HANDLE_VALUE)
		{
			send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			return;
		}
		else
		{
			if (DeleteFileA(Path) != 0)
			{
				send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			}
			else
			{
				send(Sock, INTERNAL_ERROR_RESPONSE, strlen(INTERNAL_ERROR_RESPONSE), 0);
			}
		}
	}
	else
	{
		if (search_file == INVALID_HANDLE_VALUE)
		{
			send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
			return;
		}
		else
		{
			ConcatPath(&Path, "*");
			if (DelDir(Path) != 0)
			{
				int Len = strlen(Path) - 1;
				Path = (char*)realloc(Path, Len); /*to delete '/*' symbols*/
				Path[Len - 1] = '\0';
				RemoveDirectoryA(Path);
			
				send(Sock, OK_RESPONSE, strlen(OK_RESPONSE), 0);
			}
			else
			{
				send(Sock, INTERNAL_ERROR_RESPONSE, strlen(INTERNAL_ERROR_RESPONSE), 0);
			}
		}
	}
}

void HeadFile(char *PathArr[], int ArrLen, SOCKET Sock)
{
	int Len = strlen(PATH_FOR_SERVER_ROOT);
	char *Path = (char*)calloc(Len + 1, sizeof(char));
	memcpy_s(Path, Len + 1, PATH_FOR_SERVER_ROOT, Len);
	CreateFullPath(PathArr, ArrLen, &Path);

	HANDLE search_file;
	WIN32_FIND_DATA Data;
	search_file = FindFirstFileA(Path, &Data);
	
	if (search_file == INVALID_HANDLE_VALUE)
	{
		send(Sock, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
	}
	else
	{
		if (!(Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			Len = HTTP_BUFFER_SIZE;
			char *Buff = (char*)calloc(Len + 1, sizeof(char));

			long long Size = (Data.nFileSizeHigh * (MAXDWORD + 1)) + Data.nFileSizeLow;

			sprintf_s(Buff, Len + 1, "HTTP/1.1 200 OK\r\nFile-Size: %lld\r\nType: File\r\nName: %s\r\n\r\n\0", Size, Data.cFileName);
			send(Sock, Buff, strlen(Buff), 0);
			free(Buff);
		}
		else
		{
			send(Sock, NOT_ALLOWED_RESPONSE, strlen(NOT_ALLOWED_RESPONSE), 0);
		}
	}
	
}
