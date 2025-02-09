#include <sstream>
#include <stdint.h>
#include <string>
#include <optional>
#include <algorithm>
#include <functional>
#include <ranges>
#include <iostream>
#include <memory>
#include <fstream>
#include <exception>

#include <main.hpp>
#include <PRNG/PRNG.hpp>
#include <Generators/AllTheGenerators.hpp>
#include <pmd2_dungeon_data_scraper/floor_props.hpp>
#include <tclap/CmdLine.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

thread_local tile default_tile = tile();		//owns the memory for a default-initialized tile. GetTile can return a (temporary) non-const ref to this tile.

int main(int argc, char *argv[]) {
	try {
		/* Define and parse args */
		TCLAP::CmdLine cmd("", ' ', "1.1");
		TCLAP::ValueArg<std::string> dun_name_arg("d", "dungeon", "Name of a dungeon or entry in the JSON file", true, "Beach Cave", "Name of a dungeon", cmd);
		TCLAP::ValueArg<int> floor_num_arg("f", "floor", "A floor number, without affixes (i.e. not \"9F\" or \"B9F\")", true, 1, "Floor number", cmd);
		TCLAP::ValueArg<int> rng_seed_arg("s", "seed", "A seed number in decimal in the range [0, UINT32_MAX].", false, 0, "RNG seed in decimal", cmd);
		TCLAP::ValueArg<std::string> out_file_arg("o", "out", "Write the generated floor to this file if specified (will overwrite)", false, "", "Output file", cmd);
		cmd.parse(argc, argv);

		/* Generate a floor */
		floor_properties props = get_floor_props(dun_name_arg.getValue(), floor_num_arg.getValue());
		std::cout << "Generating " << dun_name_arg.getValue() << " Floor " << floor_num_arg.getValue() << " with Seed: " << rng_seed_arg.getValue() << "\n";
		floor_t floor = generateFloor(props, rng_seed_arg);

		/* Write output */
		std::string out_str = get_floor_ascii(floor);
		if (out_file_arg.isSet() && out_file_arg.getValue() != "") {	//write to file
			try {
				std::ofstream out;
				out.open(out_file_arg.getValue());
				out << out_str;
				out.close();
				std::cout << "Wrote to file: " << out_file_arg.getValue() << "\n";
			}
			catch(const std::exception& e) {
				std::cerr << e.what() << '\n';
				return EXIT_FAILURE;
			}
		}
		else {	//no output file, write to stdout
			std::cout << out_str << "\n";
		}
	}
	catch (TCLAP::ArgException &e)  { 
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		return EXIT_FAILURE;
	}
	
    return EXIT_SUCCESS;
}

/* Read JSON file and find a floor_properties entry. If not found, throws std::domain_error */
floor_properties get_floor_props(std::string dungeon_name, int floor_num) {
	std::ifstream f("PMD_Sky_dungeon_data.json");
	const json json_props = json::parse(f);

	for (const auto& dungeon : json_props) {
		if (dungeon["name"]["en"] == dungeon_name) {
				try {
					json props = dungeon["floors"].at(std::to_string(floor_num));
					floor_properties floor_props = {
						.layout = props["layout"],
						.room_density = props["room_density"],
						.tileset = props["tileset"],
						.music_table_idx = props["music_table_idx"],
						.weather = props["weather"],
						.floor_connectivity = props["floor_connectivity"],
						.enemy_density = props["enemy_density"],
						.kecleon_shop_spawn_chance = props["kecleon_shop_spawn_chance"],
						.monster_house_spawn_chance = props["monster_house_spawn_chance"],
						.maze_room_chance = props["maze_room_chance"],
						.sticky_item_chance = props["sticky_item_chance"],
						.allow_dead_ends = props["allow_dead_ends"],
						.max_secondary_structures = props["max_secondary_structures"],
						.f_secondary_structures = props["f_secondary_structures"],
						.room_flags_unk1 = 0,	//these are unused TODO: remove from scraper repo
						.f_room_imperfections = props["f_room_imperfections"],
						.room_flags_unk3 = 0,
						.field_0xe = 0,
						.item_density = props["item_density"],
						.trap_density = props["trap_density"],
						.floor_number = props["floor_number"],
						.fixed_room_id = props["fixed_room_id"],
						.extra_hallways = props["extra_hallways"],
						.buried_item_density = props["buried_item_density"],
						.secondary_terrain_density = props["secondary_terrain_density"],
						.visibility_range = props["visibility_range"],
						.max_money_amount_div_5 = props["max_money_amount_div_5"],
						.shop_item_positions = props["shop_item_positions"],
						.itemless_monster_house_chance = props["itemless_monster_house_chance"],
						.hidden_stairs_type = props["hidden_stairs_type"],
						.hidden_stairs_spawn_chance = props["hidden_stairs_spawn_chance"],
						.enemy_iq = props["enemy_iq"],
						.iq_booster_value = props["iq_booster_value"]
						.has_extra_item = props.at("has_extra_item")
					};

					return floor_props;
				}
				catch(const json::exception& e) {
					if (e.id == 403) {
						std::cout << dungeon_name << " has no floor entry " << floor_num << " in JSON file.\n"; 
					}
					else {
						std::cerr << e.what() << '\n';
					}
				}
		}
	}
	std::stringstream err_str;
	err_str << "Dungeon \"" << dungeon_name << "\" not found in JSON file. Check the spelling and make sure you're using quote marks (e.g. \"Beach Cave\").\n";
	throw std::domain_error(err_str.str());
}

/* Entry point for dungeon generation, this is where the reverse engineered parts start.*/
status_vars generateFloor(floor_properties floor_props, uint32_t seed) {
	std::shared_ptr<PRNG> rng = std::make_shared<PRNG>(PRNG(seed, true));
	status_vars var;
	floor_t& floor = var.floor;	//convenience
	bool generate_secondary_terrain = false;
	
	for (int gen_attempt_outer = 0; gen_attempt_outer < 10; gen_attempt_outer++) {
		var.num_generation_attemps = gen_attempt_outer;
		for (int n = 0; n < 10; n++) {

			if (floor_props.fixed_room_id == 0) {   //not a fixed layout

				/* Pick number of room rows/cols */
				bool fail_flag = true;
				for (int i = 0; i < 32; i++) {
					if (floor_props.layout == floor_layout::LARGE_0x8) {
						var.grid_size_x = rng->RandRange(2, 5);
						var.grid_size_y = rng->RandRange(2, 4);
					}
					else {
						var.grid_size_x = rng->RandRange(2, 9);
						var.grid_size_y = rng->RandRange(2, 8);
					}
					if (var.grid_size_x <= 6 && var.grid_size_y <= 4) {
						fail_flag = false;
						break;
					}
				}
				if (fail_flag) {
					var.grid_size_x = 4;
					var.grid_size_y = 4;
				}

				if (FLOOR_X / var.grid_size_x < 8) {
					var.grid_size_x = 1;
				}
				if (FLOOR_Y / var.grid_size_y < 8) {
					//Note: skipped some changes to the dungeon struct here, looks like it just makes sure force_monster_house is false
					var.grid_size_y = 1;
				}

				switch(floor_props.layout) {
					default:
					case floor_layout::LARGE:
					case floor_layout::LARGE_0x8:
					{
						auto gen = Standard(rng, var, floor_props);
						floor = gen.Generate();
						generate_secondary_terrain = true;
						break;
					}
					case floor_layout::SMALL:
					{
						var.floor_size = floor_size::SMALL;
						var.grid_size_x = 4;
						var.grid_size_y = rng->RandInt(2) + 2;
						auto gen = Standard(rng, var, floor_props);
						floor = gen.Generate();
						generate_secondary_terrain = true;
						break;

					}
					case floor_layout::MEDIUM:
					{
						var.floor_size = floor_size::MEDIUM;
						var.grid_size_x = 4;
						var.grid_size_y = rng->RandInt(2) + 2;
						auto gen = Standard(rng, var, floor_props);
						floor = gen.Generate();
						generate_secondary_terrain = true;
						break;
					}
					case floor_layout::ONE_ROOM_MONSTER_HOUSE:
					{
						auto gen = OneRoomMonsterHouse(rng, var, floor_props);
						floor = gen.Generate();
						// force_monster_house = true;
						break;
					}
					case floor_layout::OUTER_RING:
					{
						auto gen = OuterRing(rng, var, floor_props);
						floor = gen.Generate();
						generate_secondary_terrain = true;
						break;
					}
					case floor_layout::CROSSROADS:
					{
						auto gen = Crossroads(rng, var, floor_props);
						floor = gen.Generate();
						generate_secondary_terrain = true;
						break;
					}
					case floor_layout::TWO_ROOMS_WITH_MONSTER_HOUSE:
					{
						auto gen = TwoRoomsWithMonsterHouse(rng, var, floor_props);
						floor = gen.Generate();
						// force_monster_house = true;
						break;
					}
					case floor_layout::LINE:
					{
						auto gen = Line(rng, var, floor_props);
						floor = gen.Generate();
						generate_secondary_terrain = true;
						break;
					}
					case floor_layout::CROSS:
					{
						auto gen = Cross(rng, var, floor_props);
						floor = gen.Generate();
						break;
					}
					case floor_layout::BEETLE:
					{
						auto gen = Beetle(rng, var, floor_props);
						floor = gen.Generate();
						break;
					}
					case floor_layout::OUTER_ROOMS:
					{
						auto gen = OuterRooms(rng, var, floor_props);
						floor = gen.Generate();
						generate_secondary_terrain = true;
						break;
					}
				}

				FinalizeJunctions(floor);
				if (generate_secondary_terrain) {
					GenerateSecondaryTerrainFormations(true, floor, floor_props, rng);	//not to be confused with "secondary structures"
				}

				[[maybe_unused]]bool itemless_monster_house = (rng->Rand100() < floor_props.itemless_monster_house_chance) ? true : false;
				MarkNonEnemySpawns(floor, floor_props, itemless_monster_house, rng);	//this includes stairs
				
				//MarkEnemySpawns(floor_props, itemless_monster_house);
				//ResolveInvalidSpawns();

				// if stairs always reachable...
				if (gen_attempt_outer >= 10) {	//Yes, this actually overwrites the floor that was generated on the 11th attempt no matter what. We can't change it without impacting the RNG though.
					std::cout << "Generation failed! Generating back-up One-Room Monster House instead...\n";
					auto gen = OneRoomMonsterHouse(rng, var, floor_props);
					var.floor = gen.Generate();
					return var;
				}
				return var;
			}

			else {
				std::cout << "Tried to generate fixed layout\n";
				floor_t dummy;
				var.floor = dummy;
				return var;
			}
		}
	}
	std::cout << "Generation failed! Generating back-up One-Room Monster House instead...\n";
	auto gen = OneRoomMonsterHouse(rng, var, floor_props);
	var.floor = gen.Generate();
	return var;
}

std::string get_floor_ascii(const floor_t& floor) {
	std::stringstream o;
    for (int j = 0; j < FLOOR_Y; j++) {
        for (int i = 0; i < FLOOR_X; i++) {
            o << floor[i][j] << " ";
        }
        o << "\n";
	}
	return o.str();
}

/* Gets the tile at (x, y), unless (x, y) is out of bounds in which case returns a default tile through the first parameter. */
tile& GetTile(floor_t& floor, const int x, const int y) {
	if (x >= 0 && x < FLOOR_X && y >= 0 && y < FLOOR_Y) {
		return floor[x][y];
	}
	default_tile = tile();
	return default_tile;
}