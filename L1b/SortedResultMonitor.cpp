#include "SortedResultMonitor.h"

CitizenComputed& SortedResultMonitor::operator[](int index)
{
    if (index < 0 || index >= size) {
        throw out_of_range("Index out of array bounds");
    }
    return objects[index];
}


int SortedResultMonitor::search(int first, int last, Citizen x) {
    int mid;
    while (first < last)
    {
        mid = (first + last) / 2;
        if (objects[mid].citizen == x)
            return mid;
        else if (objects[mid].citizen > x)
            last = mid - 1;
        else
            first = mid + 1;
    }

    if (objects[first].citizen > x)
        return first;
    else if (first == objectCount)
        return objectCount;
    else
        return first + 1;
}

void SortedResultMonitor::insertSorted(CitizenComputed newObject)
{
#pragma omp critical (writeLock)
    {
        int index = search(0, objectCount, newObject.citizen);
        for (int i = objectCount - 1; i >= index; i--)
            objects[i + 1] = objects[i];

        objects[index] = newObject;
        objectCount++;
    }
}
