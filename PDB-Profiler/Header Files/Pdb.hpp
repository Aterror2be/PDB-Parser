#ifndef PDB_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include "dia2.h"

struct EnumMember
{
	std::string name;
	int value;
};

struct EnumData
{
	std::string name;
	std::string type;
	std::vector<EnumMember> members;
};

struct StructMember
{
	std::string name;
	std::string type;
	unsigned long long offset;
};

struct StructData
{
	std::string name;
	unsigned long long size;
	std::vector<StructMember> fields;
};

struct GlobalData
{
	std::string name;
	unsigned long long rva;
};

class PdbParser
{
public:

	PdbParser();

	bool InitPDB(const wchar_t* file_path);
	bool ReleasePDB();

	std::unordered_map<std::string, EnumData> GetEnumData();
	std::unordered_map<std::string, StructData> GetStructData();
	std::unordered_map<std::string, GlobalData> GetGlobalData();

private:
	const wchar_t* GetBasicTypeString(unsigned long base_type);
	const wchar_t* GuessTypeBySize(unsigned long type_size);
	std::string WideCharToUtf8(const wchar_t* wide_str);

	IDiaDataSource* dia_data_source;
	IDiaSession* dia_session;
	IDiaSymbol* dia_symbol;
};

#endif // !PDB_HPP