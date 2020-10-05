#include <iostream>
#include "nlohmann/json.hpp"
#include <fstream>
#include <thread>
#include <mutex>
#include <iomanip>
#include "Citizen.h"
#include "DataMonitor.h"
#include "SortedResultMonitor.h"

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


static void filterData(DataMonitor& readMonitor, SortedResultMonitor& filterMonitor)
{
    while (!readMonitor.finished || readMonitor.objectCount > 0)
    {
        if (readMonitor.objectCount == 0)
            continue;

        Citizen citizen = readMonitor.pop();
        if (citizen.age != 0)
        {
            //cout << "filterData " << citizen.name << " " << citizen.age << " " << citizen.income << endl;

            if (citizen.income >= 40500 && citizen.income < 118000)
            {
                filterMonitor.insertSorted(citizen);
            }
                
        }
    }

    filterMonitor.finished = true;
    cv.notify_all();
}

static void writeData(DataMonitor& monitor, Citizen* citizens, int n)
{
    for (int i = 0; i < n; i++)
    {
        //cout << "writeData " << citizens[i].name << " " << citizens[i].age << " " << citizens[i].income << endl;
        monitor.add(citizens[i]);
    }
    monitor.finished = true;
}

static void printResults(const string fileName, SortedResultMonitor& filterMonitor)
{
    ofstream out(fileName);
    out << left << setw(30) << "Name|" << setw(10) << "Age|" << setw(10) << "Income|" << '\n';
    for (int i = 0; i < filterMonitor.objectCount; i++)
    {
        Citizen citizen = filterMonitor[i];
        out << left << setw(30) << citizen.name << setw(10) << citizen.age << setw(10) << citizen.income << '\n';
    }
    out.close();
}

int main()
{
    clock_t tStart = clock();

    int n;
    Citizen* citizens = getCitizens(DATA_FILE_1, n);

    DataMonitor readMonitor(n / 2);
    SortedResultMonitor filterMonitor(n);

    thread threads[THREAD_COUNT];

    threads[0] = thread(writeData, ref(readMonitor), citizens, n);
    for (int i = 1; i < THREAD_COUNT; i++)
    {
        threads[i] = thread(filterData, ref(readMonitor), ref(filterMonitor));
    }

    mutex lock;
    unique_lock<mutex> guard(lock);
    

    cv.wait(guard, [&] {return filterMonitor.finished;});

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        threads[i].join();
    }

    printResults(RES_FILE, filterMonitor);
    delete[] citizens;

    printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
    return 0;
}
