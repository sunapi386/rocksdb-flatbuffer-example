#include <iostream>
#include <fstream>
#include "rocksdb/db.h"
#include "schemas/game_generated.h"

using namespace rocksdb;
using namespace std;

double flatBufferTest(int totalTests, DB *pDb)
{
    int testCount = 0;
    auto beginTime = std::chrono::system_clock::now();

    bool printBufSize = true;

    while (++testCount != totalTests)
    {
        // Create a flatbuffer object
        flatbuffers::FlatBufferBuilder builder(1024);
        string swordName = "Sword " + to_string(testCount);
        auto weapon_1_name = builder.CreateString(swordName);
        short weapon_1_dmg = 3;
        auto sword = game::CreateWeapon(builder, weapon_1_name, weapon_1_dmg);
        string monsterName = "monster " + to_string(testCount);
        auto name = builder.CreateString(monsterName);
        auto weapons = builder.CreateVector(vector<flatbuffers::Offset<game::Weapon>> {sword});
        game::MonsterBuilder monsterBuilder(builder);
        monsterBuilder.add_weapons(weapons);
        monsterBuilder.add_name(name);
        auto orc = monsterBuilder.Finish();
        builder.Finish(orc);

        // get its representation in byte buffer
        uint8_t *buf = builder.GetBufferPointer();
        int size = builder.GetSize();
        if (printBufSize)
        {
            cout << "Buffer size: " << size << "\n";
            printBufSize = false;
        }

        // put into rocksdb
        const Slice monster(reinterpret_cast<const char *>(buf), size);
        Status status = pDb->Put(WriteOptions(), "monster" + to_string(testCount), monster);
        assert(status.ok());

        // read from rocksdb
        string readBuffer;
        status = pDb->Get(ReadOptions(), "monster" + to_string(testCount), &readBuffer);
        assert(status.ok());

        // construct monster from read buffer readBuffer
        auto retMonster = game::GetMonster(readBuffer.c_str());

        // do sanity check
        const char *retMonsterName = retMonster->name()->c_str();
        auto retWeapon = retMonster->weapons()->Get(0);
        const char *retWeaponName = retWeapon->name()->c_str();
        assert(strcmp(monsterName.c_str(), retMonsterName) == 0);
        assert(strcmp(swordName.c_str(), retWeaponName) == 0);
    }

    auto endTime = std::chrono::system_clock::now();
    auto diff = chrono::duration_cast<chrono::microseconds>(endTime - beginTime).count();
    double testDurationMs = diff / 1000.0;
    return testDurationMs;
}

double filesystemTest(int totalTests)
{
    int testCount = 0;
    auto beginTime = std::chrono::system_clock::now();

    ofstream outfile("/tmp/test-" + to_string(totalTests) +".bin", ofstream::binary);
    ifstream infile("/tmp/test-" + to_string(totalTests) +".bin", ifstream::binary);


    vector<size_t> offsets;
    offsets.reserve(totalTests);


    bool printBufSize = true;

    while (testCount++ != totalTests)
    {
        // Create a flatbuffer object
        flatbuffers::FlatBufferBuilder builder(1024);
        string swordName = "Sword " + to_string(testCount);
        auto weapon_1_name = builder.CreateString(swordName);
        short weapon_1_dmg = 3;
        auto sword = game::CreateWeapon(builder, weapon_1_name, weapon_1_dmg);
        string monsterName = "monster " + to_string(testCount);
        auto name = builder.CreateString(monsterName);
        auto weapons = builder.CreateVector(vector<flatbuffers::Offset<game::Weapon>> {sword});
        game::MonsterBuilder monsterBuilder(builder);
        monsterBuilder.add_weapons(weapons);
        monsterBuilder.add_name(name);
        auto orc = monsterBuilder.Finish();
        builder.Finish(orc);

        // get its representation in byte buffer
        uint8_t *buf = builder.GetBufferPointer();
        int size = builder.GetSize();
        if (printBufSize)
        {
            cout << "Buffer size: " << size << "\n";
            printBufSize = false;
        }

        // append into file
        offsets[testCount] = size;
        outfile.write(reinterpret_cast<const char *>(buf), size);
        outfile.flush();
        assert(outfile.good());

        // read from file
        infile.sync();
        infile.seekg(0,infile.end); // seek to file end
        long end = infile.tellg();
        infile.seekg(end - size); // pointer is then moved back `size` characters
        char readBuffer[size];
        memset(readBuffer, 0, size*sizeof(char));
        infile.read(readBuffer, size); // read into readBuffer
        assert(!infile.eof()); // we should have read to the end of the file but not past it
        infile.clear();

        // construct monster from read buffer readBuffer
        auto retMonster = game::GetMonster(readBuffer);

        // do sanity check
        const char *retMonsterName = retMonster->name()->c_str();
        auto retWeapon = retMonster->weapons()->Get(0);
        const char *retWeaponName = retWeapon->name()->c_str();
        assert(strcmp(monsterName.c_str(), retMonsterName) == 0);
        assert(strcmp(swordName.c_str(), retWeaponName) == 0);
    }

    auto endTime = std::chrono::system_clock::now();
    auto diff = chrono::duration_cast<chrono::microseconds>(endTime - beginTime).count();
    double testDurationMs = diff / 1000.0;

    outfile.close();

    return testDurationMs;
}


int main(int argc, char *argv[])
{
    DB *db;

    int totalTests = 1'000'000;
    if (argc == 2)
    {
        totalTests = stoi(argv[1]);
    }
    cout << "Total read-writes: " << totalTests << "\n";

    Options options;
    options.create_if_missing = true;
    Status status = DB::Open(options,"/tmp/test-" + to_string(totalTests) +".db", &db);
    assert(status.ok());

    double testFlatBufferDurationMs = flatBufferTest(totalTests, db);
    cout << "RocksDb put/get " << totalTests << "x: " << testFlatBufferDurationMs << " ms\n";

    double testFilesystemDurationMs = filesystemTest(totalTests);
    cout << "Filesystem put/get " << totalTests << "x: " << testFilesystemDurationMs << " ms\n";

    delete db;
    return 0;
}
