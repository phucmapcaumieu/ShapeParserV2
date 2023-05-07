﻿#include "Model/IShape.h"
#include "Controller/ShapeFactory.h"
#include "Controller/ConverterFactory.h"
#include "View/ShapePrinter.h"
#include "../utils/utils.h"

int main()
{
	int hr = _setmode(_fileno(stdout), _O_U16TEXT);
	typedef IShapeToStringDataConverter* (__cdecl* FN_SHAPE_CONVERTER)();
	typedef IParser* (__cdecl* FN_SHAPE_PARSER)();

	FN_SHAPE_PARSER fn_parser = nullptr;
	FN_SHAPE_CONVERTER fn_converter = nullptr;

	ConverterFactory c_factory;
	ShapeFactory s_factory;

	// register for parser		
	// register for converter	
	const fs::path find_path{ ".." };
	vector<wstring> dll_names = extractDLLFiles(find_path);

	vector<HINSTANCE> hinstLibs;
	for (wstring file_name : dll_names) {
		HINSTANCE hinstLib;
		BOOL fRunTimeLinkSuccess = FALSE;

		hinstLib = LoadLibrary(file_name.c_str());

		if (hinstLib != NULL)
		{
			hinstLibs.push_back(hinstLib);

			// TO-DO : CHECK BASE OF DLL INSTANCE

			fn_parser = (FN_SHAPE_PARSER)GetProcAddress(hinstLib, "getParserInstance");
			fn_converter = (FN_SHAPE_CONVERTER)GetProcAddress(hinstLib, "getConverterInstance");

			if (NULL != fn_parser) {
				IParser* instance = fn_parser();
				string shapeName = extractExtension(file_name);
				s_factory.registerWith(shapeName, instance);
			}

			if (NULL != fn_parser) {
				IShapeToStringDataConverter* instance = fn_converter();
				string shapeName = extractExtension(file_name);
				c_factory.registerWith(shapeName, instance);
			}

			fRunTimeLinkSuccess = TRUE;

		}

		if (!fRunTimeLinkSuccess) {
			wcout << L"Có lỗi khi đọc file: " << file_name << endl;
		}
	}

	// read file txt and store in a vector
	string input = "shape.txt";
	ifstream reader(input);
	string line = "";
	getline(reader, line);

	int count = 0;
	vector<IShape*> shapes;

	if (reader.good()) {
		count = (int)extractDouble(line)[0];

		for (int i = 0; i < count; i++) {
			getline(reader, line);
			stringstream buffer(line);
			string type;
			string data;
			getline(buffer, type, ':');
			getline(buffer, data);

			IParser* parser = s_factory.select(type);
			if (parser != nullptr) {
				IShape* shape = parser->parse(data);
				shapes.push_back(shape);
			}
		}
		reader.close();
	}

	wcout << L"Đang đọc file: " << wstring(input.begin(), input.end()) << "..." << endl;
	wcout << L"Dự kiến đọc được " << count << L" hình" << endl;
	wcout << L"Tìm thấy " << shapes.size() << L" hình." << endl;

	ShapePrinter printer;
	for (auto e : shapes) {
		IShapeToStringDataConverter* converter = nullptr;

		converter = c_factory.select(e->toString());
		printer.push(converter->convert(e));
	}

	printer.print();
	// sort vector with criteria 

	// print on screen

	for (auto& hinstance : hinstLibs) {
		BOOL freeResult = FreeLibrary(hinstance);

		if (!freeResult) {
			cout << "fail to free library" << endl;
		}
	}

	system("pause");
	return 0;

}
