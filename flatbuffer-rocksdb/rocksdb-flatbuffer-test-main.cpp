#include <iostream>
#include "rocksdb/db.h"
#include "schemas/game_generated.h"

using namespace rocksdb;
using namespace std;
DB *db;

int main()
{
    Options options;
    options.create_if_missing = true;
    Status status = DB::Open(options,"/tmp/test-rocksdb.db", &db);
    assert(status.ok());

    int testCount = 0;
    constexpr int totalTests = 1000;
    auto beginTime = std::chrono::system_clock::now();

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

        // put into rocksdb
        const Slice swordSlice(reinterpret_cast<const char *>(buf), size);
        status = db->Put(WriteOptions(), "sword", swordSlice);
        assert(status.ok());

        // read from rocksdb
        string readBuffer;
        status = db->Get(ReadOptions(), "sword", &readBuffer);
        assert(status.ok());

        // construct monster from read buffer readBuffer
        auto retMonster = game::GetMonster(readBuffer.c_str());

        // do sanity check
        const char *retMonsterName = retMonster->name()->c_str();
        auto retWeapon = retMonster->weapons()->Get(0);
        const char *retWeaponName = retWeapon->name()->c_str();
        assert(strcmp(monsterName, retMonsterName) == 0);
        assert(strcmp(swordName.c_str(), retWeaponName) == 0);
    }

    auto endTime = std::chrono::system_clock::now();
    auto diff = chrono::duration_cast<std::chrono::microseconds> (endTime - beginTime).count();
    double ms = diff / 1000.0;
    cout << "put/get " << totalTests << "x: " << ms << " ms\n";

    delete db;
    return 0;
}