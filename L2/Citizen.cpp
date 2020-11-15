#include "Citizen.h"
#include <iostream>

int Citizen::compareTo(Citizen const& other) const
{
    int cmp = name.compare(other.name);
    if (cmp != 0)
        return cmp;

    if (age < other.age)
        return -1;
    else if (age > other.age)
        return 1;

    if (income == other.income)
        return 0;

    return income < other.income ? -1 : 1;
}
