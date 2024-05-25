#pragma once
#include <array>
#include <vector>
#include <ostream>

#include <pmd2_dungeon_data_scraper/floor_props.hpp>

constexpr int floor_x = 56;
constexpr int floor_y = 32;

enum class floor_type {
	NORMAL = 0,
	FIXED = 1,
	RESCUE = 2
};

enum class terrain {
    WALL = 0,
    NORMAL = 1,
    SECONDARY = 2, // Water or lava
    CHASM = 3
};

enum class spawn_type {
	NONE,
	PLAYER,
	PARTNER,
	ENEMY
};

enum class floor_size {
	LARGE = 0,
	SMALL = 1,
	MEDIUM = 2,
};

/* Tile and floor definitions */
struct tile {
	::terrain terrain = ::terrain::WALL;
	int roomID = 0xFF;
	bool is_junction;
	bool isMonsterHouse = false;
	bool isKecleonShop = false;
	bool isKeyDoor = false;
	bool isUnreachableFromStairs = false;	//todo: default value?
	bool hasItem = false;
	bool hasStairs = false;
	bool hasTrap = false;
	spawn_type entity = spawn_type::NONE;
	bool is_dummy = false;	//extra debug info
	
	friend std::ostream& operator<<(std::ostream &stm, const tile& t) {
		if (t.hasStairs) {
			stm << "S";
		}
		else if (t.isKecleonShop) {
			stm << "K";
		}
		else if (t.isMonsterHouse) {
			stm << "M";
		}
		else if (t.isKeyDoor) {
			stm << "D";
		}
		else if (t.hasItem) {
			stm << "o";
		}
		else if (t.hasTrap) {
			stm << "X";
		}
		else if (t.is_junction) {
			stm << "+";
		}
		else if (t.terrain == terrain::SECONDARY) {
			stm << "W";
		}
		else if (t.terrain == terrain::CHASM) {
			stm << "C";
		}
		else if (t.terrain == terrain::WALL) {
			stm << "*";
		}
		else if (t.terrain == terrain::NORMAL) {
			stm << " ";
		}
		else {
			stm << t.roomID;
		}
		return stm;
	};
};

typedef std::array<std::array<tile, floor_y>, floor_x> floor_t;

/* Handy functions if you are interfacing directly with this project */
floor_t generateFloor(floor_properties floor_props, uint32_t seed);
floor_properties get_floor_props(std::string dungeon_name, int floor_num);
std::string get_floor_ascii(const floor_t& floor);
