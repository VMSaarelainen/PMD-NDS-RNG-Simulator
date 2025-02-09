#include <Generators/Generator.hpp>
#include <PRNG/PRNG.hpp>
#include <Generators/AllTheGenerators.hpp>

#include <main.hpp> //for debug
#include <iostream>

Standard::Standard(std::shared_ptr<PRNG> rng, status_vars var, floor_properties floor_props) : Generator(rng, var, floor_props) {};

floor_t Standard::Generate() {
	int cursor_x;
	int cursor_y;


	GetGridPositions(grid.x, grid.y);	//todo: move to parent function?
	InitDungeonGrid();
	AssignRooms(floor_props.room_density, (floor_props.room_density > 0) ? false : true);
	CreateRoomsAndAnchors();
	
	cursor_x = rng->RandInt(grid.x);
	cursor_y = rng->RandInt(grid.y);

	AssignGridCellConnections(cursor_x, cursor_y);
	CreateGridCellConnections(false);
	EnsureConnectedGrid();
	// if (floor_props.maze_room_chance != 0) {	//todo: implement maze rooms
	// 	GenerateMazeRoom(floor_props.maze_room_chance);
	// }
	if (int spawnChance = status.kecleon_shop_spawn_chance != 0 && !status.has_monster_house && status.floor_type != floor_type::RESCUE) {
		status.has_kecleon_shop = GenerateKecleonShop(spawnChance);
	}

	if (int spawnChance = status.monster_house_spawn_chance != 0) {
		status.has_monster_house = GenerateMonsterHouse(spawnChance);
	}

	if (floor_props.extra_hallways != 0) {
		GenerateExtraHallways(floor_props.extra_hallways);
	}

	GenerateRoomImperfections();
	GenerateSecondaryStructures();
	return floor;
};