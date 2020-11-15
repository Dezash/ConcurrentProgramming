#include <iostream>
#include "nlohmann/json.hpp"
#include <fstream>
#include <iomanip>
#include "Citizen.h"
#include "CitizenComputed.h"
#include <openssl/sha.h>
#include <sstream>
#include <mpi.h>


using namespace std;
using json = nlohmann::json;

const string DATA_FILE = "large.json";
const string RES_FILE = "IFF81_LaurinaviciusG_L1_rez.txt";
const int THREAD_COUNT = 9;


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


static void printResults(const string fileName, CitizenComputed results[], int n, int total)
{
    ofstream out(fileName);
    out << left << setw(30) << "Name|" << setw(10) << "Age|" << setw(10) << "Income|" << setw(10) << "SHA256|" << '\n';
    for (int i = 0; i < n; i++)
    {
        CitizenComputed computed = results[i];
        Citizen citizen = computed.citizen;
        out << left << setw(30) << citizen.name << setw(10) << citizen.age << setw(10) << citizen.income << setw(10) << computed.hash << '\n';
    }

    out << "Is pradziu duomenu: " << total << "\nAtfiltruota duomenu: " << n;
    out.close();
}


void loadData(string fileName)
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

    int n = data.size();
    MPI::COMM_WORLD.Send(&n, 1, MPI::INT, 1, 3);
    MPI::COMM_WORLD.Send(&n, 1, MPI::INT, 2, 3);

    for (const auto& item : data.items())
    {
        json data = item.value();

        string strName = data["Name"];
        const char* name = strName.c_str();
        int age = data["Age"];
        double income = data["Income"];

        //cout << "loadData: Sending: " << strName << " " << age << " " << income << endl;
        MPI::COMM_WORLD.Send(name, static_cast<int>(strName.size()), MPI::CHAR, 1, 0);
        MPI::COMM_WORLD.Send(&age, 1, MPI::INT, 1, 0);
        MPI::COMM_WORLD.Send(&income, 1, MPI::DOUBLE, 1, 0);
    }

    //cout << "loadData: Sending end message\n";
    MPI::COMM_WORLD.Send(NULL, 0, MPI::BOOL, 1, 2);



    int count;
    MPI::COMM_WORLD.Recv(&count, 1, MPI::INT, MPI::ANY_SOURCE, 0);
    //cout << "loadData: count = " << count << endl;

    auto* citizens = new CitizenComputed[count];

    MPI::Status status;

    for (int i = 0; i < count; i++)
    {
        MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, 0, status);

        //cout << "loadData: buffer " << status.Get_count(MPI::CHAR) << endl;
        char buffer[254];
        MPI::COMM_WORLD.Recv(buffer, status.Get_count(MPI::CHAR), MPI::CHAR, MPI::ANY_SOURCE, 0);
        string name(&buffer[0], &buffer[status.Get_count(MPI::CHAR)]);
        int age;
        MPI::COMM_WORLD.Recv(&age, 1, MPI::INT, MPI::ANY_SOURCE, 0);
        double income;
        MPI::COMM_WORLD.Recv(&income, 1, MPI::DOUBLE, MPI::ANY_SOURCE, 0);

        MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, 0, status);
        //cout << "loadData: sha256Buffer " << status.Get_count(MPI::CHAR) << endl;
        char sha256Buffer[64];
        MPI::COMM_WORLD.Recv(sha256Buffer, status.Get_count(MPI::CHAR), MPI::CHAR, MPI::ANY_SOURCE, 0);
        string hash(&sha256Buffer[0], &sha256Buffer[status.Get_count(MPI::CHAR)]);

        auto citizen = Citizen(name, age, income);
        citizens[i] = CitizenComputed(citizen, hash);

        //cout << "loadData: " << name << " " << hash << endl;
    }

    printResults(RES_FILE, citizens, count, n);
    delete[] citizens;
}


void controlInput()
{
    //cout << "controlInput: Waiting for n\n";
    int n;
    MPI::COMM_WORLD.Recv(&n, 1, MPI::INT, 0, 3);
    //cout << "controlInput: n = " << n << endl;

    int count = 0;
    const int size = n / 2;
    auto* citizens = new Citizen[size];

    bool loadEnded = false;

    int tagAwaited = MPI::ANY_TAG;
    int sourceAwaited = MPI::ANY_SOURCE;
    while(true)
    {
        //cout << "controlInput: Probing...\n";
        MPI::Status status;
        MPI::COMM_WORLD.Probe(sourceAwaited, tagAwaited, status);

        int tag = status.Get_tag();
        int source = status.Get_source();
        //cout << "controlInput: Tag " << tag << endl;

        switch (tag)
        {
        case 0:
        {
            //cout << "controlInput: Probed, name length: " << status.Get_count(MPI::CHAR) << endl;
            if (count == size - 1)
            {
                //cout << "controlInput: Message rejected because array is full\n";
                tagAwaited = 1;
                sourceAwaited = MPI::ANY_SOURCE;
                break;
            }

           // cout << "controlInput: Waiting for message...\n";
            char buffer[254];
            MPI::COMM_WORLD.Recv(buffer, status.Get_count(MPI::CHAR), MPI::CHAR, 0, 0);
            string name(&buffer[0], &buffer[status.Get_count(MPI::CHAR)]);
            //cout << "controlInput: Name: " << name << endl;
            int age;
            MPI::COMM_WORLD.Recv(&age, 1, MPI::INT, 0, 0);
            double income;
            MPI::COMM_WORLD.Recv(&income, 1, MPI::DOUBLE, 0, 0);

            citizens[count++] = Citizen(name, age, income);
            //cout << "controlInput: count = " << count << endl;

            sourceAwaited = MPI::ANY_SOURCE;
            break;
        }
        case 1:
        {
            //cout << "controlInput: Remove\n";

            if (count <= 0)
            {
                //cout << "controlInput: No elements\n";
                if (loadEnded)
                {
                    //cout << "controlInput: Ending all\n";
                    int commSize = MPI::COMM_WORLD.Get_size();
                    

                    for (int i = 0; i < commSize - 3; i++)
                    {
                        int workerRank = i + 3;
                        MPI::COMM_WORLD.Send(NULL, 0, MPI::BOOL, workerRank, 2);
                    }

                    delete[] citizens;
                    return;
                }
                else
                {
                    sourceAwaited = 0;
                }
                break;
            }

            MPI::COMM_WORLD.Recv(NULL, 0, MPI::CHAR, source, tag);
            Citizen citizen = citizens[--count];
            //cout << "controlInput: Sending: " << citizen.name << " " << citizen.age << " " << citizen.income << endl;
            //cout << "controlInput: count = " << count << endl;
            const char* name = citizen.name.c_str();
            MPI::COMM_WORLD.Send(name, static_cast<int>(citizen.name.size()), MPI::CHAR, source, 0);
            MPI::COMM_WORLD.Send(&citizen.age, 1, MPI::INT, source, 0);
            MPI::COMM_WORLD.Send(&citizen.income, 1, MPI::DOUBLE, source, 0);

            tagAwaited = MPI::ANY_TAG;
            break;
        }

        case 2:
        {
            //cout << "controlInput end\n";
            MPI::COMM_WORLD.Recv(NULL, 0, MPI::BOOL, source, tag);
            loadEnded = true;
            break;
        }
        default:
            break;
        }


    }

    delete[] citizens;
}

void processData()
{
    //string names[25];
    //int total_names = 0;

    //cout << "processData: Started\n";
    while (true)
    {
        MPI::COMM_WORLD.Send(NULL, 0, MPI::CHAR, 1, 1);

        int awaitedTag = MPI::ANY_TAG;
        MPI::Status status;
        MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, awaitedTag, status);
        int tag = status.Get_tag();
        //cout << "processData: Tag: " << tag << endl;

        if (tag == 2)
        {
            //cout << "processData: End\n";
            return;
        }

        //cout << "processData: Waiting for message...\n";
        char buffer[254];
        MPI::COMM_WORLD.Recv(buffer, status.Get_count(MPI::CHAR), MPI::CHAR, 1, 0);
        string name(&buffer[0], &buffer[status.Get_count(MPI::CHAR)]);
        int age;
        MPI::COMM_WORLD.Recv(&age, 1, MPI::INT, 1, 0);
        double income;
        MPI::COMM_WORLD.Recv(&income, 1, MPI::DOUBLE, 1, 0);

        const char* nameOut = name.c_str();
        MPI::COMM_WORLD.Send(nameOut, static_cast<int>(name.size()), MPI::CHAR, 2, 0);
        MPI::COMM_WORLD.Send(&age, 1, MPI::INT, 2, 0);
        MPI::COMM_WORLD.Send(&income, 1, MPI::DOUBLE, 2, 0);

        string hash = sha256(name + to_string(age) + to_string(income));
        //cout << "processData: Sending " << hash << "\n";
        const char* hashOut = hash.c_str();
        MPI::COMM_WORLD.Send(hashOut, static_cast<int>(hash.size()), MPI::CHAR, 2, 0);
    }
}


int search(CitizenComputed objects[], int objectCount, int first, int last, Citizen x) {
    int mid;
    while (first < last)
    {
        mid = (first + last) / 2;
        if (objects[mid].citizen == x)
            return mid;
        else if (objects[mid].citizen > x)
            last = mid - 1;
        else
            first = mid + 1;
    }

    if (objects[first].citizen > x)
        return first;
    else if (first == objectCount)
        return objectCount;
    else
        return first + 1;
        
}

void insertSorted(CitizenComputed objects[], int &objectCount, CitizenComputed newObject)
{
    int index = search(objects, objectCount, 0, objectCount, newObject.citizen);

    for (int i = objectCount - 1; i >= index; i--)
        objects[i + 1] = objects[i];

    objects[index] = newObject;
    objectCount++;
}

void processResults()
{
    //cout << "processResults: Waiting for n\n";
    int n;
    MPI::COMM_WORLD.Recv(&n, 1, MPI::INT, 0, 3);
    //cout << "processResults: n = " << n << endl;

    int count = 0;
    auto* citizens = new CitizenComputed[n];

    for (int i = 0; i < n; i++)
    {
        //cout << "processResults iterration: " << i << endl;
        MPI::Status status;
        MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, 0, status);

        char buffer[254];
        MPI::COMM_WORLD.Recv(buffer, status.Get_count(MPI::CHAR), MPI::CHAR, MPI::ANY_SOURCE, 0);
        string name(&buffer[0], &buffer[status.Get_count(MPI::CHAR)]);
        int age;
        MPI::COMM_WORLD.Recv(&age, 1, MPI::INT, MPI::ANY_SOURCE, 0);
        double income;
        MPI::COMM_WORLD.Recv(&income, 1, MPI::DOUBLE, MPI::ANY_SOURCE, 0);

        MPI::COMM_WORLD.Probe(MPI::ANY_SOURCE, 0, status);
        char sha256Buffer[64];
        MPI::COMM_WORLD.Recv(sha256Buffer, status.Get_count(MPI::CHAR), MPI::CHAR, MPI::ANY_SOURCE, 0);
        string hash(&sha256Buffer[0], &sha256Buffer[status.Get_count(MPI::CHAR)]);

        auto citizen = Citizen(name, age, income);
        auto citzenComputed = CitizenComputed(citizen, hash);

        if (citizen.income >= 40500 && citizen.income < 118000)
            insertSorted(citizens, count, citzenComputed);
    }


    MPI::COMM_WORLD.Send(&count, 1, MPI::INT, 0, 0);
    for (int i = 0; i < count; i++)
    {
        auto citizen = citizens[i];
        //cout << "processResults: Sending " << citizen.citizen.name << " " << citizen.hash << endl;

        const char* nameOut = citizen.citizen.name.c_str();
        MPI::COMM_WORLD.Send(nameOut, static_cast<int>(citizen.citizen.name.size()), MPI::CHAR, 0, 0);
        MPI::COMM_WORLD.Send(&citizen.citizen.age, 1, MPI::INT, 0, 0);
        MPI::COMM_WORLD.Send(&citizen.citizen.income, 1, MPI::DOUBLE, 0, 0);

        const char* hashOut = citizen.hash.c_str();
        MPI::COMM_WORLD.Send(hashOut, static_cast<int>(citizen.hash.size()), MPI::CHAR, 0, 0);
    }
}


int main(int argc, char **argv)
{
    clock_t tStart = clock();

    //DataMonitor readMonitor(n / 2);
    //SortedResultMonitor filterMonitor(n);

    MPI::Init();
    auto rank = MPI::COMM_WORLD.Get_rank();
    auto totalProcesses = MPI::COMM_WORLD.Get_size();
    cout << "thread " << rank << "/" << totalProcesses << " started.\n";

    if (totalProcesses < 2)
    {
        cerr << "Reikia bent 2 threadu\n";
        MPI::COMM_WORLD.Abort(1);
    }

    switch (rank)
    {
    case 0:
        // Main thread
        loadData(DATA_FILE);
        break;
    case 1:
        // Data thread
        controlInput();
        break;
    case 2:
        // Result thread
        processResults();
        break;
    default:
        // Worker thread
        processData();
        break;
    }


    MPI::Finalize();

    printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
    return 0;
}
