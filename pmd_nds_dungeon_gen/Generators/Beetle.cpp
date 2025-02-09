#include <Generators/AllTheGenerators.hpp>
#include <Generators/Generator.hpp>

Beetle::Beetle(std::shared_ptr<PRNG> rng, status_vars var, floor_properties floor_props) : Generator::Generator(rng, var, floor_props) {}

floor_t Beetle::Generate() { 
    floor_t dummy;
    return dummy;
}