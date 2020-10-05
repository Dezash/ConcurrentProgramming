#pragma once
#include <mutex>
#include <condition_variable>
#include "CitizenComputed.h"

using namespace std;

class SortedResultMonitor
{
private:
    mutex lock;
    bool available;
    condition_variable cv;
    CitizenComputed* objects;
    int size;

public:
    bool finished;
    int objectCount;

    SortedResultMonitor(int n) : objectCount(0), size(n), available(true), finished(false)
    {
        objects = new CitizenComputed[n];
    }

    ~SortedResultMonitor()
    {
        delete[] objects;
    }

    CitizenComputed& operator[](int);

    void insertSorted(CitizenComputed);

    int search(int, int, Citizen);

    void remove(int);
};
