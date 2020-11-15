#pragma once
#include "Citizen.h"

class CitizenComputed : public Citizen
{
public:
	Citizen citizen;
	string hash;

	CitizenComputed(Citizen citizen, string hash) : citizen(citizen), hash(hash) {}

	CitizenComputed() {}
};
