#pragma once
#include <main.hpp>
#include <Generators/Generator.hpp>


class Beetle: public Generator {
    public:
        Beetle(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props);
        floor_t Generate() override;
};

class Cross: public Generator {
    public:
        Cross(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props);
        floor_t Generate() override;
};
class Crossroads: public Generator {
    public:
        Crossroads(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props);
        floor_t Generate() override;
};
class Line: public Generator {
    public:
        Line(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props);
        floor_t Generate() override;
};
// class Fixed: public Generator {
//     public:
//         Fixed(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props);
//         floor_t Generate() override;
// };
class OneRoomMonsterHouse: public Generator {
    public:
        OneRoomMonsterHouse(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props);
        floor_t Generate() override;
};

class OuterRing: public Generator {
    public:
        OuterRing(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props);
        floor_t Generate() override;
};

class OuterRooms: public Generator {
    public:
        OuterRooms(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props);
        floor_t Generate() override;
};

class Standard: public Generator {
    public:
        Standard(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props);
        floor_t Generate() override;
};
class TwoRoomsWithMonsterHouse: public Generator {
    public:
        TwoRoomsWithMonsterHouse(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props);
        floor_t Generate() override;
};
