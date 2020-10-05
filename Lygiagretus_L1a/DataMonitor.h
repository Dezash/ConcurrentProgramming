#pragma once
#include <mutex>
#include <condition_variable>
#include "Citizen.h"

using namespace std;

class DataMonitor
{
private:
    mutex lock;
    bool available;
    condition_variable cv;

    void insert(Citizen, int);

public:
    bool finished;
    Citizen* objects;
    int objectCount;
    int size;

    DataMonitor(int n) : objectCount(0), size(n), available(true), finished(false)
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
