#include "DataMonitor.h"
#include <iostream>

bool DataMonitor::add(Citizen newObject)
{
    if (objectCount == size)
    {
        return false;
    }

#pragma omp critical
    {
        objects[objectCount++] = newObject;
    }

    return true;
}

Citizen DataMonitor::pop(bool &success)
{
    Citizen citizen;
    if (objectCount == 0 && !finished)
    {
        success = false;
        return citizen;
    }
    
    
#pragma omp critical
    {
        if (objectCount > 0)
            citizen = objects[--objectCount];
    }

    success = true;
    return citizen;
}
