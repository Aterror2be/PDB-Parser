#include <iostream>
#include <Windows.h>

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <tchar.h>
#include <string.h>

#include "dia2.h"

const wchar_t* file_path = L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Unturned\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc.pdb\\B16D88E459004E4EBCA92F4E686FDF941\\mono-2.0-bdwgc.pdb";
IDiaDataSource* dia_data_source;
IDiaSession* dia_session;
IDiaSymbol* dia_symbol;
DWORD machine_type = CV_CFL_80386;

int main(int argc, char* argv[])
{
    //init
    HRESULT result = CoInitialize(NULL);
    if (FAILED(result))
    {
        printf("[!] CoInitialize Failed -> HRESULT: %08X\n", result);
        return -1;
    }

    result = CoCreateInstance(__uuidof(DiaSource), NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)&dia_data_source);
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

    IDiaEnumSymbols* enum_symbols = nullptr;
    result = dia_symbol->findChildren(SymTagEnum, NULL, nsNone, &enum_symbols);
    if (FAILED(result))
    {
        printf("[!] IDiaSymbols::findChildren Failed -> HRESULT: %08X\n", result);
        return -1;
    }

    IDiaSymbol* symbol;
    ULONG celt = 0;
    while (SUCCEEDED(enum_symbols->Next(1, &symbol, &celt)) && (celt == 1))
    {
        // Retrieve and print the name of each enum
        BSTR name = nullptr;
        result = symbol->get_name(&name);
        if (SUCCEEDED(result)) {
            wprintf(L"%s\n", name);
            SysFreeString(name);
        }
    }

    //

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