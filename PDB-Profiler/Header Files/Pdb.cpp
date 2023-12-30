#include "Pdb.hpp"

#include <string>
#include <vector>
#include <unordered_map>

#include "dia2.h"
#include "diacreate.h"

PdbParser::PdbParser()
{
	dia_data_source = nullptr;
	dia_session = nullptr;
	dia_symbol = nullptr;
}

bool PdbParser::InitPDB(const wchar_t* file_path)
{
	HRESULT result = NoRegCoCreate(L"msdia140.dll", _uuidof(DiaSource), _uuidof(IDiaDataSource), (void**)(&dia_data_source));
	if (FAILED(result))
	{
		printf("[!] CoCreateInstance Failed -> HRESULT: %08X\n", result);
		return false;
	}

	wchar_t ext[MAX_PATH] = {};
	_wsplitpath_s(file_path, NULL, 0, NULL, 0, NULL, 0, ext, MAX_PATH);
	if (_wcsicmp(ext, L".pdb") != 0)
	{
		printf("[!] Invalid PDB Image\n");
		return false;
	}

	result = dia_data_source->loadDataFromPdb(file_path);
	if (FAILED(result))
	{
		wprintf(L"[!] IDiaDataSource::loadDataFromPdb Failed -> HRESULT: %08X\n", result);
		CoUninitialize();
		return false;
	}

	result = dia_data_source->openSession(&dia_session);
	if (FAILED(result))
	{
		printf("[!] IDiaDataSource::openSession Failed -> HRESULT: %08X\n", result);
		return false;
	}

	result = dia_session->get_globalScope(&dia_symbol);
	if (FAILED(result))
	{
		printf("[!] IDiaSession::get_globalScope Failed -> HRESULT: %08X\n", result);
		return false;
	}
	return true;
}

bool PdbParser::ReleasePDB()
{
	if (dia_data_source)
	{
		dia_data_source->Release();
		dia_data_source = nullptr;
	}

	if (dia_symbol)
	{
		dia_symbol->Release();
		dia_symbol = nullptr;
	}

	if (dia_session)
	{
		dia_session->Release();
		dia_session = nullptr;
	}
	CoUninitialize();
	return true;
}

std::unordered_map<std::string, EnumData> PdbParser::GetEnumData()
{
	IDiaEnumSymbols* enum_symbols = nullptr;
	HRESULT result = dia_symbol->findChildren(SymTagEnum, NULL, nsNone, &enum_symbols);
	if (FAILED(result))
	{
		printf("[!] IDiaSymbols::findChildren Failed -> HRESULT: %08X\n", result);
		return {};
	}

	ULONG celt = 0;
	IDiaSymbol* symbol = nullptr;
	std::unordered_map<std::string, EnumData> enum_table = {};
	while (SUCCEEDED(enum_symbols->Next(1, &symbol, &celt)) && (celt == 1))
	{
		wchar_t* name = nullptr;
		result = symbol->get_name(&name);
		if (FAILED(result))
		{
			printf("[!] IDiaSymbol::get_name Failed -> HRESULT: %08X\n", result);
			continue;
		}

		DWORD type_data = 0;
		result = symbol->get_baseType(&type_data);
		if (FAILED(result))
		{
			printf("[!] IDiaSymbol::get_baseType Failed -> HRESULT: %08X\n", result);
			continue;
		}

		IDiaEnumSymbols* child_enum_symbols = nullptr;
		result = symbol->findChildren(SymTagData, NULL, nsNone, &child_enum_symbols);
		if (FAILED(result))
		{
			printf("[!] IDiaSymbol::findChildren Failed -> HRESULT: %08X\n", result);
			continue;
		}

		EnumData enum_data;
		enum_data.name = WideCharToUtf8(name);
		enum_data.type = WideCharToUtf8(GetBasicTypeString(type_data));

		ULONG celt = 0;
		IDiaSymbol* child_symbol = nullptr;
		while (SUCCEEDED(child_enum_symbols->Next(1, &child_symbol, &celt)) && (celt == 1))
		{
			wchar_t* child_name = nullptr;
			result = child_symbol->get_name(&child_name);
			if (FAILED(result))
			{
				printf("[!] IDiaSymbol::get_name Failed -> HRESULT: %08X\n", result);
				continue;
			}

			VARIANT value;
			VariantInit(&value);
			result = child_symbol->get_value(&value);
			if (FAILED(result))
			{
				printf("[!] IDiaSymbol::get_name Failed -> HRESULT: %08X\n", result);
				SysFreeString(child_name);
				continue;
			}

			//build member data
			EnumMember enum_member;
			enum_member.name = WideCharToUtf8(child_name);
			enum_member.value = value.lVal;

			//add data to members list
			enum_data.members.push_back(enum_member);

			VariantClear(&value);
			SysFreeString(child_name);
			child_symbol->Release();
		}
		enum_table.insert({ enum_data.name, enum_data });

		if (name != nullptr)
			SysFreeString(name);

		child_enum_symbols->Release();
		symbol->Release();
	}
	enum_symbols->Release();
	return enum_table;
}

std::unordered_map<std::string, StructData> PdbParser::GetStructData()
{
	IDiaEnumSymbols* structure_symbols = nullptr;
	HRESULT result = dia_symbol->findChildren(SymTagUDT, NULL, nsNone, &structure_symbols);
	if (FAILED(result))
	{
	    printf("[!] IDiaSymbols::findChildren Failed -> HRESULT: %08X\n", result);
		return {};
	}

	ULONG celt = 0;
	IDiaSymbol* struct_symbol = nullptr;
	std::unordered_map<std::string, StructData> structure_table = {};
	while (SUCCEEDED(structure_symbols->Next(1, &struct_symbol, &celt)) && (celt == 1))
	{
		DWORD udt_kind = 0;
		result = struct_symbol->get_udtKind(&udt_kind);
		if (FAILED(result))
		{
			printf("[!] IDiaSymbols::get_udtKind Failed -> HRESULT: %08X\n", result);
			continue;
		}

		//not struct skip
		if (udt_kind != 0)
		{
			struct_symbol->Release();
			continue;
		}

	    wchar_t* name = nullptr;
	    result = struct_symbol->get_name(&name);
	    if (FAILED(result))
	    {
	        printf("[!] IDiaSymbols::get_name Failed -> HRESULT: %08X\n", result);
	        continue;
	    }

	    IDiaEnumSymbols* child_symbol = nullptr;
	    result = struct_symbol->findChildren(SymTagData, NULL, nsNone, &child_symbol);
	    if (FAILED(result))
	    {
	        printf("[!] IDiaSymbols::findChildren Failed -> HRESULT: %08X\n", result);
	        continue;
	    }

		ULONGLONG struct_size = 0;
		result = struct_symbol->get_length(&struct_size);
		if (FAILED(result))
		{
			printf("[!] IDiaSymbols::get_length Failed -> HRESULT: %08X\n", result);
			continue;
		}

		StructData struct_data;
		struct_data.name = WideCharToUtf8(name);
		struct_data.size = struct_size;

		ULONG celt = 0;
	    IDiaSymbol* structure_member = nullptr;
	    while (SUCCEEDED(child_symbol->Next(1, &structure_member, &celt)) && (celt == 1))
	    {
	        wchar_t* member_name = nullptr;
	        result = structure_member->get_name(&member_name);
	        if (FAILED(result))
	        {
	            printf("[!] IDiaSymbols::get_name Failed -> HRESULT: %08X\n", result);
	            continue;
	        }

	        LONG member_offset = 0;
	        result = structure_member->get_offset(&member_offset);
	        if (FAILED(result))
	        {
	            printf("[!] IDiaSymbols::get_offset Failed -> HRESULT: %08X\n", result);
	            continue;
	        }

	        IDiaSymbol* member_type = nullptr;
	        result = structure_member->get_type(&member_type);
	        if (FAILED(result))
	        {
	            printf("[!] IDiaSymbols::get_type Failed -> HRESULT: %08X\n", result);
	            continue;
	        }

	        wchar_t* member_type_str = nullptr;
	        result = member_type->get_name(&member_type_str);
	        if (FAILED(result))
	        {
	            printf("[!] IDiaSymbols::get_name Failed -> HRESULT: %08X\n", result);
	            continue;
	        }

	        ULONGLONG type_size = 0;
	        result = member_type->get_length(&type_size);
	        if (FAILED(result))
	        {
	            printf("[!] IDiaSymbols::get_length Failed -> HRESULT: %08X\n", result);
	            continue;
	        }

	        if (member_type_str == nullptr || wcscmp(member_type_str, L"(null)") == 0)
	        {
	            member_type_str = const_cast<wchar_t*>(GuessTypeBySize(type_size));
	        }

			StructMember struct_member;
			struct_member.name = WideCharToUtf8(member_name);
			struct_member.type = WideCharToUtf8(member_type_str);
			struct_member.offset = member_offset;

			struct_data.fields.push_back(struct_member);

			if(member_name != nullptr)
				SysFreeString(member_name);
	        member_type->Release();
	        structure_member->Release();
	    }
		structure_table.insert({ struct_data.name, struct_data});

		if(name != nullptr)
			SysFreeString(name);

		child_symbol->Release();
	    struct_symbol->Release();
	}
	structure_symbols->Release();
	return structure_table;
}

std::unordered_map<std::string, GlobalData> PdbParser::GetGlobalData()
{
   IDiaEnumSymbols* data_symbols = nullptr;
   HRESULT result = dia_symbol->findChildren(SymTagData, NULL, nsNone, &data_symbols);
   if (FAILED(result))
   {
       printf("[!] IDiaSymbols::findChildren Failed -> HRESULT: %08X\n", result);
	   return {};
   }

   ULONG celt = 1;
   IDiaSymbol* data_symbol = nullptr;
   std::unordered_map<std::string, GlobalData> global_data_table = {};
   while (SUCCEEDED(data_symbols->Next(1, &data_symbol, &celt)) && (celt == 1))
   {
       wchar_t* name = nullptr;
       result = data_symbol->get_name(&name);
	   if (FAILED(result))
	   {
		   printf("[!] IDiaSymbols::get_name Failed -> HRESULT: %08X\n", result);
		   continue;
	   }

	   DWORD rva = 0;
	   result = data_symbol->get_relativeVirtualAddress(&rva);
	   if (FAILED(result))
	   {
		   printf("[!] IDiaSymbols::get_relativeVirtualAddress Failed -> HRESULT: %08X\n", result);
		   continue;
	   }

	   GlobalData global_data;
	   global_data.name = WideCharToUtf8(name);
	   global_data.rva = rva;

	   global_data_table.insert({ global_data.name, global_data });

	   if (name != nullptr)
		   SysFreeString(name);
       data_symbol->Release();
   }
   data_symbols->Release();
   return global_data_table;
}

const wchar_t* PdbParser::GetBasicTypeString(unsigned long base_type)
{
	switch (base_type)
	{
	case btNoType: return L"None";
	case btVoid: return L"void";
	case btChar: return L"char";
	case btWChar: return L"wchar_t";
	case btInt: return L"int";
	case btUInt: return L"uint32_t";
	case btFloat: return L"float";
	case btBCD: return L"BCD";
	case btBool: return L"bool";
	case btLong: return L"long";
	case btULong:return L"unsigned long";
	case btCurrency:return L"Currency";
	case btDate:return L"Date";
	case btVariant:return L"Variant";
	case btComplex:return L"Complex";
	case btBit:return L"Bit";
	case btBSTR:return L"BSTR";
	case btHresult:return L"HRESULT";
	case btChar16:return L"char_16_t";
	case btChar32: return L"char_32_t";
	case btChar8: return L"char_8_t";
	default: return L"UnknownType";
	}
}

const wchar_t* PdbParser::GuessTypeBySize(unsigned long type_size)
{
	switch (type_size)
	{
	case 1: return L"db";
	case 2: return L"dw";
	case 4: return L"dd";
	case 8: return L"dq";
	case 16: return L"do";
	case 32: return L"dy";
	case 64: return L"dz";
	default: break;
	}

	wchar_t array_type[128] = {};
	swprintf_s(array_type, 128, L"buf ? (%i)", type_size);
	return array_type;
}

std::string PdbParser::WideCharToUtf8(const wchar_t* wide_str)
{
	int length = wcslen(wide_str);
	int size = WideCharToMultiByte(CP_UTF8, 0, wide_str, length, nullptr, 0, nullptr, nullptr);
	std::string str(size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wide_str, length, &str[0], size, nullptr, nullptr);
	return str;
}