﻿#include <windows.h>
#include <objbase.h>
#include "pch.h"
#include "CircleParser.h"
#include "CircleToStringConverter.h"

// Define the Shape interface
extern "C" {
    SHAPE_API IParser* __stdcall getParserInstance()
    {
        IParser* result = CircleParser::getInstance();
        return result;
    }

    SHAPE_API IShapeToStringDataConverter* __stdcall getConverterInstance()
    {
        IShapeToStringDataConverter* result = CircleToStringConverter::getInstance();
        return result;
    }
}