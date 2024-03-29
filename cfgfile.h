#pragma once

#include <stdio.h>
#include <map>
#include <string.h>
#include <ctype.h>
#include <string>

//#include "windows.h"
#define MAX_PATH          260

//#define MAX_INI_STR 280
//#define MAX_SECTION 64
//#define MAX_NAME 64
//
///* Chars that begin a start-of-line comment. Per Python configparser, allow
//   both ; and # comments at the start of a line by default. */
//#ifndef INI_START_COMMENT_PREFIXES
//#define INI_START_COMMENT_PREFIXES ";#"
//#endif
//
///* Nonzero to allow inline comments (with valid inline comment characters
//	specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
//	Python 3.2+ configparser behaviour. */
//#ifndef INI_ALLOW_INLINE_COMMENTS
//#define INI_ALLOW_INLINE_COMMENTS 1
//#endif
//#ifndef INI_INLINE_COMMENT_PREFIXES
//#define INI_INLINE_COMMENT_PREFIXES ";"
//#endif
//
class CCfgFile
{
public:

	CCfgFile(const std::string& filename);
	//~CIniFile();
	int GetError() const;

	int getString(const char *section, const char *name, const char *defString,
		char *outString, int nSize);

	int writeString(const char *section, const char *name, const char *inString);

	// Get an integer (long) value from INI file, returning default_value if
	// not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
	//long GetInt(const std::string& section, const std::string& name, long default_value) const;

	// Get a real (floating point double) value from INI file, returning
	// default_value if not found or not a valid floating point value
	// according to strtod().
	//double GetDouble(const std::string& section, const std::string& name, double default_value) const;

	// Get a boolean value from INI file, returning default_value if not found or if
	// not a valid true/false value. Valid true values are "true", "yes", "on", "1",
	// and valid false values are "false", "no", "off", "0" (not case sensitive).
	//bool GetBool(const std::string& section, const std::string& name, bool default_value) const;

	// Return true if the given section exists (section must contain at least
	// one name=value pair).
	//bool HasSection(const std::string& section) const;

	// Return true if a value exists with the given section and field names.
	//bool HasValue(const std::string& section, const std::string& name) const;

private:

	FILE* m_hfile;
	int m_error;
	//std::map<std::string, std::string> m_values;
	std::string m_filename;
	//int ini_parse_file();

	//static std::string MakeKey(const std::string& section, const std::string& name);
	//int GetValue(const char* section, const char* name, const char* value);

	int strlwr(char* str);
	int FindSection(const char* src, const char* section);
	int IsSection(const char* src);
	int IsOptionName(const char* src);
	int FindOption(const char* src, const char* option, char *Buffer, int BufferSize, int *set_default);

};

