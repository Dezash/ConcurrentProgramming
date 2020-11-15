#pragma once
#include <string>

using namespace std;

class Citizen
{
public:
    string name;
    int age;
    double income;


    Citizen(string _name, int _age, double _income)
        : name(_name), age(_age), income(_income)
    {}

    Citizen()
        : age(0), income(0)
    {}


    int compareTo(Citizen const&) const;

    bool operator < (Citizen const& rhs) const
    {
        return compareTo(rhs) < 0;
    }

    bool operator > (Citizen const& rhs) const
    {
        return compareTo(rhs) > 0;
    }

    bool operator == (Citizen const& rhs) const
    {
        return compareTo(rhs) == 0;
    }
};
