#include "SortedResultMonitor.h"

Citizen& SortedResultMonitor::operator[](int index)
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
        if (objects[mid] == x)
            return mid;
        else if (objects[mid] > x)
            last = mid - 1;
        else
            first = mid + 1;
    }

    if (objects[first] > x)
        return first;
    else if (first == objectCount)
        return objectCount;
    else
        return first + 1;
}

void SortedResultMonitor::insertSorted(Citizen newObject)
{
    unique_lock<mutex> guard(lock);
    cv.wait(guard, [&] {return available;});
    available = false;

    int index = search(0, objectCount, newObject);
    for (int i = objectCount - 1; i >= index; i--)
        objects[i + 1] = objects[i];

    objects[index] = newObject;
    objectCount++;

    available = true;
    cv.notify_all();
}

void SortedResultMonitor::remove(int index)
{
    unique_lock<mutex> guard(lock);
    cv.wait(guard, [&] {return available;});
    available = false;

    for (int i = index; i < objectCount - 1; i++)
        objects[i] = objects[i + 1];

    objectCount--;

    available = true;
    cv.notify_all();
}
