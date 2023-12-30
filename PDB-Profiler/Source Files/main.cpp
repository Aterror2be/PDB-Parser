#include <iostream>
#include "Pdb.hpp"

const wchar_t* file_path = L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Unturned\\MonoBleedingEdge\\EmbedRuntime\\mono-2.0-bdwgc.pdb\\B16D88E459004E4EBCA92F4E686FDF941\\mono-2.0-bdwgc.pdb";
int main(int argc, char* argv[])
{
    PdbParser mono_parser = PdbParser();
    mono_parser.InitPDB(file_path);

    std::unordered_map<std::string, EnumData> enum_table = mono_parser.GetEnumData();
    std::unordered_map<std::string, StructData> struct_table = mono_parser.GetStructData();
    std::unordered_map<std::string, GlobalData> global_data_table = mono_parser.GetGlobalData();

    mono_parser.ReleasePDB();

    system("pause");
	return 0;
}