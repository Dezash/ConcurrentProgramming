#include "DataMonitor.h"

void DataMonitor::add(Citizen newObject)
{
    unique_lock<mutex> guard(lock);
    cv.wait(guard, [&] {return objectCount != size;});
    objects[objectCount++] = newObject;
    cv.notify_all();
}

Citizen DataMonitor::pop()
{
    std::unique_lock<mutex> guard(lock);
    cv.wait(guard, [&] {return available;});
    available = false;

    cv.wait(guard, [&] {return finished || objectCount > 0;});

    Citizen citizen = objectCount > 0 ? objects[--objectCount] : Citizen();

    available = true;
    cv.notify_all();
    return citizen;
}
