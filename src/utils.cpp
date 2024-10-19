#include "utils.hpp"
#include "exports.hpp"

char** str_split(char* str, char delim, int* numSplits)
{
	char** ret;
	int retLen;
	char* c;

	if ((str == NULL) ||
		(delim == '\0'))
	{
		/* Either of those will cause problems */
		ret = NULL;
		retLen = -1;
	}
	else
	{
		retLen = 0;
		c = str;

		/* Pre-calculate number of elements */
		do
		{
			if (*c == delim)
			{
				retLen++;
			}

			c++;
		} while (*c != '\0');

		ret = (char **)malloc((retLen + 1) * sizeof(*ret));
		ret[retLen] = NULL;

		c = str;
		retLen = 1;
		ret[0] = str;

		do
		{
			if (*c == delim)
			{
				ret[retLen++] = &c[1];
				*c = '\0';
			}

			c++;
		} while (*c != '\0');
	}

	if (numSplits != NULL)
	{
		*numSplits = retLen;
	}

	return ret;
}

void RecursiveMkdir(const char *dir)
{
	char tmp[1024];
	char *p = NULL;
	uint32_t i;
	size_t len;

	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, sizeof(tmp), "%s", dir);

	i = strlen(tmp);
	while (tmp[i] != '/')
		i--;
	tmp[i] = 0;

	printf("Writing directory: %s...\n", tmp);
	len = strlen(tmp);
	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;

	for (p = tmp + 1; *p; p++)
	{
		if (*p == '/')
		{
			*p = 0;
			cellFsMkdir(tmp, CELL_FS_DEFAULT_CREATE_MODE_1);
			*p = '/';
		}
	}

	cellFsMkdir(tmp, CELL_FS_DEFAULT_CREATE_MODE_1);
}