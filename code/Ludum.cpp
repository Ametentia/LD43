internal void PushState(Game_State *state, State *new_state) {
    State *old_top = state->current_state;

    new_state->next = old_top;
    if (old_top) old_top->prev = new_state;

    state->current_state = new_state;
}

internal State *PopState(Game_State *state) {
    State *old_top = state->current_state;
    Assert(old_top);

    state->current_state = old_top->next;

    return old_top;
}

internal State *AddState(Game_State *state, State *new_state) {
    State *old = PopState(state);
    PushState(state, new_state);

    return old;
}

internal void CreatePlayState(Game_State *state) {
    State *new_state = cast(State *) malloc(sizeof(State));
    new_state->type = StateType_Play;
    Play_State *play_state = &new_state->play;
    *play_state = {};

    play_state->alive = true;
    play_state->health = 100;
    play_state->money = 500;
    play_state->time_to_spawn = 1.5f;
    play_state->current_tower_type = TowerType_Fire;

    GenerateWorld(&play_state->world);

    State *menu = AddState(state, new_state);
    free(menu); // Not needed anymore
}

internal void CreateGameOverState(Game_State *state, Play_State *play) {
    State *new_state = cast(State *) malloc(sizeof(State));

    new_state->type = StateType_GameOver;
    Game_Over_State *game_over = &new_state->game_over;
    *game_over = {};
    game_over->normal_killed    = play->normal_killed;
    game_over->fire_killed      = play->fire_killed;
    game_over->electric_killed  = play->electric_killed;
    game_over->explosive_killed = play->explosive_killed;
    game_over->ice_killed       = play->ice_killed;
    game_over->time_survived = play->time;


    u32 minutes = (cast(u32) game_over->time_survived) / 60;
    game_over->total_score = (100 * minutes) +
        play->normal_killed + play->fire_killed + play->electric_killed +
        play->explosive_killed + play->ice_killed;


    game_over->character_size = 60;
    game_over->smaller = true;
    game_over->high_score = game_over->total_score > state->high_score;

    sf::Text text("HIGH SCORE!", assets.ui_font, 60);
    game_over->origin = v2(text.getLocalBounds().width / 2.0f,
            text.getLocalBounds().height / 2.0f);

    State *old = AddState(state, new_state);
    free(old);
}

internal void GameUpdateRenderMenuState(Game_State *state, Menu_State *menu,
        Game_Input *input, f32 dt)
{
    sf::RectangleShape shape(v2(1920, 1080));
    shape.setPosition(0, 0);
    shape.setTexture(&assets.title_screen);
    window.draw(shape);

    v2 play_size = v2(assets.play_button[0].getSize());
    v2 exit_size = v2(assets.exit_button[0].getSize());

    v2 play_position = (v2(1920, 1550) - play_size) / 2.0f;
    v2 exit_position = (v2(1920, 1920) - exit_size) / 2.0f;

    bool play_hover = false, exit_hover = false;
    v2 mouse = input->mouse_position;
    if (mouse.x > play_position.x && mouse.x < play_position.x + play_size.x) {
        if (mouse.y > play_position.y && mouse.y < play_position.y + play_size.y) {
            play_hover = true;
            if (GameButtonJustPressed(input->left_mouse)) {
                CreatePlayState(state);
            }
        }
    }

    if (mouse.x > exit_position.x && mouse.x < exit_position.x + exit_size.x) {
        if (mouse.y > exit_position.y && mouse.y < exit_position.y + exit_size.y) {
            exit_hover = true;
            if (GameButtonJustPressed(input->left_mouse)) {
                input->requested_quit = true;
            }
        }
    }

    f32 size_offset = 0;
    if (mouse.x > 25 && mouse.x < 145) {
        if (mouse.y > (1080 - 145) && mouse.y < (1080 - 25)) {
            if (GameButtonJustPressed(input->left_mouse)) {
                state->fullscreen = !state->fullscreen;
                if (state->fullscreen) {
                    window.create(sf::VideoMode::getDesktopMode(), "The Cost of Defence",
                            sf::Style::Fullscreen);
                }
                else {
                    window.create(sf::VideoMode(1280, 720), "The Cost of Defence",
                            sf::Style::Close);
                }

                window.setFramerateLimit(60);
                sf::View main_view(v2(960, 540), v2(1920, 1080));
                window.setView(main_view);
            }
        }
    }



    sf::RectangleShape play(play_size);
    play.setPosition(play_position);
    play.setTexture(&assets.play_button[play_hover ? 1 : 0]);
    window.draw(play);

    sf::RectangleShape exit(exit_size);
    exit.setPosition(exit_position);
    exit.setTexture(&assets.exit_button[exit_hover ? 1 : 0]);
    window.draw(exit);

    sf::RectangleShape fullscreen_button(v2(120, 120));
    fullscreen_button.setPosition(25, 1080 - 145);
    fullscreen_button.setTexture(state->fullscreen ? &assets.minimise_button
            : &assets.fullscreen_button);
    window.draw(fullscreen_button);
}


internal void GameUpdateRenderGameOverState(Game_State *game_state, Game_Over_State *game_over,
        Game_Input *input, f32 dt)
{
    if (GameButtonPressed(input->refresh)) {
        CreatePlayState(game_state);
        return;
    }
    else {
        for (u32 i = 0; i < ArrayCount(input->buttons); ++i) {
            if (GameButtonPressed(input->buttons[i])) {
                // Go back to the main menu
                State *new_state = cast(State *) malloc(sizeof(State));
                new_state->type = StateType_Menu;

                if (game_over->total_score > game_state->high_score) {
                    game_state->high_score = game_over->total_score;
                }

                State *old = AddState(game_state, new_state);
                free(old);

                return;
            }
        }
    }

    window.clear(v4(25, 25, 25));

    sf::Text text;
    text.setFont(assets.ui_font);
    text.setCharacterSize(45);

    text.setPosition(100, 100);

    char buffer[256];

    u32 minutes = (cast(u32) game_over->time_survived) / 60;
    u32 seconds = (cast(u32) game_over->time_survived) % 60;

    if (minutes == 0) {
        snprintf(buffer, sizeof(buffer), "You Survived for: %us\n", seconds);
    }
    else {
        snprintf(buffer, sizeof(buffer), "You Survived for: %um %us\n", minutes, seconds);
    }

    text.setString(buffer);
    window.draw(text);

    text.move(0, 60);

    snprintf(buffer, sizeof(buffer), "Enemies Killed:");
    text.setString(buffer);
    window.draw(text);

    text.move(0, 60);
    snprintf(buffer, sizeof(buffer), "- Normal:    %u\n", game_over->normal_killed);
    text.setString(buffer);
    window.draw(text);

    text.move(0, 60);
    snprintf(buffer, sizeof(buffer), "- Fire:      %u\n", game_over->fire_killed);
    text.setString(buffer);
    window.draw(text);

    text.move(0, 60);
    snprintf(buffer, sizeof(buffer), "- Electric:  %u\n", game_over->electric_killed);
    text.setString(buffer);
    window.draw(text);

    text.move(0, 60);
    snprintf(buffer, sizeof(buffer), "- Ice:       %u\n", game_over->ice_killed);
    text.setString(buffer);
    window.draw(text);

    text.move(0, 60);
    snprintf(buffer, sizeof(buffer), "- Explosive: %u\n", game_over->explosive_killed);
    text.setString(buffer);
    window.draw(text);

    text.move(0, 60);
    memset(buffer, '-', 45);
    buffer[45] = 0;
    text.setString(buffer);
    window.draw(text);

    text.move(0, 60);
    snprintf(buffer, sizeof(buffer), "Total Score: %u\n", game_over->total_score);
    text.setString(buffer);
    window.draw(text);

    if (game_over->high_score) {
        if (game_over->smaller) {
            game_over->character_size -= 1;
            if (game_over->character_size == 30) game_over->smaller = false;
        }
        else {
            game_over->character_size += 1;
            if (game_over->character_size == 60) game_over->smaller = true;
        }

        text.setCharacterSize(game_over->character_size);
        text.setOrigin(game_over->origin);
        text.setString("HIGH SCORE!");
        text.setPosition(1620, 780);
        text.setRotation(-45);
        text.setFillColor(sf::Color::Red);
        window.draw(text);
    }

}

internal void GameUpdateRenderPlayState(Game_State *game_state, Play_State *state,
        Game_Input *input, f32 dt)
{
    Overworld *world = &state->world;

    state->time += dt;

    if (!state->alive) {
        CreateGameOverState(game_state, state);
        return;
    }

    if (GameButtonJustPressed(input->one)) {
        state->current_tower_type = TowerType_Fire;
    }
    else if (GameButtonJustPressed(input->two)) {
        state->current_tower_type = TowerType_Electric;
    }
    else if (GameButtonJustPressed(input->three)) {
        state->current_tower_type = TowerType_Ice;
    }
    else if (GameButtonJustPressed(input->four)) {
        state->current_tower_type = TowerType_Explosion;
    }

    // Drawing the map
    sf::RectangleShape tile(v2(60, 60));
    for (u32 i = 0; i < GRID_WIDTH; ++i) {
        for (u32 j = 0; j < GRID_HEIGHT; ++j) {
            Grid_Square *current = &world->grid[i][j];
            tile.setPosition(GridSize(i, j));

            tile.setTexture(current->road ?
                assets.dirt_textures + current->texture_index :
                assets.grass_textures + current->texture_index);

            window.draw(tile);

            if (current->house) {
                tile.setTexture(&assets.house);
                window.draw(tile);
            }
        }
    }

    // Spawning enemies
    state->spawn_timer += dt;
    if (state->spawn_timer >= state->time_to_spawn) {
        CreateEnemy(state, cast(Enemy_Type) RandomChoice(EnemyType_Count));
        state->spawn_timer = 0;
    }

    if (state->time > state->last_time_check + 0.8f) {
        state->time_to_spawn = Max(state->time_to_spawn - 0.03f, 0.05f);
        state->money_per_enemy = Max(state->time_to_spawn * 10, 0.2f);
        state->last_time_check = state->time;
    }


    bool draw_tower_info = false;
    // Trying to place a tower
    s32 grid_x = cast(s32) (input->mouse_position.x / 60);
    s32 grid_y = cast(s32) (input->mouse_position.y / 60);

    // Make sure within the screen bounds
    if (grid_x >= 0 && grid_x < GRID_WIDTH && grid_y >= 0 && grid_y < GRID_HEIGHT) {
        Grid_Square *selected = &world->grid[grid_x][grid_y];

        if (selected->tower) {
            draw_tower_info = true;
        }
        else {

            sf::CircleShape radius(250);
            radius.setPosition(input->mouse_position);
            radius.setOrigin(250, 250);
            radius.setOutlineColor(sf::Color::Green);
            radius.setFillColor(sf::Color::Transparent);
            radius.setOutlineThickness(2);

            sf::RectangleShape selected_square;
            selected_square.setSize(GridSize(1, 2));
            selected_square.setPosition(GridSize(grid_x, grid_y) - GridSize(0, 1));
            selected_square.setTexture(assets.tower_textures + state->current_tower_type);

            bool allowed = true;
            if (!selected->occupied) {
                allowed = (state->current_tower_type == TowerType_Fire);
                for (u32 i = Max(cast(s32) selected->x - 4, 0);
                        i < Min(selected->x + 4, GRID_WIDTH); i++)
                {
                    for (u32 j = Max(cast(s32) selected->y - 4, 0);
                            j < Min(selected->y + 4, GRID_HEIGHT); j++)
                    {
                        Grid_Square *square = &world->grid[i][j];
                        if (square->tower) {
                            if (square->tower->destroyed) continue;
                            Tower_Type found = square->tower->type;
                            Tower_Type current = state->current_tower_type;

                            allowed = allowed ||
                                (found == TowerType_Fire
                                    && current == TowerType_Electric) ||
                                (found == TowerType_Electric
                                 && current == TowerType_Ice) ||
                                (found == TowerType_Ice &&
                                 current == TowerType_Explosion);

                            break;
                        }
                    }

                    if (allowed) break;
                }
            }
            else if (selected->occupied) { allowed = false; }

            if (allowed) {
                if (GameButtonJustPressed(input->left_mouse)) {
                    u32 cost = tower_costs[state->current_tower_type];
                    if (state->money >= cost) {
                        selected->occupied = true;
                        state->money -= cost;
                        AddTower(world, grid_x, grid_y, state->current_tower_type);
                    }
                }
            }
            else {
                radius.setOutlineColor(sf::Color::Red);
                selected_square.setFillColor(v4(255, 0, 0, 128));
            }

            window.draw(radius);
            window.draw(selected_square);
        }
    }

    if (GameButtonJustPressed(input->next_tower)) {
        state->current_tower_type = cast(Tower_Type)
            ((state->current_tower_type + 1) % TowerType_Count);
    }
    else if (GameButtonJustPressed(input->prev_tower)) {
        s32 next = cast(s32) (state->current_tower_type - 1);
        if (next < 0) state->current_tower_type = TowerType_Ice;
        else state->current_tower_type = cast(Tower_Type) next;
    }

    // Update entities
    UpdateEnemies(state, dt);
    UpdateTowers(world, state, dt);
    UpdateProjectiles(state, dt);

    sf::RectangleShape bar(v2(assets.tower_bar.getSize()));
    bar.setPosition(v2((1920 - bar.getSize().x) / 2.0f, 1080 - bar.getSize().y));
    bar.setTexture(&assets.tower_bar);
    window.draw(bar);

    {
        bar.setTexture(0);
        bar.setPosition(0, 0);

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Health: %d\n", state->health);
        sf::Text health_text(buffer, assets.ui_font, 45);

        if (state->health > 66) {
            health_text.setFillColor(sf::Color::Green);
        }
        else if (state->health > 33) {
            health_text.setFillColor(v4(255, 102, 0));
        }
        else {
            health_text.setFillColor(sf::Color::Red);
        }

        snprintf(buffer, sizeof(buffer), "Money: %d\n", cast(u32) state->money);
        sf::Text money_text(buffer, assets.ui_font, 45);
        money_text.setFillColor(sf::Color::Yellow);
        bar.setFillColor(v4(25, 25, 25));
        bar.setOutlineColor(v4(120, 120, 120));
        bar.setOutlineThickness(-4);
        bar.setSize(v2(Max(health_text.getLocalBounds().width,
                    money_text.getLocalBounds().width) + 60, 130));

        window.draw(bar);

        u32 x_pos = Min(1920 - health_text.getLocalBounds().width - 30,
                1920 - money_text.getLocalBounds().width - 30);

        health_text.setPosition(25, 15);
        window.draw(health_text);

        money_text.setPosition(25, 55);
        window.draw(money_text);
    }

    if (draw_tower_info) {
        Grid_Square *selected = &world->grid[grid_x][grid_y];
        Tower *tower = selected->tower;
        sf::RectangleShape info(v2(250, 300));
        info.setPosition(input->mouse_position);
        info.setOutlineColor(v4(120, 120, 120));
        info.setFillColor(v4(25, 25, 25));
        info.setOutlineThickness(-6);
        window.draw(info);

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Tower Info:");
        sf::Text info_text(buffer, assets.ui_font, 20);
        info_text.setPosition(input->mouse_position + v2(25, 12));
        window.draw(info_text);



        if (!tower->destroyed) {
            snprintf(buffer, sizeof(buffer), "Damage: %d", cast(u32) tower->damage_output);
            info_text.setString(buffer);
            info_text.move(0, 30);
            window.draw(info_text);

            snprintf(buffer, sizeof(buffer), "Speed Modifier: %.3f", tower->speed_modifier);
            info_text.setString(buffer);
            info_text.move(0, 30);
            window.draw(info_text);

            snprintf(buffer, sizeof(buffer), "Damage Types:");
            info_text.setString(buffer);
            info_text.move(0, 30);
            window.draw(info_text);
            for (u32 i = 0; i < TowerType_Count; ++i) {
                Tower_Type_Flags flags = TowerBitsFromType(cast(Tower_Type) i);
                if (tower->tower_type_bits & flags) {
                    char *name = TowerTypeNameFromType(cast(Tower_Type) i);
                    snprintf(buffer, sizeof(buffer), "- %s", name);
                    info_text.setString(buffer);
                    info_text.move(0, 30);
                    window.draw(info_text);
                }
            }
        }
        else {
            snprintf(buffer, sizeof(buffer), "DESTROYED!");
            info_text.rotate(45);
            info_text.move(45, 70);
            info_text.setCharacterSize(30);
            info_text.setString(buffer);

            window.draw(info_text);
        }
    }

    }

internal void GameUpdateRender(Game_State *state, Game_Input *input) {
    f32 dt = state->dt;

    switch (state->current_state->type) {
        case StateType_Menu: {
            GameUpdateRenderMenuState(state, &state->current_state->menu,
                    input, dt);
        }
        break;
        case StateType_Play: {
            GameUpdateRenderPlayState(state, &state->current_state->play,
                    input, dt);
        }
        break;
        case StateType_GameOver: {
            GameUpdateRenderGameOverState(state, &state->current_state->game_over,
                    input, dt);
        }
        break;
    }
}
