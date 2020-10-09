#include "DataMonitor.h"
#include <iostream>

void DataMonitor::add(Citizen newObject)
{
    while (objectCount == size)
    {
    }

#pragma omp critical
    {
        objects[objectCount++] = newObject;
    }
}

Citizen DataMonitor::pop()
{
    Citizen citizen;
    while (objectCount == 0 && !finished)
    {
    }
    
#pragma omp critical
    {
        if (objectCount > 0)
            citizen = objects[--objectCount];
    }

    return citizen;
}
