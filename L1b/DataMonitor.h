#pragma once
#include <condition_variable>
#include "Citizen.h"

using namespace std;

class DataMonitor
{
public:
    bool finished;
    Citizen* objects;
    int objectCount;
    int size;

    DataMonitor(int n) : objectCount(0), size(n), finished(false)
    {
        objects = new Citizen[n];
    }

    ~DataMonitor()
    {
        delete[] objects;
    }

    void add(Citizen);

    Citizen pop();
};
