#pragma once
#include <mutex>
#include <condition_variable>
#include "Citizen.h"

using namespace std;

class SortedResultMonitor
{
private:
    mutex lock;
    bool available;
    condition_variable cv;
    Citizen* objects;
    void insert(Citizen, int);

public:
    bool finished;
    int objectCount;
    int size;

    SortedResultMonitor(int n) : objectCount(0), size(n), available(true), finished(false)
    {
        objects = new Citizen[n];
    }

    ~SortedResultMonitor()
    {
        delete[] objects;
    }

    Citizen& operator[](int);

    void insertSorted(Citizen);

    void remove(int);
};