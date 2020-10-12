#include <iostream>
#include "nlohmann/json.hpp"
#include <fstream>
#include <thread>
#include <mutex>
#include <iomanip>
#include "Citizen.h"
#include "DataMonitor.h"
#include "SortedResultMonitor.h"
#include <openssl/sha.h>
#include <sstream>


using namespace std;
using json = nlohmann::json;

const string DATA_FILE_1 = "IFF81_LaurinaviciusG_L1_dat_1.json";
const string DATA_FILE_2 = "IFF81_LaurinaviciusG_L1_dat_2.json";
const string DATA_FILE_3 = "IFF81_LaurinaviciusG_L1_dat_3.json";
const string RES_FILE = "IFF81_LaurinaviciusG_L1_rez.txt";
const int THREAD_COUNT = 9;
condition_variable cv;

Citizen* getCitizens(string fileName, int& n)
{
    ifstream in(fileName);
    if (!in.good()) {
        cout << "Cannot open file '" << fileName << "'\n";
        cin;
        exit(1);
    }

    json data;
    in >> data;
    in.close();

    n = data.size();
    unsigned int count = 0;
    Citizen* citizens = new Citizen[n];

    for (const auto& item : data.items())
    {
        Citizen citizen = Citizen(item.value());
        citizens[count++] = citizen;
    }

    return citizens;
}

string sha256(const string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

static void filterData(DataMonitor& readMonitor, SortedResultMonitor& filterMonitor, int n)
{
    for (int i = 0; i < n; i++)
    {
        Citizen citizen = readMonitor.pop();

        //cout << "filterData " << citizen.name << " " << citizen.age << " " << citizen.income << endl;

        if (citizen.income >= 40500 && citizen.income < 118000)
        {
            string hash = sha256(citizen.name + to_string(citizen.age) + to_string(citizen.income));

            auto computed = CitizenComputed(citizen, hash);
            filterMonitor.insertSorted(computed);
        }

        if (readMonitor.finished && readMonitor.objectCount == 0)
            break;

    }

    filterMonitor.finished = true;
    cv.notify_all();
}


static void printResults(const string fileName, SortedResultMonitor& filterMonitor, int n)
{
    ofstream out(fileName);
    out << left << setw(30) << "Name|" << setw(10) << "Age|" << setw(10) << "Income|" << setw(10) << "SHA256|" << '\n';
    for (int i = 0; i < filterMonitor.objectCount; i++)
    {
        CitizenComputed computed = filterMonitor[i];
        Citizen citizen = computed.citizen;
        out << left << setw(30) << citizen.name << setw(10) << citizen.age << setw(10) << citizen.income << setw(10) << computed.hash << '\n';
    }

    out << "Is pradziu duomenu: " << n << "\nAtfiltruota duomenu: " << filterMonitor.objectCount;
    out.close();
}

int main()
{
    clock_t tStart = clock();

    int n;
    Citizen* citizens = getCitizens(DATA_FILE_1, n);

    DataMonitor readMonitor(n / 2);
    SortedResultMonitor filterMonitor(n);

    printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);

    thread threads[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        threads[i] = thread(filterData, ref(readMonitor), ref(filterMonitor), n);
    }

    for (int i = 0; i < n; i++)
    {
        //cout << "writeData " << citizens[i].name << " " << citizens[i].age << " " << citizens[i].income << endl;
        readMonitor.add(citizens[i]);
    }
    readMonitor.finished = true;

    mutex lock;
    unique_lock<mutex> guard(lock);
    

    cv.wait(guard, [&] {return filterMonitor.finished;});

    printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        threads[i].join();
    }

    printResults(RES_FILE, filterMonitor, n);
    delete[] citizens;

    printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
    return 0;
}
