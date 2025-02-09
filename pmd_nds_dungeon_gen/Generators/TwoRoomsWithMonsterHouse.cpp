#include <Generators/AllTheGenerators.hpp>
#include <Generators/Generator.hpp>

TwoRoomsWithMonsterHouse::TwoRoomsWithMonsterHouse(std::shared_ptr<PRNG> rng, status_vars var, floor_properties floor_props) : Generator(rng, var, floor_props) {};

floor_t TwoRoomsWithMonsterHouse::Generate() { 
    floor_t dummy;
    return dummy;
}