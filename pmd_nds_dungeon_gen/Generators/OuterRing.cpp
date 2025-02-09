#include <Generators/AllTheGenerators.hpp>
#include <Generators/Generator.hpp>

OuterRing::OuterRing(std::shared_ptr<PRNG> rng, status_vars var, floor_properties floor_props) : Generator(rng, var, floor_props) {};

floor_t OuterRing::Generate() { 
    floor_t dummy;
    return dummy;
}