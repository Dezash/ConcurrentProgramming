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
    int size;

public:
    bool finished;
    int objectCount;

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

    int search(int, int, Citizen);

    void remove(int);
};
