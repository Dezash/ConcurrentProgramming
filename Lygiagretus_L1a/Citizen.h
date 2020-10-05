#pragma once
#include <string>
#include "nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

class Citizen
{
public:
    string name;
    int age;
    double income;


    Citizen(json data)
        : name(data["Name"]), age(data["Age"]), income(data["Income"])
    {}

    Citizen()
        : age(0), income(0)
    {}

    json getJson();

    int compareTo(Citizen const&) const;

    bool operator < (Citizen const& rhs) const
    {
        return compareTo(rhs) == -1;
    }

    bool operator > (Citizen const& rhs) const
    {
        return compareTo(rhs) == 1;
    }

    bool operator == (Citizen const& rhs) const
    {
        return compareTo(rhs) == 0;
    }
};
