#include <Generators/AllTheGenerators.hpp>
#include <Generators/Generator.hpp>


Crossroads::Crossroads(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props) : Generator(rng, var, floor_props) {};

floor_t Crossroads::Generate() { 
    floor_t dummy;
    return dummy;
}