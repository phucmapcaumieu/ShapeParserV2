﻿// Load MVC component
#include "Model/IShape.h"
#include "Controller/ShapeFactory.h"
#include "Controller/ConverterFactory.h"
#include "View/ShapePrinter.h"
#include "View/ColumnStrategy.h"
#include "View/DetailStrategy.h"
#include "View/LoadingFailStrategy.h"
#include "View/IStrategy.h"

// Load depenencies
#include "../utils/utils.h"
#include "../Exception/Exception.h"

// Main function
int main()
{
    // ------------ Phase 1: Prepare ------------
	// this variable hold the result of 
	// setting up mode for print out utf16 text
	bool hr = _setmode(_fileno(stdout), _O_U16TEXT);
	
	// define function pointer and return type of it.
	typedef IParser* (__cdecl* FN_SHAPE_PARSER)();
	typedef IShapeToStringDataConverter* (__cdecl* FN_SHAPE_CONVERTER)();

	// Initialize function pointer as nullptr
	FN_SHAPE_PARSER fn_parser = nullptr;

	// Initialize function pointer as nullptr
	FN_SHAPE_CONVERTER fn_converter = nullptr;

	// Initial empty factory
	ConverterFactory converter_factory;
	ShapeFactory parser_factory;

	// --------------------------------------------------
	// ------------ Phase 2: Load dependencies ----------
	// --------------------------------------------------
	// Find all file dll exists in project
	const fs::path find_path{ ".." };
	vector<wstring> dll_names = extractDLLFiles(find_path);
	
	// vector store all alive instance of DLL 
	vector<HINSTANCE> hinstLibs;
	for (wstring file_name : dll_names) {
		// variable that store DLL instance
		HINSTANCE hinstLib;
		BOOL fRunTimeLinkSuccess = FALSE;

		// load DLL
		hinstLib = LoadLibrary(file_name.c_str());

		// If success to load file DLL
		if (hinstLib != NULL)
		{
			fRunTimeLinkSuccess = TRUE;

			// load address of function
			fn_parser = (FN_SHAPE_PARSER)GetProcAddress(hinstLib, "getParserInstance");
			fn_converter = (FN_SHAPE_CONVERTER)GetProcAddress(hinstLib, "getConverterInstance");

			// get the shape name
			string shapeName = extractExtension(file_name);

			if (NULL != fn_parser) {
				// Invoked function to initial instance
				IParser* instance = fn_parser();
				// Register with parser factory
				parser_factory.registerWith(shapeName, instance);
			}

			if (NULL != fn_converter) {
				// Invoked function to initial instance
				IShapeToStringDataConverter* instance = fn_converter();
				// Register with converter factory
				converter_factory.registerWith(shapeName, instance);			
			}
			
			// Check whether load function is success or not
			fRunTimeLinkSuccess = fn_converter && fn_parser;
			
			if (!fRunTimeLinkSuccess) {
				wcout << L"Có lỗi khi đọc file: " << file_name << endl;
				continue;
			}

			// push lib instace to vector 
			// to manually free it.
			hinstLibs.push_back(hinstLib);
		}
	}
	// --------------------------------------------
	// ------------ Phase 3: Read data ------------
 	// --------------------------------------------

	// Open file to read
	string input = "shapes.txt";
	ifstream reader(input);
	string line = "";

	vector<IShape*> shapes;
	int count = 0; // the actual shape must read
	if (reader.good()) {
		// Read line and parse to shape
		while (!reader.eof()) {
			/*	
			This block of code will plit 
			data with delimiter is ':'
			Example:
				source data="Circle: r=10"
				-- after split --
				type = "Circle"
				data = " r=10"
			*/
			count += 1;
			getline(reader, line);
			stringstream buffer(line);
			string type; // hold the name type of parser
			string data;
			getline(buffer, type, ':');
			getline(buffer, data);

			// Select parser
			IParser* parser = parser_factory.select(type);

			// If parser is not registed
			if (parser == nullptr) {
				continue;
			}

			// Parse data and return instance of shape
			try {
				IShape* shape = parser->parse(data);
				shapes.push_back(shape); // store it
			}
			catch (exception*& ex)
			{
				string message(ex->what());
				wcout << W_STR(message) << endl;
			}
		}
		reader.close();
	}

	// --------------------------------------------------
	// ------------ Phase 4: Print to screen ------------
	// --------------------------------------------------
	
	// Pre-print phase: Convert Shape to string
	IShapeToStringDataConverter* converter = nullptr;
	vector<SHAPECONTAINER> container;
	for (auto shape : shapes) {
		// select converter
		converter = converter_factory.select(shape->toString()); 
		container.push_back(converter->convert(shape));
	}

	// Set printer to print to screen
	ShapePrinter printer;
	IStrategy* strategy = nullptr;

	// Set strategy for specific case
	if (count == container.size()) {
		strategy = DetailStrategy::getInstance();
	}
	else {
		strategy = LoadingFailStrategy::getInstance();
	}

	printer.setStrategy(strategy);
	printer.print(container, input, count);
	wcout << endl;
	
	// Set strategy to print out result of shape
	// after calculate Perimeter and Area.
	// Sort ascending by area
	sort(container.begin(), container.end(), byArea);
	strategy = ColumnStrategy::getInstance();
	printer.setStrategy(strategy);
	printer.print(container, input, count);


	// ----------------------------------------------
	// ------------ Phase 5: Free memory ------------
	// ----------------------------------------------
	// delete instance of shape.
	for (auto& shape : shapes) {
		delete shape;
	}

	// free libs
	for (auto& hinstance : hinstLibs) {
		BOOL freeResult = FreeLibrary(hinstance);

		if (!freeResult) {
			wcout << "fail to free library" << endl;
		}
	}

	system("pause");
	return 0;
}
