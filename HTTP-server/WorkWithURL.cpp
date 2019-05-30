#include <stdlib.h>
#include "WorkWithURL.h"
#include "ReciveHTTP.h"

char *GetURL(char *BuffWithURl)
{
	char *StartOfURL = strstr(BuffWithURl, HTTP_DELIMITER) + 2/*need for truncate first '/' symbol*/;
	char *EndOfURL = strstr(StartOfURL, HTTP_DELIMITER);
	int URLLen = abs(EndOfURL - StartOfURL);
	char *Result = (char*)calloc(URLLen + 1, sizeof(char));
	memcpy_s(Result, URLLen, StartOfURL, URLLen);
	return Result;
}

char **ParseURL(char *BuffWithURl, int *Len)
{
	char *URL = GetURL(BuffWithURl);
	if (strlen(URL) == 0)
	{
		/*if need to show server root directory*/
	}

	(*Len) = URL_DEFAULT_ARR_LEN;
	char **Result = (char**)malloc((*Len) * sizeof(char*));
	int i = 0, TokenLen;
	char *NextToken = NULL;
	char *Token = strtok_s(URL, URL_NAME_DELIMITER, &NextToken);
	while (Token != NULL)
	{
		if (i == (*Len))
		{
			(*Len) = (*Len) * 2 + 1;
			Result = (char**)realloc(Result, (*Len) * sizeof(char*));
		}
		TokenLen = strlen(Token);
		Result[i] = (char*)malloc((TokenLen + 1) * sizeof(char));
		strcpy_s(Result[i++], TokenLen + 1, Token);

		Token = strtok_s(NULL, URL_NAME_DELIMITER, &NextToken);
	}

	(*Len) = i;
	Result = (char**)realloc(Result, (*Len) * sizeof(char*));

	free(URL);
	return Result;
}