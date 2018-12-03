#define GRID_SIZE 60.0f
#define GridSize(x, y) v2((x) * GRID_SIZE, (y) * GRID_SIZE)

internal void PlaceRoad(Overworld *world, Grid_Square *last, u32 x, u32 y) {
        Grid_Square *current = &world->grid[x][y];
        current->occupied = true;
        current->road     = true;
        current->x = x;
        current->y = y;
        current->texture_index = RandomChoice(3);

        current->prev = last;
        current->next = 0;
        last->next = current;
}

internal void GenerateWorld(Overworld *world) {
    // Clear the world first
    for (u32 i = 0; i < GRID_WIDTH; ++i) {
        for (u32 j = 0; j < GRID_HEIGHT; ++j) {
            world->grid[i][j].occupied = false;
            world->grid[i][j].road = false;
            world->grid[i][j].x = i;
            world->grid[i][j].y = j;
            world->grid[i][j].destroyed = 2;
            u32 texture_choice = RandomInt(0, 100);
            u32 texture_index = 0;
            if (texture_choice < 80) {
                texture_index = RandomChoice(2);
            }
            else {
                texture_index = RandomChoice(7);
            }
            world->grid[i][j].texture_index = texture_index;
        }
    }

    // Choose a random start position at the left of the screen
    u32 current_x = 0;
    u32 current_y = RandomInt(2, GRID_HEIGHT - 4);

    bool generating = true;

    world->path = &world->grid[current_x][current_y];

    world->path->texture_index = RandomChoice(3);
    world->path->x = 0;
    world->path->y = current_y;
    world->path->occupied = true;
    world->path->road = true;
    world->path->prev = 0;

    Grid_Square *last = world->path;
    u32 direction = 2; // Always go left
    u32 last_direction = 2;
    u32 count = RandomInt(2, 5);
    bool generate_new_direction = false;
    while (generating) {

        bool moved = false;
        while (!moved) {
            bool up_valid = current_y > 2 && last_direction != 0 &&
                 !world->grid[current_x][current_y - 2].road;
#if 0
            if (current_x > 0) {
                up_valid &= !world->grid[current_x - 1][current_y - 2].road;
            }

            if (current_x < GRID_WIDTH - 2) {
                up_valid &= !world->grid[current_x + 1][current_y - 2].road;
            }
#endif

            bool down_valid = current_y < GRID_HEIGHT - 3 && last_direction != 1;
            if (!down_valid && direction != 2) {
                count = 3;
                direction = 1; // Go up if down isn't possible
            }

            bool right_valid = current_x > 2 && last_direction != 2;
            bool left_valid = current_x < GRID_WIDTH - 1 && last_direction != 3;

            switch (direction) {
                case 0: // Down
                    if (down_valid) {
                        current_y++;
                        moved = true;
                    }
                    break;
                case 1: // Up
                    if (up_valid) {
                        current_y--;
                        moved = true;
                    }
                    break;
                case 2: // Left
                    if (left_valid) {
                        current_x++;
                        moved = true;
                    }
                    break;
                case 3: // Right
                    if (right_valid) {
                        current_x--;
                        moved = true;
                    }
                    break;
            }

            if (moved) count--;

            generate_new_direction = !moved || (count == 0);
            if (generate_new_direction) {
                last_direction = direction;
                direction = RandomChoice(4);
                while (last_direction == direction) direction = RandomChoice(4);
                count = RandomInt(3, 6);
                generate_new_direction = false;
            }
        }

        PlaceRoad(world, last, current_x, current_y);
        last = &world->grid[current_x][current_y];

        generating = current_x < GRID_WIDTH - 1;
    }

    u32 x = last->x;
    u32 y = last->y;

    if (y > 0) {
        world->grid[x][y - 1].occupied = true;
        world->grid[x][y - 1].house = true;

        if (RandomChoice(10) > 5) {
            u32 offset = 1;
            if (world->grid[x - offset][y - 1].road) offset++;
            if (!world->grid[x - offset][y - 1].road) {
                world->grid[x - offset][y - 1].occupied = true;
                world->grid[x - offset][y - 1].house = true;
            }
        }
    }

    if (y < GRID_HEIGHT - 1) {
        world->grid[x][y + 1].occupied = true;
        world->grid[x][y + 1].house = true;

        if (RandomChoice(10) > 5) {
            u32 offset = 1;
            if (world->grid[x - offset][y + 1].road) offset++;
            if (!world->grid[x - offset][y + 1].road) {
                world->grid[x - offset][y + 1].occupied = true;
                world->grid[x - offset][y + 1].house = true;
            }
        }
    }

}
