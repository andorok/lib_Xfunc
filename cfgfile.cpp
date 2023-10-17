
#include "cfgfile.h"

//char* CCfgFile::strlwr(char* str)
//{
//	unsigned char* p = (unsigned char*)str;
//
//	while (*p) {
//		*p = tolower((unsigned char)*p);
//		p++;
//	}
//
//	return str;
//}

int CCfgFile::strlwr(char* str)
{
	size_t i;
	size_t nSize;

	if (str == 0)
		return 0;

	nSize = strlen(str);

	for (i = 0; i < nSize; i++)
		str[i] = (char)tolower((int)str[i]);

	return 0;
}

int CCfgFile::FindSection(const char* src, const char* section)
{
	//int key_size = strlen(section);
	int size = (int)(strlen(src) - strlen(section));
	int tmp_len;

	//for (int i = 0; i < (strlen(src) - key_size); i++)
	for (int i = 0; i < size; i++)
	{
		const char *psubstr = &src[i];

		if (psubstr[i] == ';')
			return -1;
		if (psubstr[i] == '\0')
			return -2;

		if (psubstr[i] == '[') //begin section
		{
			char name[MAX_PATH] = { 0 };
			strcpy(name, &psubstr[i + 1]);
			strlwr(name);

			char section_tmp[1024];
			strcpy(section_tmp, section);
			strlwr(section_tmp);
			tmp_len = (int)strlen(section_tmp);

			if (strstr(name, section_tmp) && (name[tmp_len] == ']'))
			{
				//DEBUG_PRINT("Section: < %s > was found in the string < %s >\n", section, src);
				return 0;
			}
			else
			{
				//DEBUG_PRINT("Section: < %s > was not found\n", section);
				return -3;
			}
		}
	}
	return -4;
}

//-----------------------------------------------------------------------------

int CCfgFile::IsSection(const char* src)
{
	//int nSize;
	size_t nSize;

	if ((src == 0) || (src[0] != '['))
		return 0;

	nSize = strlen(src);

	if (src[nSize - 1] != ']')
		return 0;

	return 1;
}

//-----------------------------------------------------------------------------

int CCfgFile::IsOptionName(const char* src)
{
	char *pStr;

	pStr = strstr((char *)src, "=");

	if (pStr == 0)
		return 0;

	*pStr = '\0';

	return 1;
}

//-----------------------------------------------------------------------------

int CCfgFile::FindOption(const char* src, const char* option, char *Buffer, int BufferSize, int *set_default)
{
	//int key_size = strlen(option);
	int size = (int)(strlen(src) - strlen(option));
	int option_len;

	if (!set_default)
		return -1;

	*set_default = 1;

	//for (int i = 0; i < (strlen(src) - key_size); i++)
	for (int i = 0; i < size; i++)
	{
		const char *psubstr = &src[i];

		if (psubstr[i] == ';')
			return -1;
		if (psubstr[i] == '\0')
			return -2;

		if (psubstr[i] == '[')
			return -3;

		char option_tmp[1024];
		strcpy(option_tmp, option);
		strlwr(option_tmp);
		option_len = (int)strlen(option_tmp);

		char name[1024] = { 0 };
		strcpy(name, &psubstr[i]);
		strlwr(name);

		if (strstr(name, option_tmp) && ((name[option_len] == ' ') || (name[option_len] == '=') || (name[option_len] == '\t')))
		{
			//DEBUG_PRINT("Option: < %s > was found in the string < %s >\n", option, src);

			char *val = (char*)strstr(src, "=");

			val++;

			while ((val[0] == ' ') || (val[0] == '\t'))
				val++;
			/*
						if( strstr(val, ".") ) {
							DEBUG_PRINT(stderr, "Val = %f\n", atof(val));
						} else {
							DEBUG_PRINT(stderr, "Val = %d\n", atoi(val));
						}
			*/

			char aSymb[6][3] = { "\n", "\r", "\t", ";", "#", "//" };

			for (int i = 0; i < 6; i++)
			{
				char *tmp;

				tmp = strstr(val, aSymb[i]);

				if (tmp)
					tmp[0] = '\0';
			}

			if (BufferSize >= (int)(strlen(val) + 1)) {
				strcpy(Buffer, val);
			}
			else {
				//DEBUG_PRINT("Option: < %s > was found in the string < %s >. But buffer to small\n", option, src);
				return -3;
			}

			while ((Buffer[strlen(Buffer) - 1] == '\r') ||
				(Buffer[strlen(Buffer) - 1] == '\n'))
				Buffer[strlen(Buffer) - 1] = '\0';

			*set_default = 0;

			return 0;
		}
		else
		{
			//DEBUG_PRINT("Option: < %s > was not found\n", option);
			return -4;
		}
	}
	return -5;
}

//-----------------------------------------------------------------------------
#include <fstream>
using namespace std;
//-----------------------------------------------------------------------------

int CCfgFile::getString(const char *section, const char *name, const char *defString,
					char *outString, int nSize)
{
	char str[MAX_PATH];
	ifstream ifs;
	int set_default = 1;
	int size;
	int sectionsSize = 0;
	int found = 0;

	ifs.open(m_filename.c_str(), ios::in);
	if (!ifs.is_open()) {
		//DEBUG_PRINT("Can't open file: %s. %s\n", lpFileName, strerror(errno));
		return -1;
	}

	while (!ifs.eof() && (!found)) {
		ifs.getline(str, sizeof(str), '\n');

		if (section == 0) {
			size = (int)strlen(str);
			str[size] = '\0';

			if (IsSection(str)) {
				size--;
				str[size] = '\0';
				sectionsSize += size;

				if (sectionsSize > (nSize - 1))
					break;

				memcpy(outString, str + 1, size);
				outString += size;
			}
		}
		else if (FindSection(str, section) == 0) {

			//DEBUG_PRINT("inside cycle\n");

			while (!ifs.eof()) {

				ifs.getline(str, sizeof(str), '\n');

				if (name == 0) {
					size = (int)strlen(str);
					str[size] = '\0';

					if (IsSection(str))
						break;

					if (IsOptionName(str)) {
						size = (int)strlen(str) + 1;
						sectionsSize += size;

						if (sectionsSize > (nSize - 1))
							break;

						memcpy(outString, str, size);
						outString += size;
					}
				}
				else if (FindOption(str, name, outString, nSize, &set_default) == 0) {
					found = 1;
					break;
				}
				else {
					found = 0;
				}
			}

		}
	}

	if (set_default) {
		if (defString) {
			memcpy(outString, defString, strlen(defString) + 1);
		}
	}

	if ((section == 0) || (name == 0))
		*outString = '\0';

	ifs.close();

	return found;
}

//-----------------------------------------------------------------------------
int CCfgFile::writeString(const char *section, const char *name, const char *inString)
{
	long nStart, nEnd, nSize;
	int isFindSection = 0;
	//int isFindParam = 0;
	char *pBuf, *pCur;
	char sLine[1024];
	char sParamName[1024];
	char sNextSection[1024];
	//FILE *pFile;

	strcpy(sNextSection, "");

	//pFile = fopen(lpFileName, "r");
	m_hfile = fopen(m_filename.c_str(), "r");

	//if (pFile == 0)
	//	return -1;

	// Вычисление размера буфера
	fseek(m_hfile, 0, SEEK_SET);
	nStart = ftell(m_hfile);
	fseek(m_hfile, 0, SEEK_END);
	nEnd = ftell(m_hfile);
	fseek(m_hfile, 0, SEEK_SET);
	nSize = nEnd - nStart + 1024;

	pBuf = new char[nSize];
	pCur = pBuf;

	nSize = 0;

	// Поиск секции
	while (!feof(m_hfile))
	{
		strcpy(sLine, "");
		fgets(sLine, 1024, m_hfile);

		if (strlen(sLine) == 0)
			continue;

		strcpy(pCur, sLine);

		pCur += strlen(sLine);

		nSize += (int)strlen(sLine);

		sLine[strlen(sLine) - 1] = '\0';

		if (!IsSection(sLine))
			continue;

		sLine[strlen(sLine) - 1] = '\0';
		strcpy(sLine, sLine + 1);

		if (strcmp(sLine, section) == 0)
		{
			isFindSection = 1;
			break;
		}
	}

	if (isFindSection)
	{   // Секция найдена, поиск параметра
		while (!feof(m_hfile))
		{
			strcpy(sLine, "");
			fgets(sLine, 1024, m_hfile);

			strcpy(sParamName, sLine);

			if (IsOptionName(sParamName))
			{   // Параметр
				if (strcmp(sParamName, name) == 0)
				{   // Найден нужный параметр
					//isFindParam = 1;
					break;
				}
			}

			strcpy(sNextSection, sLine);
			sNextSection[strlen(sLine) - 1] = '\0';

			if (IsSection(sNextSection))
			{   // Следующая секция
				strcpy(sNextSection, sLine);
				break;
			}
			else
				strcpy(sNextSection, "");

			strcpy(pCur, sLine);

			pCur += strlen(sLine);

			nSize += (int)strlen(sLine);
		}
	}

	if (!isFindSection)
	{   // Секция не найдена
		strcpy(sLine, "");
		strcpy(sLine, "[");
		strcat(sLine, name);
		strcat(sLine, "]\n");
		strcat(pCur, sLine);

		pCur += strlen(sLine);
		nSize += (int)strlen(sLine);
	}

	// Добавление параметра
	strcpy(sLine, "");
	strcpy(sLine, name);
	strcat(sLine, "=");
	strcat(sLine, inString);
	strcat(sLine, "\n");
	strcat(sLine, sNextSection);
	strcat(pCur, sLine);

	pCur += strlen(sLine);
	nSize += (int)strlen(sLine);

	// Чтение оставшихся строк
	while (!feof(m_hfile))
	{
		strcpy(sLine, "");
		fgets(sLine, 1024, m_hfile);

		strcpy(pCur, sLine);

		pCur += strlen(sLine);

		nSize += (int)strlen(sLine);
	}

	fclose(m_hfile);

	// Запись в файл
	//m_hfile = fopen(lpFileName, "w");
	m_hfile = fopen(m_filename.c_str(), "w");

	//if (pFile == 0)
	//	return -1;

	fwrite(pBuf, nSize, 1, m_hfile);
	fclose(m_hfile);

	// Удаление буфера
	delete[] pBuf;

	return 0;
}

CCfgFile::CCfgFile(const string& filename)
{
	m_hfile = fopen(filename.c_str(), "r+");
	if (!m_hfile)
		m_error = -1;
	else
	{
		m_filename = filename;
		//m_error = ini_parse_file();
		m_error = 0;
		fclose(m_hfile);
	}
}

int CCfgFile::GetError() const
{
	return m_error;
}

