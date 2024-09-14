#include <optional>
#include <algorithm>
#include <cstdint>
#include <ranges>
#include <memory>

#include <PRNG/PRNG.hpp>
#include <Generators/Generator.hpp>
#include <pmd2_dungeon_data_scraper/floor_props.hpp>

Generator::Generator(std::shared_ptr<PRNG> prng, early_status_variables var, floor_properties props) {
	rng = prng;
	grid.x = var.grid_size_x;
    grid.y = var.grid_size_y;
	status.floor_size = var.floor_size;
	status.generate_secondary_terrain = var.generate_secondary_terrain;
	status.num_generation_attemps = var.num_generation_attemps;
	floor_props = props;
	default_tile = tile();
}

/* Gets the tile at (x, y), unless (x, y) is out of bounds in which case returns a default tile. */
tile& Generator::GetTile(const int x, const int y) {
	if (x >= 0 && x < floor_x && y >= 0 && y < floor_y) {
		return floor[x][y];
	}
	default_tile = tile();
	return default_tile;
	
}

void Generator::GetGridPositions(int size_x, int size_y) {
	gridCoords_t gridCoords;

	int c = 0;
	for (int i = 0; i <= size_x; i++) {
		gridCoords.x[i] = c;
		c += 56 / size_x;
	}

	c = 0;
	for (int i = 0; i <= size_y; i++) {
		gridCoords.y[i] = c;
		c += 32 / size_y;
	}

	this->gridCoords = gridCoords;
}

void Generator::InitDungeonGrid() {
	for (int i = 0; i < grid.x; i++) {
		for (int j = 0; j < grid.y; j++) {
			if ((status.floor_size == floor_size::SMALL) && (grid.x /2 <= i)) {
				grid.cells[i][j].is_invalid = true;
			}
			else if (status.floor_size == floor_size::MEDIUM && grid.x * 3 + ((grid.x * 3 / 2 / 4) <= i)) {	//not sure if this last bit is correct, but pmd2 doesn't seem to use the medium floor size anyway
				grid.cells[i][j].is_invalid = true; 
			}
			else {
				grid.cells[i][j].is_invalid = false;
			}
		//the game would intialize a bunch of other fields in the grid cells here, but theyre already defaulted correctly 
		}
	}
}

void Generator::AssignRooms(int num_rooms, std::optional<bool> exact_num_rooms) {
	int variance = rng->RandInt(3);   //The game gets a random number before checking if it needs it, so we need to as well to stay on the same RNG
	if (exact_num_rooms.value_or(false)) {
		num_rooms += variance;
	}

	std::array<bool, 256> flags {false};
	for (int i = 0; i < num_rooms; i++) {
		flags[i] = true;
	}

	for (int i = 0; i < 64; i++) {
		int a = rng->RandInt(grid.x * grid.y);
		int b = rng->RandInt(grid.x * grid.y);
		std::swap(flags[a], flags[b]);
	}

	int k = 0;
	int room_count = 0;
	for (int i = 0; i < grid.x; i++) {
		for (int j = 0; j < grid.y; j++) {
			if (!grid.cells[i][j].is_invalid) {
				if (room_count > 31) {
					grid.cells[i][j].is_room = false;
				}
				if (flags[k] == false) {
					grid.cells[i][j].is_room = false;
				}
				else {
					grid.cells[i][j].is_room = true;
					room_count += 1;
					if ((grid.x % 2 != 0) && (i == (grid.x -1) / 2 && j == 1)) {
						grid.cells[i][j].is_room = false;
					}
				}
				k += 1;
			}
		}
	}

	if (room_count < 2) {
		for (int i = 0; i < 200; i++) {
			for (int j = 0; j < grid.x; j++) {
				for (int k = 0; k < grid.y; k++) {
					if (!grid.cells[j][k].is_invalid && rng->RandInt(100) < 60) {
						grid.cells[j][k].is_room = true;
						return;
					}
				}
			}
		}
	}
	status.n_rooms = room_count;
}

void Generator::CreateRoomsAndAnchors() {
	int room_num = 0;
	for (int j = 0; j < grid.y; j++) {
		for (int i = 0; i < grid.x; i++) {   //note i and j reversed from standard for some reason
			/* A lot of one-letter variables ahead, most of it is just some maths to make things appear more random */
			int a = gridCoords.x[i] + 2;
			int b = gridCoords.y[j] + 2;
			int c = (gridCoords.x[i + 1] - gridCoords.x[i]) -4;
			int d = (gridCoords.y[j + 1] - gridCoords.y[j]) -3;

			if (!grid.cells[i][j].is_invalid) {
				if (!grid.cells[i][j].is_room) {
					int e = 2;
					int f = 2;
					int g = 4;
					int h = 4;

					if (i == 0) {
						f = 1;
					}
					if (j == 0) {
						e = 1;
					}
					if (i == grid.x -1) {
						g = 2;
					}
					if (j == grid.y -1) {
						h = 2;
					}

					int rand_x = rng->RandRange(a + f,(a + c) - g);
					int rand_y = rng->RandRange(b + e,(b + d) - h);

					grid.cells[i][j].start_x = rand_x;
					grid.cells[i][j].end_x = rand_x +1;
					grid.cells[i][j].start_y = rand_y;
					grid.cells[i][j].end_y = rand_y +1;
					auto& t = GetTile(rand_x, rand_y);
					t.is_junction = true;
					t.roomID = 0xFE;
					t.terrain = terrain::NORMAL;
				}
				else {
					int randA = rng->RandRange(5, c);
					int randB = rng->RandRange(4, d);

					if ((randA | 1) < c) {
						randA = randA | 1;
					}
					if ((randB | 1) < d) {
						randB = randB | 1;
					}
					if ((randB * 3 / 2) < randA) {
						randA = randB * 3 / 2;
					}
					if ((randA * 3 / 2) < randB) {
						randB = randA * 3 / 2;
					}

					a += rng->RandInt(c - randA);
					b += rng->RandInt(d - randB);
					grid.cells[i][j].start_x = a;
					grid.cells[i][j].end_x = a + randA;
					grid.cells[i][j].start_y = b;
					grid.cells[i][j].end_y = b + randB;
					
					auto mark_room = [=](tile& t) { t.terrain = terrain::NORMAL;
													t.roomID = room_num; };
					markTiles(a, b, a + randA, b + randB, mark_room);

					bool flag1 = floor_props.f_secondary_structures && rng->RandInt(100) < 50;
					bool flag2 = floor_props.max_secondary_structures;
					if (flag1 && flag2) {
						int rand = rng->RandInt(100);
						flag2 = 0x31 < rand && flag2;
						if (a < 49) {
							flag1 = false;
						}
					}
					if (flag1) {
						grid.cells[i][j].flag_secondary_structure = true;
					}
					if (flag2) {
						grid.cells[i][j].flag_imperfect = true;
					}
					
					room_num++;
				}
			}
		}
	}
}

void Generator::AssignGridCellConnections(int cursor_x, int cursor_y) {

	int rand = rng->RandInt(4);
	for (int i = 0; i < floor_props.floor_connectivity; i++) {
		int rand_a = rng->RandInt(8);
		int rand_b = rng->RandInt(4);

		if (rand_a < 4) {
			rand = rand_b;
		}


		bool flag = false;
		while (!flag) {
			switch(rand % 4) {
			case 0:
				if (cursor_x < grid.x -1) {
					flag = true;
				}
				else {
					rand++;
				}
				break;
			case 1:
				if (cursor_y > 0) {
					flag = true;
				}
				else {
					rand++;
				}
				break;
			case 2:
				if (cursor_x > 0) {
					flag = true;
				}
				else {
					rand++;
				}
				break;
			case 3:
				if (cursor_y < grid.y -1) {
					flag = true;
				}
				else {
					rand++;
				}
				break;
			}
		}

		switch(rand % 4) {
		case 0:
			if (!grid.cells[cursor_x +1][cursor_y].is_invalid) {
				grid.cells[cursor_x][cursor_y].is_connected_to_right = true;
				grid.cells[cursor_x +1][cursor_y].is_connected_to_left = true;
				cursor_x++;
			}
			break;

		case 1:
			if (!grid.cells[cursor_x][cursor_y -1].is_invalid) {
				grid.cells[cursor_x][cursor_y].is_connected_to_top = true;
				grid.cells[cursor_x][cursor_y -1].is_connected_to_bottom = true;
				cursor_y--;
			}
			break;

		case 2:
			if (!grid.cells[cursor_x -1][cursor_y].is_invalid) {
				grid.cells[cursor_x][cursor_y].is_connected_to_left = true;
				grid.cells[cursor_x -1][cursor_y].is_connected_to_right = true;
				cursor_x--;
			}
			break;

		case 3:
			if (!grid.cells[cursor_x][cursor_y +1].is_invalid) {
				grid.cells[cursor_x][cursor_y].is_connected_to_bottom = true;
				grid.cells[cursor_x][cursor_y +1].is_connected_to_top = true;
				cursor_y++;
			}
			break;

		}
	}
	
	if (!floor_props.allow_dead_ends) {
		bool repeat = true;
		while (repeat) {
			repeat = false;
			for (int i = 0; i < grid.x; i++) {
				for (int j = 0; j < grid.y; j++) {
					if (!grid.cells[i][j].is_invalid && !grid.cells[i][j].is_room && grid.cells[i][j].is_connected_exactly_once()) {
						int rand = rng->RandInt(4);
						bool f_break = false;
						for (int k = 0; k < 8; k++) {
							switch(rand & 3) {
							case 0:
								if (i >= grid.x -1 || (f_break = !grid.cells[i][j].is_connected_to_right, !f_break)) {		//note weird assignment in all these if's
									rand++;
								}
								break;
							case 1:
								if (j < 1 || (f_break = !grid.cells[i][j].is_connected_to_top, !f_break)) {
									rand++;
								}
								break;
							case 2:
								if (i < 1 || (f_break = !grid.cells[i][j].is_connected_to_left, !f_break)) {
									rand++;
								}
								break;
							case 3:
								if (j >= grid.y -1 || (f_break = !grid.cells[i][j].is_connected_to_bottom, !f_break)) {
									rand++;
								}
								break;
							}
							if (f_break) {
								break;
							}
						}

						if (f_break) {
							switch (rand & 3) {
							case 0:
								if (!grid.cells[i+1][j].is_invalid) {
									repeat = true;
									grid.cells[i][j].is_connected_to_right = true;
									grid.cells[i][j+1].is_connected_to_left = true;
								}
								break;
							case 1:
								if (!grid.cells[i+1][j].is_invalid) {
									repeat = true;
									grid.cells[i][j].is_connected_to_top = true;
									grid.cells[i][j-1].is_connected_to_bottom = true;
								}
								break;
							case 2:
								if (!grid.cells[i+1][j].is_invalid) {
									repeat = true;
									grid.cells[i][j].is_connected_to_left = true;
									grid.cells[i-1][j].is_connected_to_right = true;
								}
								break;
							case 3:
								if (!grid.cells[i+1][j].is_invalid) {
									repeat = true;
									grid.cells[i][j].is_connected_to_bottom = true;
									grid.cells[i+1][j].is_connected_to_top = true;
								}
								break;
							}
						}
					}
				}
			}
		}
	}
}

void Generator::TryMergeRooms(gridcell_t& origin, gridcell_t& target) {
	if (target.is_connected_room() && !target.has_secondary_structure && !target.is_merged_room ) {
		int minX = std::min(origin.start_x, target.start_x);
		int minY = std::min(origin.start_y, target.start_y);
		int maxX = std::max(origin.end_x, target.end_x);
		int maxY = std::max(origin.end_y, target.end_y);
		int tempRoomID = GetTile(origin.start_x, origin.start_y).roomID;
		auto merge = [=] (tile& t) {
			t.terrain = terrain::NORMAL;
			t.roomID = tempRoomID;
		};
		markTiles(minX, minY, maxX, maxY, merge);

		target.start_x = minX;								
		target.end_x = maxX;
		target.start_y = minY;
		target.end_y = maxY;
		target.was_merged_into_other_room = true;
		origin.is_merged_room = true;
		origin.is_connected = false;
		origin.was_merged_into_other_room = true;
	}
}

void Generator::CreateGridCellConnections(bool disable_merging) {
	for (int i = 0; i < grid.x; i++) {
		for (int j = 0; j < grid.y; j++) {
			auto& cell = grid.cells[i][j];
			if (!cell.is_invalid) {
				if (i < 1) {
					cell.is_connected_to_left = false;
				}
				if (j < 1) {
					cell.is_connected_to_top = false;
				}
				if (grid.x -1 <= i) {
					cell.is_connected_to_right = false;
				}
				if (grid.y -1 <= j) {
					cell.is_connected_to_bottom = false;
				}
				cell.should_connect_to_bottom = cell.is_connected_to_bottom;
				cell.should_connect_to_left = cell.is_connected_to_left;
				cell.should_connect_to_right = cell.is_connected_to_right;
				cell.should_connect_to_top = cell.is_connected_to_top;

			}
			else {
				cell.should_connect_to_bottom = false;
				cell.should_connect_to_left = false;
				cell.should_connect_to_right = false;
				cell.should_connect_to_top = false;
			}
		}
	}

	/* Create hallways connecting grid cells (rooms and hallway anchors) */
	for (int i = 0; i < grid.x; i++) {
		for (int j = 0; j < grid.y; j++) {
			int createHallwayParam1;
			int createHallwayParam2;
			int createHallwayParam3; 

			if (!grid.cells[i][j].is_invalid) {
				if (!grid.cells[i][j].is_room) {
					createHallwayParam1 = grid.cells[i][j].start_x;
					createHallwayParam2 = grid.cells[i][j].start_y;
				}
				else {
					createHallwayParam1 = rng->RandRange(grid.cells[i][j].start_x +1, grid.cells[i][j].end_x -1);
					createHallwayParam2 = rng->RandRange(grid.cells[i][j].start_y +1, grid.cells[i][j].end_y -1);	
				}

				if (grid.cells[i][j].should_connect_to_top) {
					if (!grid.cells[i][j-1].is_invalid) {
						if (!grid.cells[i][j-1].is_room) {
							createHallwayParam3 = grid.cells[i][j-1].start_x;
						}
						else {
							createHallwayParam3 = rng->RandRange(grid.cells[i][j-1].start_x +1, grid.cells[i][j-1].end_x -1);
						}

						CreateHallway(createHallwayParam1, grid.cells[i][j].start_y, createHallwayParam3, grid.cells[i][j-1].end_y, true, gridCoords.x[i], gridCoords.y[j]);
					}
						grid.cells[i][j].should_connect_to_top = false;
						grid.cells[i][j-1].should_connect_to_bottom = false;
						grid.cells[i][j].is_connected = true;
						grid.cells[i][j-1].is_connected = true;
				}

				if (grid.cells[i][j].should_connect_to_bottom) {
					if (!grid.cells[i][j+1].is_invalid) {
						if (!grid.cells[i][j+1].is_room) {
							createHallwayParam3 = grid.cells[i][j+1].start_x;
						}
						else {
							createHallwayParam3 = rng->RandRange(grid.cells[i][j+1].start_x +1, grid.cells[i][j+1].end_x -1);
						}
						
						CreateHallway(createHallwayParam1, grid.cells[i][j].end_y -1, createHallwayParam3, grid.cells[i][j+1].start_y, true, gridCoords.x[i], gridCoords.y[j+1] -1);
					}
					grid.cells[i][j].should_connect_to_bottom = false;
					grid.cells[i][j+1].should_connect_to_top = false;
					grid.cells[i][j].is_connected = true;
					grid.cells[i][j+1].is_connected = true;
				}
				if (grid.cells[i][j].should_connect_to_left) {
					if (!grid.cells[i-1][j].is_invalid) {
						if (!grid.cells[i-1][j].is_room) {
							createHallwayParam1 = grid.cells[i-1][j].start_y;
						}
						else {
							createHallwayParam1 = rng->RandRange(grid.cells[i-1][j].start_y +1, grid.cells[i-1][j].end_y -1);
						}

						CreateHallway(grid.cells[i][j].start_x, createHallwayParam2, grid.cells[i-1][j].start_x -1, createHallwayParam1, false, gridCoords.x[i], gridCoords.y[j]);
						
					}
					grid.cells[i][j].should_connect_to_right = false;
					grid.cells[i-1][j].should_connect_to_left = false;
					grid.cells[i][j].is_connected = true;
					grid.cells[i-1][j].is_connected = true;
				}
				if (grid.cells[i][j].should_connect_to_right) {
					if (!grid.cells[i+1][j].is_invalid) {
						if (!grid.cells[i+1][j].is_room) {
							createHallwayParam1 = grid.cells[i+1][j].start_y;
						}
						else {
							createHallwayParam1 = rng->RandRange(grid.cells[i+1][j].start_y +1, grid.cells[i+1][j].end_y -1);
						}
						
						CreateHallway(grid.cells[i][j].end_x -1, createHallwayParam2, grid.cells[i+1][j].start_x, createHallwayParam1, false, gridCoords.x[i+1]-1, gridCoords.y[j]);
					}
					grid.cells[i][j].should_connect_to_right = false;
					grid.cells[i+1][j].should_connect_to_left = false;
					grid.cells[i][j].is_connected = true;
					grid.cells[i+1][j].is_connected = true;
				}
			}
		}	
	}
	if (!disable_merging) {
		for (int i = 0; i < grid.x; i++) {
			for (int j = 0; j < grid.y; j++) {
				auto& origin = grid.cells[i][j];
				if (rng->RandInt(100) < 5 && origin.is_connected_room() &&
					!origin.is_merged_room && !origin.has_secondary_structure ) {
					switch(rng->RandInt(4)) {
						case 0:	//merge to the left
						{
							if (i > 0) {
								TryMergeRooms(origin, grid.cells[i-1][j]);
							}
							break;
						}
						case 1:	//merge down
						{
							if (j > 0) {
								TryMergeRooms(origin, grid.cells[i][j-1]);
							}
							break;
						}
						case 2:	//merge right
						{
							if (i <= grid.x -2) {
								TryMergeRooms(origin, grid.cells[i+1][j]);
							}
							break;
						}
						case 3:	//merge up
						{
							if (j <= grid.y -2) {
								TryMergeRooms(origin, grid.cells[i][j+1]);
							}
							break;
						}
					}
				}
			}
		}
	}
	return;
}

void Generator::EnsureConnectedGrid() {
	int i = 0;
	while (true) {
		if (grid.x <= i) {
			for (int i = 0; i < grid.x; i++) {		//seems to erase some rooms
				for (int j = 0; j < grid.y; j++) {
					auto& cell = grid.cells[i][j]; 
					if (!cell.is_invalid && !cell.is_connected && !cell.was_merged_into_other_room && !cell.field_0xf) {
						auto erase = [] (tile& t) { 
							t.roomID = 0xFF; 
							t.terrain = terrain::WALL; 
						};
						markTiles(cell.start_x, cell.start_y, cell.end_x, cell.end_y, erase);
					}
				}
			}
			return;
		}

		for (int j = 0; j < grid.y; j++) {
			auto& cellO = grid.cells[i][j];
			if (!cellO.is_invalid && !cellO.was_merged_into_other_room && !cellO.is_connected) {
				if (!cellO.is_room || cellO.has_secondary_structure) {
					GetTile(cellO.start_x, cellO.start_y).terrain = terrain::WALL;
					//skipping clearing some flags since those shouldnt have been touched by the algorithm yet anyway
				}
				else {
					int a = rng->RandRange(cellO.start_x, cellO.end_x -1);
					int b = rng->RandRange(cellO.start_y, cellO.end_y -1);				

					if (j > 0) {
						auto& cellM = grid.cells[i][j-1];
						if (cellM.is_connected_room() && !cellM.is_merged_room) {
							int c = cellM.start_x;
							[[maybe_unused]] int d = cellM.start_y;
							if (cellM.is_room) {
								c = rng->RandRange(cellM.start_x, cellM.end_x -1);
								d = rng->RandRange(cellM.start_y, cellM.end_y -1);	//the game calls the rng function here, without actually using the return value in this if-block. The devs copy-pasted it in, I guess.
							}
							CreateHallway(a, cellO.start_y, c, cellM.end_y -1, true, gridCoords.x[i], gridCoords.y[j]);
							cellO.is_connected = true;
							cellO.is_connected_to_top = true;
							cellM.is_connected_to_bottom = true;

						}
						

					}
					else if (j < grid.y -1) {
						auto& cellM = grid.cells[i][j+1];
						if (cellM.is_connected_room() && !cellM.is_merged_room) {
							int c = cellM.start_x;
							[[maybe_unused]] int d = cellM.start_y;
							if (cellM.is_room) {
								c = rng->RandRange(cellM.start_x, cellM.end_x -1);
								d = rng->RandRange(cellM.start_y, cellM.end_y -1);
							}
							CreateHallway(a, cellO.end_y -1, c, cellM.start_y, true, gridCoords.x[i], gridCoords.y[j+1] -1);
							cellO.is_connected = true;
							cellO.is_connected_to_bottom = true;
							cellM.is_connected_to_top = true;

						}
						

					}
					else if (i > 0) {
						auto& cellM = grid.cells[i-1][j];
						if (cellM.is_connected_room() && !cellM.is_merged_room) {
							[[maybe_unused]] int c = cellM.start_x;
							int d = cellM.start_y;
							if (cellM.is_room) {
								c = rng->RandRange(cellM.start_x, cellM.end_x -1);
								d = rng->RandRange(cellM.start_y, cellM.end_y -1);
							}
							CreateHallway(cellO.start_x, b, cellM.start_x, d, false, gridCoords.x[i], gridCoords.y[j]);
							cellO.is_connected = true;
							cellO.is_connected_to_left = true;
							cellM.is_connected_to_right = true;

						}
						

					}
					else if (i < grid.x -1) {
						auto& cellM = grid.cells[i+1][j];
						if (cellM.is_connected_room() && !cellM.is_merged_room) {
							[[maybe_unused]] int c = cellM.start_x;
							int d = cellM.start_y;
							if (cellM.is_room) {
								c = rng->RandRange(cellM.start_x, cellM.end_x -1);
								d = rng->RandRange(cellM.start_y, cellM.end_y -1);
							}
							CreateHallway(cellO.end_x -1, b, cellM.start_x, d, false, gridCoords.x[i+1] -1, gridCoords.y[j]);
							cellO.is_connected = true;
							cellO.is_connected_to_right = true;
							cellM.is_connected_to_left = true;
						}
					}
				}
			}
		}
		i++;
	}
}

void Generator::GenerateMazeRooms([[maybe_unused]]int spawnChance) {
	return;
	// if (spawnChance > 0 && rng->RandInt(100) < spawnChance && floor.floorGenerationCounter < 0)	//todo: implement maze room gen
}

bool Generator::GenerateKecleonShop(int spawnChance) {
	if (rng->RandInt(100) < spawnChance) {
		std::array<int, 15> randomArrayX = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
		std::array<int, 15> randomArrayY = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
		int i = 0;
		while (i < 200) {
			int a = rng->RandInt(15);
			int b = rng->RandInt(15);
			std::swap(randomArrayX[a], randomArrayX[b]);
			i++;
		}
		i = 0;
		while (i < 200) {
			int a = rng->RandInt(15);
			int b = rng->RandInt(15);
			std::swap(randomArrayY[a], randomArrayY[b]);
			i++;
		}

		for (int i = 0; i < grid.x; i++) {
			if (randomArrayX[i] < grid.x) {
				for (int j = 0; j < grid.y; j++) {
					if (randomArrayY[j] < grid.y) {
						auto& cell = grid.cells[randomArrayX[i]][randomArrayY[j]];
						if (cell.can_spawn_kek_shop()) {
							if (abs(cell.end_x - cell.start_x) > 4 && abs(cell.end_y - cell.start_y) > 3) {		//room is at least 4x3
								cell.is_kecleon_shop = true;
								markTiles(cell.start_x, cell.start_y, cell.end_x, cell.end_y, [](tile& t) { t.isKecleonShop = true; });
								//here the game writes the shop coordinates into the status struct, but that doesnt get used for the rest of (this part of) the algorithm
								//additionally, it sets the f_items flag for each tile in the grid cell
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

bool Generator::GenerateMonsterHouse(int spawnChance) {
	if (spawnChance > 0 && rng->RandInt(100) < spawnChance && status.has_kecleon_shop && status.can_spawn_mon_house()) {
		int validSpawnRoomCounter = 0;
		for (int i = 0; i < grid.x; i++) {
			for (int j = 0; j < grid.y; j++) {
				if (grid.cells[i][j].can_spawn_mon_house()) {
					validSpawnRoomCounter++;
				}
			}
		}

		/* Shuffle a flag around in an array to pick a random room */
		if (validSpawnRoomCounter > 0) {
			std::array<bool, 0x100> flagArray;
			std::ranges::fill(flagArray, false);
			flagArray[0] = true;

			for (int i = 0; i < 0x40; i++) {
				int a = rng->RandInt(validSpawnRoomCounter);				
				int b = rng->RandInt(validSpawnRoomCounter);
				std::swap(flagArray[a], flagArray[b]);
			}

			/* Mark a room as a monster house */
			int flagArrayIndex = 0;
			for (int i = 0; i < grid.x; i++) {
				for (int j = 0; j < grid.y; j++) {
					if (grid.cells[i][j].can_spawn_mon_house() && flagArray[flagArrayIndex]) {
						auto& cell = grid.cells[i][j];
						//here the game actually sets a flag marked unknown in the decompile project
						//presumably, the real in_monster_house flag is set at a later stage. We'll just set it right away.
						markTiles(cell.start_x, cell.start_y, cell.end_x, cell.end_y, [](tile& t) { t.isMonsterHouse = true; });
						//also, it writes the room ID into the dungeon struct presumably also to be used later
						return true;
					}
					flagArrayIndex++;
				}
			}
		}
	}
	return false;
}

void Generator::GenerateExtraHallways(int num_hallways) {
	while (num_hallways-- > 0) {
		int random_start_x = rng->RandInt(grid.x);
		int random_start_y = rng->RandInt(grid.y);
		auto& cell = grid.cells[random_start_x][random_start_y];
		if (cell.is_room && cell.is_connected && !cell.is_invalid && !cell.is_maze_room ) {
		
			int cursor_x = rng->RandRange(cell.start_x, cell.end_x);
			int cursor_y = rng->RandRange(cell.start_y, cell.end_y);
			int random_step_dist = rng->RandInt(4) * 2;

			for (int i = 0; i < 3; i++) {
				if ((random_step_dist == 0) && (grid.y -1 <= random_start_x)) {
					random_step_dist = 2;
				}
				if ((random_step_dist == 2) && (grid.x -1 <= random_start_y)) {
					random_step_dist = 4;
				}
				if ((random_step_dist == 4) && (random_start_y < 1)) {
					random_step_dist = 6;
				}
				if ((random_step_dist == 6) && (random_start_x < 1)) {
					random_step_dist = 0;
				}
			}
			
			/* Used by the game for randomness */
			const std::array<int16_t, 64> memory_block = {
				0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, -1, -1, 
				0, 0, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, -1, -1, 1, 0, 
				0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, -1, 
				0, 0, 0, -1, 0, -1, 0, -1, 0, -1, 0, 0, 0, -1, 0, 1
			};

			int start_room_id = GetTile(cursor_x, cursor_y).roomID;
			while (start_room_id == GetTile(cursor_x, cursor_y).roomID) {
				cursor_x += memory_block[random_step_dist * 4];
				cursor_y += memory_block[random_step_dist * 4 +2];
			}
			while (GetTile(cursor_x, cursor_y).terrain == terrain::NORMAL) {
				cursor_x += memory_block[random_step_dist * 4];
				cursor_y += memory_block[random_step_dist * 4 +2];
			}

			if (GetTile(cursor_x, cursor_y).terrain != terrain::SECONDARY) {
				bool break_outer = false;
				for (int i = cursor_x -2;  i <= cursor_x + 2; i++) {
					for (int j = cursor_y -2; j <= cursor_y + 2; j++) {
						if (i < 0 || i > floor_x -1 || j < 0 || j > floor_y -1) {
							break_outer = true;
							break;
						}
					}
					if (break_outer) { break; };
				}

				if (!break_outer) {
					int rand1 = ((random_step_dist + 2) & 6) * 4;
					int rand2 = ((random_step_dist - 2) & 6) * 4;
					if (GetTile(cursor_x + memory_block[rand1], cursor_y + memory_block[rand1 +2]).terrain != terrain::NORMAL && 
						GetTile(cursor_x + memory_block[rand2], cursor_y + memory_block[rand2 +2]).terrain != terrain::NORMAL ) 
					{
						int rand = rng->RandInt(3) + 3;
						while (cursor_x > 1 && cursor_y > 1 && 
							   cursor_x < floor_x -1 && cursor_y < floor_y -1 &&
							   GetTile(cursor_x, cursor_y).terrain != terrain::NORMAL /* &&
							   GetTile(cursor_x, cursor_y).is_impassable = false */ 	// uncomment in case impassable flag gets implemented
						)
						{
							/* checks if the 3 adjacent tiles are all normal (open) terrain, for example tiles down, right, and diagonally down right from the cursor. 
							   If that's the case, the extra hallway is done. Otherwise, continue the random walk. */
							bool set_normal_terrain_flag = true;
							if (GetTile(cursor_x +1, cursor_y).terrain == terrain::NORMAL &&			//down-right
								GetTile(cursor_x +1, cursor_y +1).terrain == terrain::NORMAL &&
								GetTile(cursor_x, cursor_y +1).terrain == terrain::NORMAL) 
							{
								set_normal_terrain_flag = false;
							}
							else if (GetTile(cursor_x +1, cursor_y).terrain == terrain::NORMAL &&		//up-right
									 GetTile(cursor_x +1, cursor_y -1).terrain == terrain::NORMAL &&
									 GetTile(cursor_x, cursor_y -1).terrain == terrain::NORMAL) 
							{
								set_normal_terrain_flag = false;
							}							
							else if (GetTile(cursor_x -1, cursor_y).terrain == terrain::NORMAL &&		//down-left
									 GetTile(cursor_x -1, cursor_y +1).terrain == terrain::NORMAL &&
									 GetTile(cursor_x, cursor_y +1).terrain == terrain::NORMAL) 
							{
								set_normal_terrain_flag = false;
							}
							else if (GetTile(cursor_x -1, cursor_y).terrain == terrain::NORMAL &&		//up-left
									 GetTile(cursor_x -1, cursor_y -1).terrain == terrain::NORMAL &&
									 GetTile(cursor_x, cursor_y -1).terrain == terrain::NORMAL) 
							{
								set_normal_terrain_flag = false;
							}

							if (set_normal_terrain_flag) {
								GetTile(cursor_x, cursor_y).terrain = terrain::NORMAL;
							}	
							
							if (GetTile(cursor_x + memory_block[rand1], cursor_y + memory_block[rand1 +2]).terrain == terrain::NORMAL ||
								GetTile(cursor_x + memory_block[rand2], cursor_y + memory_block[rand2 +2]).terrain == terrain::NORMAL) 
							{
								break;
							}
							
							rand--;
							if (rand == 0) {
								rand = rng->RandInt(3) +3;
								if (rng->RandInt(100) < 0x32) {
									random_step_dist += 2;
								} else {
									random_step_dist -= 2;
								}
								random_step_dist &= 6;

								if (random_step_dist == 2 && ( (cursor_x > floor_y -1 && status.floor_size == floor_size::SMALL) || (cursor_y > 0x2f && status.floor_size == floor_size::MEDIUM) )) {
									break;
								}
							}
							rand1 = ((random_step_dist + 2) & 6) * 4;
							rand2 = ((random_step_dist - 2) & 6) * 4;
							cursor_x += memory_block[random_step_dist * 4];
							cursor_y += memory_block[random_step_dist * 4 +2];
						}
					}
				}
            }
		}
	}
}

/* for some reason, this function got compiled into goto spaghetti. Luckily these are disabled in the vanilla game just like maze rooms TODO: implement */
void Generator::GenerateRoomImperfections() {
	return;
// 	int n;
// 	int n2;
// 	int count;
// 	int a, b;
// 	int x, y;

// 	for (int i = 0; i < grid.x; i++) {
// 		for (int j = 0; j < grid.y; j++) {
// 			auto& cell = grid.cells[i][j];
// 			if (cell.is_connected_room() && cell.is_not_merged() && 
// 				!cell.has_secondary_structure && !cell.is_maze_room && cell.flag_imperfect &&
// 				rng->RandInt(100) < 0x3c) 
// 			{
// 				n = cell.end_x - cell.start_x + cell.end_y - cell.start_y;
// 				n2 = n + ((static_cast<unsigned int>(n) >> 1) >> 0x1e) >> 2;	//todo: simplify this
// 				if (n2 == 0) {
// 					n2 = 1;
// 				}
// 				count = 0;

// LABEL_A:
// 				if (count < n2) {
// 					n = 0;
// LABEL_B:
// 					switch(rng->RandInt(4)) {
// 					case 0:
// 						if (n != 0) {
// 							a = 1;
// 							b = 0;
// 						}
// 						else {
// 							a = 0;
// 							b = 1;
// 						}
// 						x = cell.start_x;
// 						y = cell.start_y;

//                 		break;					
// 					case 1:
// 						if (n != 0) {
// 							a = 0;
// 							b = 1;
// 						}
// 						else {
// 							a = 0;
// 							b = 0;
// 						}
// 						x = cell.end_x -1;
// 						y = cell.start_y;
// 						break;
// 					case 2:
// 						if (n != 0) {
// 							b = 0;
// 							a = 0xffffffff;
// 							uVar6 = ffffff;
// 						}
// 						else {
// 							uVar6 = 0;
// 							b = ffffff;
// 							a = uVar6;
// 						}
// 						x = cell.end_x -1;
// 						y = cell.end_y -1;
// 						break;
// 					case 3:
// 					case 4:
// 					}
// 				}

// 			}
// 		}
// 	}
}

void Generator::GenerateSecondaryStructures() {
	for (int j = 0; j < grid.y; j++) {		//note backwards i, j loops
		for (int i = 0; i < grid.x; i++) {
			auto& cell = grid.cells[i][j];
			if (!cell.is_invalid && cell.is_room && !cell.is_monster_house && !cell.is_merged_room && !cell.flag_imperfect && cell.flag_secondary_structure) {
				switch(rng->RandInt(6)) {
				case 0:
					break;
				case 1:
					if (status.secondary_structures_budget > 0) {
						status.secondary_structures_budget--;
						int x_dist = cell.end_x - cell.start_x;
						int y_dist = cell.end_y - cell.start_y;
						if (x_dist % 2 == 0 || y_dist % 2 != 0) {
							int x_mid = (cell.start_x + cell.end_x) / 2;
							int y_mid = (cell.start_y + cell.end_y) / 2;
							if (x_dist > 4 || y_dist < 5) {
								GetTile(x_mid, y_mid).terrain = terrain::SECONDARY;
							}
							else {	// "+" shape of water or lava
								GetTile(x_mid, y_mid).terrain = terrain::SECONDARY;								
								GetTile(x_mid +1, y_mid).terrain = terrain::SECONDARY;								
								GetTile(x_mid, y_mid +1).terrain = terrain::SECONDARY;
								GetTile(x_mid -1, y_mid).terrain = terrain::SECONDARY;
								GetTile(x_mid, y_mid -1).terrain = terrain::SECONDARY;
							}
						}
						else {
							//skipping setting an unknown flag on a bunch of tiles
							GenerateMaze(cell, true);
						}
					}
					break;
				case 2:
					if (status.secondary_structures_budget > 0 && cell.end_x - cell.start_x != 0 && cell.end_y - cell.start_y != 0) {
						status.secondary_structures_budget--;
						//skipping setting an unknown flag on a bunch of tiles
						for (int k = 0; k < 0x40; k++) {
							unsigned int rand_a = rng->RandInt(cell.end_x - cell.start_x);
							unsigned int rand_b = rng->RandInt(cell.end_y - cell.start_y);
							if ((rand_a + rand_b) % 1 != 0) {
								GetTile(cell.start_x + rand_a, cell.start_y + rand_b).terrain = terrain::SECONDARY;
							}
						}
						cell.has_secondary_structure = true;
					}
					break;
				case 3:
					if (cell.end_x - cell.start_x > 4 && cell.end_y - cell.start_y > 4) {
						int a = rng->RandRange(cell.start_x +2, cell.end_x -3);
						int b = rng->RandRange(cell.start_y +2, cell.end_y -3);
						int c = rng->RandRange(cell.start_x +2, cell.end_x -3);
						int d = rng->RandRange(cell.start_y +2, cell.end_y -3);
						if (c < a) {
							std::swap(c, a);
						}
						if (d < b) {
							std::swap(d, b);
						}

						for (a = b; a <= c; a++) {	//todo: cleanup
							for (; a <= d; a++) {
								GetTile(a, a).terrain = terrain::SECONDARY;
							}
						}
						status.secondary_structures_budget++;
					}
					break;
				case 4:
					if (cell.end_x - cell.start_x > 4 && cell.end_y - cell.start_y > 5) {
						int half_x = (cell.start_x + cell.end_x) / 2;
						int half_y = (cell.start_x + cell.end_x) / 2;
						if (status.secondary_structures_budget > 0) {
							status.secondary_structures_budget--;
							int a = half_x -2;
							int b = half_y -2;
							int c = half_x -1;
							int d = half_y -1;
							GetTile(a, b).terrain = terrain::SECONDARY;
							GetTile(c, b).terrain = terrain::SECONDARY;
							GetTile(half_x, b).terrain = terrain::SECONDARY;
							GetTile(half_x + 1, b).terrain = terrain::SECONDARY;
							GetTile(a, d).terrain = terrain::SECONDARY;
							GetTile(a, half_y).terrain = terrain::SECONDARY;
							GetTile(a, half_y +1).terrain = terrain::SECONDARY;
							GetTile(a, half_y +1).terrain = terrain::SECONDARY;
							GetTile(c, half_y +1).terrain = terrain::SECONDARY;
							//ptVar3->terrain_type = ptVar3->terrain_type | 4;
							GetTile(half_x, half_y + 1).terrain = terrain::SECONDARY;		//todo: cleanup
							GetTile(half_x + 1, b).terrain = terrain::SECONDARY;
							GetTile(half_x + 1, d).terrain = terrain::SECONDARY;
							GetTile(half_x + 1, half_y).terrain = terrain::SECONDARY;
							GetTile(half_x + 1, half_y + 1).terrain = terrain::SECONDARY;
							GetTile(c, d).hasTrap = true;
							// GetTile(c, d].has | 0x40;		//sets bit 7 of spawn_flags, an unknown bit
							GetTile(half_x, d).hasItem = true;
							GetTile(c, half_y).hasItem = true;
							GetTile(half_x, half_y).hasItem = true;
							// GetTile(c, d].has | 0x10);		//bit 5, another unknown bit
							// GetTile(half_x, d].has | 0x10);
							// GetTile(c, half_y].has | 0x10);
							// GetTile(half_x,half_y].has | 0x10);
							
							cell.has_secondary_structure = true;
						}
					}
					break;
				case 5:		//split room
					if (status.secondary_structures_budget > 0) {
						status.secondary_structures_budget--;
						//skipping setting an unknown flag on a bunch of tiles
						if (rng->RandInt(2) == 0) {
							int half_y = (cell.start_y + cell.end_y) / 2;
							for (int x = cell.start_x; x < cell.end_x -1; x++) {
								if (!IsNextToHallway(x, half_y)) {
									markTiles(cell.start_x, half_y, cell.end_x, half_y, [](tile& t){ t.terrain = terrain::SECONDARY; });
									markTiles(cell.start_x, cell.start_y, cell.end_x, cell.end_y, [](tile& t){ t.isUnreachableFromStairs = true; });
								}
							}
							cell.has_secondary_structure = true;
						}
						else {
							int half_x = (cell.start_x + cell.end_x) / 2;
							for (int y = cell.start_y; y < cell.end_y -1; y++) {
								if (!IsNextToHallway(y, half_x)) {
									markTiles(half_x, cell.start_y, half_x, cell.end_y, [](tile& t){ t.terrain = terrain::SECONDARY; });
									markTiles(cell.start_x, cell.start_y, cell.end_x, cell.end_y, [](tile& t){ t.isUnreachableFromStairs = true; });
								}
							}
							cell.has_secondary_structure = true;
						}
					}
					break;
				}
			}
		}
	}
}

bool Generator::IsNextToHallway(int x, int y) {		//todo: pass tile&
	for (int i = -1; i <= 1; i++) {	// -1, 0, 1
		const int x_00 = x + i;
		if (x_00 >= 0) {
			if (x_00 < floor_x -1) {
				return false;
			}
			for (int j = -1; j <= 1; j++) {
				const int y_00 = y + j;
				if (y_00 > floor_y -1) {
					break;
				}
				if ((i == 0 || j == 0) &&
					 GetTile(i, j).terrain == terrain::NORMAL &&
					 GetTile(i, j).roomID == 0xFF )
				{
					return true;
				}
			}
		}
	}
	return false;
}

void Generator::GenerateMaze(gridcell_t& cell, bool use_secondary_terrain) {
	status.has_maze = true;
	cell.is_maze_room = true;
	int roomID = GetTile(cell.start_x, cell.start_y).roomID;

	for (int i = cell.start_x + 1; i < cell.end_x -1; i += 2) {
		if (GetTile(i, cell.start_y -1).terrain != terrain::NORMAL) {
			GenerateMazeLine(i,cell.start_y -1, cell.start_x, cell.start_y, cell.end_x, cell.end_y, use_secondary_terrain, roomID);
		}
	}
	for (int i = cell.start_y + 1; i < cell.end_y -1; i += 2) {
		if (GetTile(cell.end_x, i).terrain != terrain::NORMAL) {
			GenerateMazeLine(cell.end_x, i, cell.start_x, cell.start_y, cell.end_x, cell.end_y, use_secondary_terrain, roomID);
		}
	}
	for (int i = cell.start_x + 1; i < cell.end_x -1; i += 2) {
		if (GetTile(i, cell.end_y).terrain != terrain::NORMAL) {
			GenerateMazeLine(i, cell.end_y, cell.start_x, cell.start_y, cell.end_x, cell.end_y, use_secondary_terrain, roomID);
		}
	}
	for (int i = cell.start_y + 1; i < cell.end_y -1; i += 2) {
		if (GetTile(cell.start_x -1, i).terrain != terrain::NORMAL) {
			GenerateMazeLine(cell.start_x -1, i, cell.start_x, cell.start_y, cell.end_x, cell.end_y, use_secondary_terrain, roomID);
		}
	}

	for (int i = cell.start_x + 3; i < cell.end_x -3; i += 2) {
		for (int j = cell.start_y + 3; j < cell.end_y -3; j += 2) {
			if (GetTile(i, j).terrain == terrain::NORMAL) {
				if (use_secondary_terrain) {
					GetTile(i -1, j).terrain = terrain::SECONDARY;
				}
				else {
					GetTile(i -1, j).terrain = terrain::WALL;
				}
				GenerateMazeLine(i, j, cell.start_x, cell.start_y, cell.end_x, cell.end_y, use_secondary_terrain, roomID);
			}
		}
	}

  return;
}

void Generator::GenerateMazeLine(int x0, int y0, int xmin, int ymin, int xmax, int ymax, bool use_secondary_terrain, int roomID) {
	int i = 0;
	int rand_fallthrough = rng->RandInt(4);
	while (true) {
		SetTerrainObstacleChecked(GetTile(x0, y0), use_secondary_terrain, roomID);
		while (true) {
			int a;
			int b;
			switch(rand_fallthrough % 4) {
			case 0:
				a = 2;
				b = 0;
				break;
			case 1:
				a = 0;
				b = -2;
				break;
			case 2:
				a = -2;
				b = 0;
				break;
			case 3:
				a = 0;
				b = 2;
				break;
			}
			a += x0;
			b += y0;
			i++;
			rand_fallthrough++;
			if (a >= xmin && a < xmax && b >= ymin && b < ymax && GetTile(a, b).terrain == terrain::NORMAL) {
				break;
			}
			if (i > 3) {
				return;
			}
		}
		switch(rand_fallthrough % 4) {
		case 0:
			SetTerrainObstacleChecked(GetTile(x0 +1, y0), use_secondary_terrain, roomID);
			x0 += 2;
			break;
		case 1:
			SetTerrainObstacleChecked(GetTile(x0, y0 -1), use_secondary_terrain, roomID);
			y0 += -2;
			break;
		case 2:
			SetTerrainObstacleChecked(GetTile(x0 -1, y0), use_secondary_terrain, roomID);
			x0 += -2;
			break;
		case 3:
			SetTerrainObstacleChecked(GetTile(x0, y0 +1), use_secondary_terrain, roomID);
			y0 += 2;
		}
	}
}

void Generator::SetTerrainObstacleChecked(tile& tile, bool use_secondary_terrain, int roomID) {
	tile.terrain = terrain::WALL;
	if (!use_secondary_terrain) {
		return;
	}
	if (tile.roomID != roomID) {
		return;
	}
	tile.terrain = terrain::SECONDARY;
	return;
}

void Generator::markTiles(const int minX, const int minY, const int maxX, const int maxY, std::function<void(tile&)> apply_mark) {
	for (int i = minX; i < maxX; i++) {	
		for (int j = minY; j < maxY; j++) {
			apply_mark(GetTile(i, j));
		}
	}
}

void Generator::resetOuterEdgeTiles() {
	ResetInnerBoundaryTileRows();
	EnsureImpassableTilesAreWalls();
}

void Generator::ResetInnerBoundaryTileRows() {
	auto wall = [](tile& t) { t.terrain = terrain::WALL; };
	markTiles(0,  0,  0,  floor_y, wall);
	markTiles(floor_x, 0,  floor_x, floor_y, wall);
	markTiles(0,  1,  floor_x, 1,  wall);
	markTiles(0,  30, floor_x, 30, wall);
}

void Generator::EnsureImpassableTilesAreWalls() {
	auto wall = [](tile& t) { t.terrain = terrain::WALL; };
	markTiles(0,  0,  0,  32, wall);
	markTiles(56, 0,  56, 32, wall);
	markTiles(0,  0,  56, 0,  wall);	//note slight difference in y-coordinate from ResetInnerBoundaryTileRows()
	markTiles(0,  31, 56, 31, wall); 	//yes both functions set the same x-coordinate tiles to wall
}

void Generator::CreateHallway(int x0, int y0, int x1, int y1, bool vertical, int x_mid, int y_mid) {	//todo cleanup
	int count = 0;
	int x = x0;
	int y = y0;
	if (!vertical) {
		while (x != x_mid) {
			if (count > floor_x -1) { return; }

			if (GetTile(x, y0).terrain == terrain::NORMAL) {
				if (x != x0) { return; }
			}
			else {
				GetTile(x, y0).terrain = terrain::NORMAL;
			}

			if (x < x_mid) {
				x++;
				count++;
			}
			else {
				x--;
				count--;
			}

		}
		
		count = 0;
		while (true) {
			if (y == y1) {
				count = 0;
				while (true) {
					if (x == x1) { return; }

					if (count > floor_x -1) { break; }

					if (GetTile(x, y).terrain == terrain::NORMAL) {
						if (x != x0 || y != y0) {
							return;
						}
					}
					else {
						GetTile(x, y).terrain = terrain::NORMAL;
					}

					if (x < x1) {
						x++;
						count++;
					}
					else {
						x--;
						count++;
					}
				}
				return;
			}

			if (floor_x -1 < count) { return; }

			if (GetTile(x, y).terrain == terrain::NORMAL) {
				if (x != x0 || y != y0) {
					return;
				}
			}
			else {
				GetTile(x, y).terrain = terrain::NORMAL;
			}

			if (y < y1) {
				y++;
				count++;
			}
			else {
				y--;
				count--;
			}
		}
		return;
	}
	while (y != y_mid) {
		if (floor_x -1 < count) {
			return;
		}	

		if (GetTile(x0, y).terrain == terrain::NORMAL) {
			if (y != y0) {
				return;
			}
		}
		else {
			GetTile(x0, y).terrain = terrain::NORMAL;
		}

		if (y < y_mid) {
			y++;
			count++;
		}
		else {
			y--;
			count--;
		}

	}

	count = 0;
	while (x != x1) {
		if (floor_x -1 < count) { return; }

		if (GetTile(x, y).terrain == terrain::NORMAL) {
			if (x != x0 || y != y0) {
				return;
			}
		}
		else {
			GetTile(x, y).terrain = terrain::NORMAL;
		}

		if (x < x1) {
			x++;
			count++;
		}
		else {
			x--;
			count++;
		}
	}

	count = 0;
	while (true) {
		if (y == y1) { return; }

		if (floor_x -1 < count) { break; }

		if (GetTile(x, y).terrain == terrain::NORMAL) {
			if (x != x0 || y != y0) {
				return;
			}
		}
		else {
			GetTile(x, y).terrain = terrain::NORMAL;
		}

		if (y < y1) {
			y++;
			count++;
		}
		else {
			y--;
			count++;
		}
	}
	return;
}

/* In the games code this is originally done outside the function that generates the floors themselves, but all the generators execute this to finish up. */
void Generator::Finalize() {
	resetOuterEdgeTiles();
	//TODO: skipping shuffling some room ID's around, shouldn't impact the layout of the floor but should maybe still be implemented later
	if (status.num_generation_attemps >= 10) {
		return;
	}
	FinalizeJunctions();
}

/* Note: this function intentionally emulates a bug in the game. For details see FinalizeJunctions() in the pmdsky-debug project.
 * Basically this is supposed to ensure there is no water or lava near a junction point.
 */
void Generator::FinalizeJunctions() {
	for (int i = 0; i < floor_y; i++) {
		for (int j = 0; j < floor_x; j++) {
			if (GetTile(i, j).terrain == terrain::NORMAL && GetTile(i, j).roomID == 0xFF) {
				tile& t = GetTile(i -1, j);
				if (i > 0 && t.roomID != 0xFF) {
					t.is_junction = true;
					if (t.terrain == terrain::SECONDARY) {
						t.terrain = terrain::NORMAL;
					}
				}
				t = GetTile(i, j -1);
				if (j > 0 && t.roomID != 0xFF) {
					t.is_junction = true;
					if (t.terrain == terrain::SECONDARY) {
						t.terrain = terrain::NORMAL;
					}
				}
				t = GetTile(i, j +1);
				if (j < floor_y -1 && t.roomID != 0xFF) {
					t.is_junction = true;
					if (t.terrain == terrain::SECONDARY) {
						t.terrain = terrain::NORMAL;
					}
				}
				t = GetTile(i +1, j);
				if (i < floor_x -1 && t.roomID != 0xFF) {
					t.is_junction = true;
					if (t.terrain == terrain::SECONDARY) {
						t.terrain = terrain::NORMAL;
					}
				}
			}
			else if (GetTile(i, j).roomID == 0xFE) {
				GetTile(i, j).roomID = 0xFF;
			}
		}
	}
	//skipping counting all the junctions (function FUN_02340700)
	return;
}