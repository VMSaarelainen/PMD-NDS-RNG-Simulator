#pragma once
#include <optional>
#include <functional>
#include <memory>

#include <main.hpp>
#include <PRNG/PRNG.hpp>

/* Generator internals */

// Some parameters that need to be tracked before Generator instantiation
struct early_status_variables {
    int num_generation_attemps = 0;
    int grid_size_x = 2;
    int grid_size_y = 2;
    bool generate_secondary_terrain = false;
    ::floor_size floor_size = ::floor_size::LARGE;
};

struct gridcell_t {
	int start_x = 0;
	int start_y = 0;
	int end_x = 0;
	int end_y = 0;
	bool is_invalid = false;
	bool is_room = true;
	bool is_junction = false;
	bool is_connected = false;
	bool is_kecleon_shop = false;
	bool is_monster_house = false;
	bool has_secondary_structure = false;
	int field_0xf = 0;
	bool is_maze_room = false;
	bool was_merged_into_other_room = false;
	bool is_merged_room = false;
	bool is_connected_to_top = false;
	bool is_connected_to_bottom = false;
	bool is_connected_to_left = false;
	bool is_connected_to_right = false;
	bool should_connect_to_top = false;
	bool should_connect_to_bottom = false;
	bool should_connect_to_left = false;
	bool should_connect_to_right = false;
	int field_0x1b;
	bool flag_imperfect = false;
	bool flag_secondary_structure = false;

	bool is_connected_room() {
		return !is_invalid &&
			   is_connected &&
			   is_room;
	}

	bool is_not_merged() {
		return !was_merged_into_other_room &&
			   !is_merged_room;
	}

	bool can_spawn_kek_shop() {
		return !is_invalid &&
			   !was_merged_into_other_room &&
			   !is_merged_room &&
			   is_connected &&
			   is_room &&
			   !has_secondary_structure &&
			   !is_maze_room &&
			   !flag_secondary_structure;
	}

	bool can_spawn_mon_house() {
		return !is_invalid && 
			   !was_merged_into_other_room && 
			   is_connected && 
			   is_room && 
			   !is_kecleon_shop &&
			   !field_0xf &&
			   !is_maze_room &&
			   !has_secondary_structure;
	}

	bool is_connected_exactly_once() {
		int count = 0;
		if (is_connected_to_top) { count++; }
		if (is_connected_to_bottom) { count++; }
		if (is_connected_to_left) { count++; }
		if (is_connected_to_right) { count++; }
		if (count == 1) {
			return true;
		}
		return false;
	}
};

struct floor_generation_status {
	::floor_type floor_type = ::floor_type::NORMAL;		//TODO: namespace this entire project so this would be less ugly
	bool second_spawn;
	bool has_monster_house = false;
	int stairs_room;
	bool has_chasms_as_secondary_terrain = false;
	bool is_invalid = false;
	::floor_size floor_size;
	bool has_maze = false;
	bool no_enemy_spawns = false;
	int kecleon_shop_spawn_chance;
	int monster_house_spawn_chance;
	bool has_kecleon_shop = false;
	int n_rooms;
	int secondary_structures_budget;

	int num_generation_attemps = 0;
	bool generate_secondary_terrain = false;
	bool failedToGenerateFloorFlag = false;

	//TODO: missions substruct here

	bool can_spawn_mon_house() {
		if (/* (!missions.isOutlawMonsterHouseFloor() || !missions.isDestinationFloorWithMonster()) && */ floor_type == floor_type::NORMAL) {
			return true;
		}
		return false;
	}
};


class Generator {
    public:
        Generator(std::shared_ptr<PRNG> prng, early_status_variables var, floor_properties floor_props);
        virtual floor_t Generate() = 0;
        virtual ~Generator() = default;
    protected:
        struct gridCoords_t {
            std::array<int, 15> x;
            std::array<int, 15> y;
        };

        struct grid_t {
            std::array<std::array<gridcell_t, 15>, 15> cells;
            int x = 0;	//the game often doesnt use the entire 15*15 array, so these are the actual highest indices in the arrays
            int y = 0;	//these are used all the time, hence short names
            grid_t(int x, int y) {
                this->x = x;
                this->y = y;
            }
            grid_t() = default;
        };

        floor_t floor;
        std::shared_ptr<PRNG> rng;  //rng is also used outside this class, and the internal rng state needs to carry through potentially several Generator objects
        floor_generation_status status;
        floor_properties floor_props;
        grid_t grid;
        gridCoords_t gridCoords;
        tile default_tile;
        int generationAttempts = 0;

        tile& GetTile(const int x, const int y);
        void GetGridPositions(int sizeX, int sizeY);
        void InitDungeonGrid();
        void AssignRooms(int numRooms, std::optional<bool> exactNumRooms);
        void CreateRoomsAndAnchors();
        void AssignGridCellConnections(int cursorX, int cursorY);
        void CreateGridCellConnections(bool noMerging);
        void EnsureConnectedGrid();
        void GenerateMazeRooms(int spawnChance);
        bool GenerateKecleonShop(int spawnChance);
        bool GenerateMonsterHouse(int spawnChance);
        void GenerateExtraHallways(int numHallways);
        void GenerateRoomImperfections();
        void GenerateSecondaryStructures();
        void GenerateMaze(gridcell_t& cell, bool use_secondary_terrain);
        void GenerateMazeLine(int x0, int y0, int xmin, int ymin, int xmax, int ymax, bool use_secondary_terrain, int roomID);
        bool IsNextToHallway(int x, int y);
        void SetTerrainObstacleChecked(tile& tile, bool use_secondary_terrain, int roomID);
        void markTiles(const int minX, const int minY, const int maxX, const int maxY, std::function<void(tile&)> applyMark);
        void TryMergeRooms(gridcell_t& origin, gridcell_t& target);
        void resetOuterEdgeTiles();
        void ResetInnerBoundaryTileRows();
        void EnsureImpassableTilesAreWalls();
        void CreateHallway(int x0, int y0, int x1, int y1, bool vertical, int x_mid, int y_mid);
        void Finalize();
        void FinalizeJunctions();
};