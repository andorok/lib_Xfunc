
#ifndef _CMDLINE_ARG_H_
#define _CMDLINE_ARG_H_

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// argument types
enum
{
	ARG_TYPE_NOT_NUM = 0,		// �������� ������������ ��� �����
	ARG_TYPE_CHR_NUM = 1,		// �������� - ����� � ������
	ARG_TYPE_STR_NUM = 2		// �������� - ������ � ������
};

int get_cmdl_arg(int &ii, char *argv[], const char *name, int type, int def_val)
{
	int val = def_val;
	char	*pLin, *endptr;

	switch (type)
	{
	case 0:
	{	// ��� �����
		if (!strcmp(argv[ii], name))
		{
			val = 1;
			printf("Command line: %s\n", name);
		}
		break;
	}
	case 1:
	{	// ����� � ������
		if (tolower(argv[ii][1]) == name[1])
		{
			pLin = &argv[ii][2];
			if (argv[ii][2] == '\0')
			{ // ����� ������ ����� ������
				ii++;
				pLin = argv[ii];
			}
			val = strtoul(pLin, &endptr, 0);
			printf("Command line: %s%d\n", name, val);
		}
		break;
	}
	case 2:
		if (strstr(argv[ii], name))
		{
			int len = (int)strlen(name);
			pLin = &argv[ii][len];
			if (argv[ii][len] == '\0')
			{
				ii++;
				pLin = argv[ii];
			}
			val = strtoul(pLin, &endptr, 0);
			printf("Command line: %s%d\n", name, val);
		}
		break;
	}

	return val;
}

double get_cmdl_arg(int &ii, char *argv[], const char *name, int type, double def_val)
{
	double val = def_val;
	char	*pLin, *endptr;

	switch (type)
	{
	case 0:
	{	// ��� �����
		if (!strcmp(argv[ii], name))
		{
			val = 1;
			printf("Command line: %s\n", name);
		}
		break;
	}
	case 1:
	{	// ����� � ������
		if (tolower(argv[ii][1]) == name[1])
		{
			pLin = &argv[ii][2];
			if (argv[ii][2] == '\0')
			{ // ����� ������ ����� ������
				ii++;
				pLin = argv[ii];
			}
			val = strtod(pLin, &endptr);
			printf("Command line: %s%.2f\n", name, val);
		}
		break;
	}
	case 2:
		if (strstr(argv[ii], name))
		{
			int len = (int)strlen(name);
			pLin = &argv[ii][len];
			if (argv[ii][len] == '\0')
			{
				ii++;
				pLin = argv[ii];
			}
			val = strtod(pLin, &endptr);
			printf("Command line: %s%.2f\n", name, val);
		}
		break;
	}

	return val;
}

#endif	// _CMDLINE_ARG_H_
