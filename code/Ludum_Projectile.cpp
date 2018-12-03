internal void AddProjectile(Play_State *state, v2 position,
        v2 direction, Tower *tower)
{
    Projectile *result = cast(Projectile *) malloc(sizeof(Projectile));

    result->parent_tower = tower;
    if (tower->type == TowerType_Electric) {
        result->is_sprite = false;
        result->animation = CreateAnimation(assets.projectile_textures +
                tower->type, 1, 4, position, 0.06f, RandomChoice(4));
        result->angular_vel = 0;
    }
    else {
        result->is_sprite = true;
        result->texture = assets.projectile_textures + tower->type;
        result->angular_vel = 30;
    }

    // Assumes normalised
    result->direction = direction;
    result->position = position;

    result->speed = 250;
    result->angle = 0;

    result->next = state->projectiles;
    if (result->next) result->next->prev = result;
    result->prev = 0;
    state->projectiles = result;
}

inline void AddExplosion(Play_State *state, v2 position, Tower_Type type) {
    state->next_explosion = (state->next_explosion + 1) % 25;
    Animation *next = state->explosions + state->next_explosion;

    if (next->valid) return;

    sf::Texture *texture = 0;
    if (type == TowerType_Ice) texture = &assets.ice_explosion;
    else texture = &assets.explosion;

    *next = CreateAnimation(texture, 3, 5, position,
            0.1f, RandomChoice(7));
}

internal void RemoveProjectile(Play_State *state, Projectile *projectile, Enemy *enemy = 0) {
    if (state->projectiles == projectile && projectile->next) {
        state->projectiles = projectile->next;
    }
    else if (state->projectiles == projectile) {
        state->projectiles = 0;
    }

    if (projectile->next) projectile->next->prev = projectile->prev;
    if (projectile->prev) projectile->prev->next = projectile->next;

    projectile->parent_tower->projectile_count--;

    if (projectile->parent_tower->type == TowerType_Explosion
            || projectile->parent_tower->type == TowerType_Ice) {
        v2 offset = { 30 * RandomUnilateral(), 30 * RandomUnilateral() };
        v2 position = projectile->position;
        if (enemy) position = enemy->animation.position + v2(30, 30);
        AddExplosion(state, projectile->position + offset,
                projectile->parent_tower->type);
    }

    free(projectile);
}

internal bool CheckEnemiesToDamage(Play_State *state, Projectile *projectile,
        Grid_Square *square, Enemy **hit)
{
    u32 hit_count = 0;
    bool result = false;
    WhileList(Enemy, state->enemies) {
        bool killed = false;
        if (item->next_square && item->next_square->prev == square) {
            // Do damage
            killed = DamageEnemy(projectile->parent_tower, item);
            result = true;
            if (!*hit) *hit = item;
            hit_count++;
        }

        if (killed) {
            Enemy *old = item;
            item = item->next;
            switch (old->type) {
                case EnemyType_Normal:    { state->normal_killed++; } break;
                case EnemyType_Fire:      { state->fire_killed++; } break;
                case EnemyType_Electric:  { state->electric_killed++; } break;
                case EnemyType_Explosive: { state->explosive_killed++; } break;
                case EnemyType_Ice:       { state->ice_killed++; } break;
            }

            RemoveEnemy(state, old);
            state->money += state->money_per_enemy;

            continue;
        }


        if (hit_count >= 10) break;
        item = item->next;
    }

    return result;
}


internal void UpdateProjectiles(Play_State *state, f32 dt) {
    Enemy *hit = 0;
    WhileList(Projectile, state->projectiles) {
        bool destroyed = false;
        item->position += (dt * item->speed * item->direction);
        item->angle += (dt * item->angular_vel);
        if (item->angle > 360) item->angle = 0;
        // Check it is still inside the range of the tower otherwise destroy
        destroyed = !InsideTowerRaidus(item->parent_tower, item->position);
        if (!destroyed) {
            // Still inside, check if it is over a road square
            s32 grid_x = cast(s32) (item->position.x / 60.0f);
            s32 grid_y = cast(s32) (item->position.y / 60.0f);

            // Out of the map
            destroyed = grid_x < 0 || grid_y < 0 || grid_x > GRID_WIDTH || grid_y >
                GRID_HEIGHT;

            if (!destroyed) {
                Grid_Square *square = &state->world.grid[grid_x][grid_y];
                if (square->road) {
                    destroyed = CheckEnemiesToDamage(state, item, square, &hit);
                }
            }
        }

        if (destroyed) {
            Projectile *old = item;
            item = item->next;
            RemoveProjectile(state, old, hit);
            continue;
        }

        if (item->is_sprite) {
            sf::Sprite sprite(*item->texture);
            sprite.setOrigin(0.5f * v2(item->texture->getSize()));
            sprite.setPosition(item->position);
            sprite.setRotation(item->angle);
            window.draw(sprite);
        }
        else {
            item->animation.position = item->position;
            DrawAnimation(&item->animation, dt);
        }

        item = item->next;
    }
}
