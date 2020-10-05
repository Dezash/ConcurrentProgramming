#include "Citizen.h"

int Citizen::compareTo(Citizen const& other) const
{
    int cmp = name.compare(other.name);
    if (cmp != 0)
        return cmp;

    if (age < other.age)
        return -1;
    else if (age > other.age)
        return 1;

    return income < other.income ? -1 : 1;
}

json Citizen::getJson()
{
    json citizen;
    citizen["name"] = name;
    citizen["age"] = age;
    citizen["income"] = income;

    return citizen;
}
