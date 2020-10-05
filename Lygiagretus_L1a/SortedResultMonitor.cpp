#include "SortedResultMonitor.h"

Citizen& SortedResultMonitor::operator[](int index)
{
    if (index < 0 || index >= size) {
        throw out_of_range("Index out of array bounds");
    }
    return objects[index];
}

void SortedResultMonitor::insert(Citizen citizen, int index)
{
    for (int i = objectCount - 1; i >= index; i--)
        objects[i + 1] = objects[i];

    objects[index] = citizen;
    objectCount++;
}

void SortedResultMonitor::insertSorted(Citizen newObject)
{
    unique_lock<mutex> guard(lock);
    cv.wait(guard, [&] {return available;});
    available = false;

    //this_thread::sleep_for(std::chrono::milliseconds(50));

    bool found = false;
    for (int i = 0; i < objectCount; i++)
    {
        if (newObject < objects[i])
        {
            insert(newObject, i);
            found = true;
            break;
        }
    }

    if (!found)
        insert(newObject, objectCount);


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
