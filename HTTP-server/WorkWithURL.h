#pragma once

#define URL_DEFAULT_ARR_LEN 10
#define URL_NAME_DELIMITER "/"

char **ParseURL(char *BuffWithURl, int *Len);