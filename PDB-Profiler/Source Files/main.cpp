#include <iostream>
#include <Windows.h>

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <tchar.h>
#include <string.h>

#include "dia2.h"
#include "diacreate.h"

const wchar_t* file_path = L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Unturned\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc.pdb\\B16D88E459004E4EBCA92F4E686FDF941\\mono-2.0-bdwgc.pdb";
IDiaDataSource* dia_data_source;
IDiaSession* dia_session;
IDiaSymbol* dia_symbol;

const wchar_t* GetBasicTypeString(DWORD base_type)
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

const wchar_t* GuessTypeBySize(DWORD type_size)
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

int main(int argc, char* argv[])
{
    //init
    HRESULT result = NoRegCoCreate(L"msdia140.dll", _uuidof(DiaSource), _uuidof(IDiaDataSource), (void**)(&dia_data_source));
    if (FAILED(result))
    {
        printf("[!] CoCreateInstance Failed -> HRESULT: %08X\n", result);
        return -1;
    }

    wchar_t ext[MAX_PATH] = {};
    _wsplitpath_s(file_path, NULL, 0, NULL, 0, NULL, 0, ext, MAX_PATH);
    if (_wcsicmp(ext, L".pdb") != 0)
    {
        printf("[!] Invalid PDB Image\n");
        return -1;
    }

    result = dia_data_source->loadDataFromPdb(file_path);
    if (FAILED(result))
    {
        wprintf(L"[!] IDiaDataSource::loadDataFromPdb Failed -> HRESULT: %08X\n", result);
        CoUninitialize();
        return -1;
    }

    result = dia_data_source->openSession(&dia_session);
    if (FAILED(result))
    {
        printf("[!] IDiaDataSource::openSession Failed -> HRESULT: %08X\n", result);
        return -1;
    }

    result = dia_session->get_globalScope(&dia_symbol);
    if (FAILED(result))
    {
        printf("[!] IDiaSession::get_globalScope Failed -> HRESULT: %08X\n", result);
        return -1;
    }


    //dump enums

    //IDiaEnumSymbols* enum_symbols = nullptr;
    //result = dia_symbol->findChildren(SymTagEnum, NULL, nsNone, &enum_symbols);
    //if (FAILED(result))
    //{
    //    printf("[!] IDiaSymbols::findChildren Failed -> HRESULT: %08X\n", result);
    //    return -1;
    //}

    //IDiaSymbol* symbol;
    //ULONG celt = 0;
    //while (SUCCEEDED(enum_symbols->Next(1, &symbol, &celt)) && (celt == 1))
    //{
    //    // Retrieve and print the name of each enum
    //    wchar_t* name = nullptr;
    //    result = symbol->get_name(&name);
    //    if (FAILED(result))
    //    {
    //        printf("[!] IDiaSymbol::get_name Failed -> HRESULT: %08X\n", result);
    //        continue;
    //    }

    //    DWORD type_data = 0;
    //    result = symbol->get_baseType(&type_data);
    //    if (FAILED(result))
    //    {
    //        printf("[!] IDiaSymbol::get_baseType Failed -> HRESULT: %08X\n", result);
    //        continue;
    //    }

    //    wprintf(L"%s : %ws\n", name, GetBasicTypeString(type_data));
    //    SysFreeString(name);

    //    IDiaEnumSymbols* child_enum_symbols = nullptr;
    //    result = symbol->findChildren(SymTagData, NULL, nsNone, &child_enum_symbols);
    //    if (FAILED(result))
    //    {
    //        printf("[!] IDiaSymbol::findChildren Failed -> HRESULT: %08X\n", result);
    //        continue;
    //    }

    //    IDiaSymbol* child_symbol = nullptr;
    //    ULONG celt = 0;
    //    while (SUCCEEDED(child_enum_symbols->Next(1, &child_symbol, &celt)) && (celt == 1))
    //    {
    //        wchar_t* child_name = nullptr;
    //        result = child_symbol->get_name(&child_name);
    //        if (FAILED(result))
    //        {
    //            printf("[!] IDiaSymbol::get_name Failed -> HRESULT: %08X\n", result);
    //            continue;
    //        }

    //        VARIANT value;
    //        VariantInit(&value);
    //        result = child_symbol->get_value(&value);
    //        if (FAILED(result))
    //        {
    //            printf("[!] IDiaSymbol::get_name Failed -> HRESULT: %08X\n", result);
    //            SysFreeString(child_name);
    //            continue;
    //        }

    //        printf("\t%ws = %i\n", child_name, value.lVal);

    //        VariantClear(&value);
    //        SysFreeString(child_name);
    //        child_symbol->Release();
    //    }
    //    symbol->Release();
    //}
    //enum_symbols->Release();

    //dump structures
    //IDiaEnumSymbols* structure_symbols = nullptr;
    //result = dia_symbol->findChildren(SymTagUDT, NULL, nsNone, &structure_symbols);
    //if (FAILED(result))
    //{
    //    printf("[!] IDiaSymbols::findChildren Failed -> HRESULT: %08X\n", result);
    //    return -1;
    //}

    //IDiaSymbol* struct_symbol;
    //ULONG celt = 0;
    //while (SUCCEEDED(structure_symbols->Next(1, &struct_symbol, &celt)) && (celt == 1))
    //{
    //    wchar_t* name = nullptr;
    //    result = struct_symbol->get_name(&name);
    //    if (FAILED(result))
    //    {
    //        printf("[!] IDiaSymbols::get_name Failed -> HRESULT: %08X\n", result);
    //        continue;
    //    }

    //    printf("struct %ws\n{\n", name);
    //    SysFreeString(name);

    //    IDiaEnumSymbols* child_symbol = nullptr;
    //    result = struct_symbol->findChildren(SymTagData, NULL, nsNone, &child_symbol);
    //    if (FAILED(result))
    //    {
    //        printf("[!] IDiaSymbols::findChildren Failed -> HRESULT: %08X\n", result);
    //        continue;
    //    }

    //    IDiaSymbol* structure_member = nullptr;
    //    ULONG celt = 0;
    //    DWORD last_offset = 0;
    //    while (SUCCEEDED(child_symbol->Next(1, &structure_member, &celt)) && (celt == 1))
    //    {
    //        wchar_t* member_name = nullptr;
    //        result = structure_member->get_name(&member_name);
    //        if (FAILED(result))
    //        {
    //            printf("[!] IDiaSymbols::get_name Failed -> HRESULT: %08X\n", result);
    //            continue;
    //        }

    //        LONG member_offset = 0;
    //        result = structure_member->get_offset(&member_offset);
    //        if (FAILED(result))
    //        {
    //            printf("[!] IDiaSymbols::get_offset Failed -> HRESULT: %08X\n", result);
    //            continue;
    //        }

    //        IDiaSymbol* member_type = nullptr;
    //        result = structure_member->get_type(&member_type);
    //        if (FAILED(result))
    //        {
    //            printf("[!] IDiaSymbols::get_type Failed -> HRESULT: %08X\n", result);
    //            continue;
    //        }

    //        wchar_t* member_type_str = nullptr;
    //        result = member_type->get_name(&member_type_str);
    //        if (FAILED(result))
    //        {
    //            printf("[!] IDiaSymbols::get_name Failed -> HRESULT: %08X\n", result);
    //            continue;
    //        }

    //        ULONGLONG type_size = 0;
    //        result = member_type->get_length(&type_size);
    //        if (FAILED(result))
    //        {
    //            printf("[!] IDiaSymbols::get_length Failed -> HRESULT: %08X\n", result);
    //            continue;
    //        }

    //        if (member_type_str == nullptr || wcscmp(member_type_str, L"(null)") == 0)
    //        {
    //            member_type_str = const_cast<wchar_t*>(GuessTypeBySize(type_size));
    //        }

    //        printf("\t0x%X %ws %ws\n", member_offset, member_name, member_type_str);

    //        last_offset = member_offset;

    //        SysFreeString(member_name);
    //        member_type->Release();
    //        structure_member->Release();
    //    }
    //    printf("};\n\n");
    //    struct_symbol->Release();
    //}
    //structure_symbols->Release();

    
    //global data
    IDiaEnumSymbols* data_symbols = nullptr;
    result = dia_symbol->findChildren(SymTagData, NULL, nsNone, &data_symbols);
    if (FAILED(result))
    {
        printf("[!] IDiaSymbols::findChildren Failed -> HRESULT: %08X\n", result);
        return -1;
    }

    IDiaSymbol* data_symbol = nullptr;
    ULONG celt = 1;
    while (SUCCEEDED(data_symbols->Next(1, &data_symbol, &celt)) && (celt == 1))
    {
        wchar_t* data_name = nullptr;
        data_symbol->get_name(&data_name);

		if (wcscmp(data_name, L"mono_root_domain") == 0)
		{
            DWORD rva = 0;
            data_symbol->get_relativeVirtualAddress(&rva);

			printf("%ws\t%x\n", data_name, rva);
		}

        data_symbol->Release();
    }
    data_symbols->Release();

	system("pause");

    //clean up
    if (dia_symbol)
    {
        dia_symbol->Release();
        dia_symbol = NULL;
    }
    if (dia_session) 
    {
        dia_session->Release();
        dia_session = NULL;
    }
    CoUninitialize();

	return 0;
}