#pragma once

#include "pch.h"
#include "Circle.h"


extern "C" {
    class CircleToStringConverter :
        public IShapeToStringDataConverter
    {
    private:
        inline static CircleToStringConverter* _instance = nullptr;
        CircleToStringConverter();

    public:
        static CircleToStringConverter* getInstance();
        SHAPECONTAINER convert(IShape*);
        string toString();
    };
}
