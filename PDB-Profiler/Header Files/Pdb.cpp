#include "Pdb.hpp"

#include <iostream>
#include <Windows.h>

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