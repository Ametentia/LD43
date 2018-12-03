internal void CreateEnemy(Play_State *state, Enemy_Type type) {
    Enemy *result = cast(Enemy *) malloc(sizeof(Enemy));

    f32 offset_y = RandomUnilateral() * 60;
    v2 position = {
        state->world.path->x * 60.0f,
        offset_y + state->world.path->y * 60.0f
    };

    result->type = type;
    result->animation = CreateAnimation(assets.enemy_textures + type,
            assets.enemy_textures[type].getSize().y / 20,
            7, position, 0.05f, RandomChoice(14));

    // The first square in the path
    result->next_square = state->world.path;
    result->direction = v2(1, 0);

    // Not delayed when created
    result->delayed = false;
    result->delay_timer = 0;

    result->health = 100 + (100 * (state->total_enemies_spawned / 45));

    // Not slow by default
    result->speed_mod = 1;
    result->slowed = false;
    result->slow_time = 0;

    switch (type) {
        case EnemyType_Normal: {
            result->fire_mod = 1;
            result->electric_mod = 1;
            result->explosion_mod = 1;
            result->ice_mod = 1;
        }
        break;
        case EnemyType_Fire: {
            result->fire_mod = 0.2f;
            result->ice_mod = 1.5f;
            result->electric_mod = 0.7f;
            result->explosion_mod = 1.2f;
        }
        break;
        case EnemyType_Electric: {
            result->fire_mod = 1.5f;
            result->ice_mod = 0.5f;
            result->electric_mod = 0.2f;
            result->explosion_mod = 1;
        }
        break;
        case EnemyType_Explosive: {
            result->fire_mod = 1;
            result->ice_mod = 0.8f;
            result->electric_mod = 1.5f;
            result->explosion_mod = 0.4f;
        }
        break;
        case EnemyType_Ice: {
            result->fire_mod = 1.5f;
            result->ice_mod = 0.2f;
            result->electric_mod = 0.6f;
            result->explosion_mod = 1.1f;
        }
        break;
    }

    // Link into the enemy list
    result->prev = 0;
    result->next = state->enemies;
    if (state->enemies) state->enemies->prev = result;
    state->enemies = result;

    state->enemies_spawned++;
    state->total_enemies_spawned++;
}

internal void RemoveEnemy(Play_State *state, Enemy *enemy) {
    if (state->enemies == enemy && !enemy->next) {
        state->enemies = 0;
    }
    else if (state->enemies == enemy) {
        state->enemies = enemy->next;
    }

    // Unlink from the list
    if (enemy->next) enemy->next->prev = enemy->prev;
    if (enemy->prev) enemy->prev->next = enemy->next;

    state->enemies_spawned--;

    free(enemy);
}

internal bool DamageEnemy(Tower *tower, Enemy *enemy) {
    u32 flags = tower->tower_type_bits;
    f32 modification = 0;
    if (flags & TowerTypeFlags_Fire) {
        modification += enemy->fire_mod;
        // modification = Max(enemy->fire_mod, modification);
    }

    if (flags & TowerTypeFlags_Electric) {
        modification += enemy->electric_mod;
        // modification = Max(enemy->electric_mod, modification);
    }

    if (flags & TowerTypeFlags_Explosion) {
        modification += enemy->explosion_mod;
        // modification = Max(enemy->explosion_mod, modification);
    }

    if (flags & TowerTypeFlags_Ice) {
        modification += enemy->ice_mod;
    }

    // Means it is only a ice tower so don't do damage
    u32 damage = cast(u32) (tower->damage_output * modification);
    damage = Max(damage, 1); // Make towers do at least one damage
    enemy->health -= damage;


    // Ice tower so slow
    if (tower->type == TowerType_Ice) { // || (flags & TowerTypeFlags_Ice) == TowerTypeFlags_Ice) {
        enemy->slowed = true;
        enemy->speed_mod = tower->speed_modifier;
        enemy->slow_time = 1.5f;
    }

    return enemy->health <= 0;
}

internal bool CheckPathSquare(Enemy *enemy) {
    v2 offset = {
        enemy->direction.x * cast(f32) RandomInt(5, 55),
        enemy->direction.y * cast(f32) RandomInt(5, 55)
    };

    v2 min = {
        (60.0f * enemy->next_square->x),
        (60.0f * enemy->next_square->y)
    };
    v2 max = {
        (60.0f * enemy->next_square->x) + 60,
        (60.0f * enemy->next_square->y) + 60
    };

    if (enemy->direction.x > 0) {
        min.x += Abs(offset.x);
    }
    else if (enemy->direction.x < 0) {
        max.x -= Abs(offset.x);
    }
    else if (enemy->direction.y > 0) {
        min.y += Abs(offset.y);
    }
    else if (enemy->direction.y < 0) {
        max.y -= Abs(offset.y);
    }

#if 0
    sf::RectangleShape shape(max - min);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineColor(sf::Color::Blue);
    shape.setOutlineThickness(-2);
    shape.setPosition(enemy->next_square->x * 60.0f, enemy->next_square->y * 60.0f);
    window.draw(shape);
#endif

    v2 position = enemy->animation.position;

    if (position.x > min.x && position.x < max.x) {
        if (position.y > min.y && position.y < max.y) {
            Grid_Square *old_square = enemy->next_square;
            enemy->next_square = enemy->next_square->next;
            if (!enemy->next_square) {
                // @Todo(James): Make damage
                return true;
            }
            else {
                v2 old_pos = { 60.0f * old_square->x, 60.0f * old_square->y };
                v2 new_pos = { 60.0f * enemy->next_square->x, 60.0f * enemy->next_square->y };
                enemy->direction = Normalise(new_pos - old_pos);
            }
        }
    }

    return false;
}

#define ENEMY_SPEED 240
internal void UpdateEnemies(Play_State *state, f32 dt) {
    Enemy *enemy = state->enemies;
    while (enemy) {
        if (!enemy->delayed) {
            v2 next_pos = {
                30 + (60.0f * enemy->next_square->x),
                30 + (60.0f * enemy->next_square->y)
            };

            v2 dist = enemy->speed_mod * ENEMY_SPEED * dt * enemy->direction;
            // Normalise(next_pos - enemy->animation.position);
            enemy->animation.position += dist;

            if (enemy->slowed) {
                enemy->slow_time -= dt;
                if (enemy->slow_time <= 0) {
                    enemy->speed_mod = 1;
                    enemy->slowed = false;
                }
            }
        }
        else {
            enemy->delay_timer -= dt;
            if (enemy->delay_timer <= 0) enemy->delayed = false;
        }

        bool removed = CheckPathSquare(enemy);
        if (!removed) {
            f32 actual_dt = dt;

            DrawAnimation(&enemy->animation, actual_dt);
            if (enemy->slowed) {
                sf::RectangleShape slowed_sprite;
                slowed_sprite.setSize(v2(enemy->animation.width, enemy->animation.height));
                slowed_sprite.setOrigin(v2(enemy->animation.width / 2,
                            enemy->animation.height / 2));
                slowed_sprite.setPosition(enemy->animation.position);
                slowed_sprite.setTexture(&assets.slowed_texture);

                window.draw(slowed_sprite);
            }
        }

        if (removed) {
            Enemy *old_enemy = enemy;
            enemy = enemy->next;
            state->health -= 1;
            if (state->health <= 0) state->alive = false;
            RemoveEnemy(state, old_enemy);
        }
        else {
            enemy = enemy->next;
        }
    }
}
