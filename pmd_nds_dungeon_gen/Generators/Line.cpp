#include <Generators/AllTheGenerators.hpp>
#include <Generators/Generator.hpp>

Line::Line(std::shared_ptr<PRNG> rng, early_status_variables var, floor_properties floor_props) : Generator(rng, var, floor_props) {};

floor_t Line::Generate() {
    floor_t dummy;
    return dummy;
}