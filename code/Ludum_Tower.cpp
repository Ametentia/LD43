internal void CombineTowers(Tower *tower, Tower *combined) {
    f32 total_damage = tower->damage_output + combined->damage_output;
    tower->damage_output = total_damage;
    if (tower->damage_output < 600) {
        tower->damage_output += (total_damage * RandomInt(2, 6));
    }
    else if (tower->type == TowerType_Explosion) {
        tower->damage_output *= (3 * RandomUnilateral());
    }
    // 4 * combined->damage_output;
  //  tower->enemy_limit += 5;
    //tower->tower_type_bits |= combined->tower_type_bits;

    tower->speed_modifier = Min(combined->speed_modifier, tower->speed_modifier);
}

internal void AddTower(Overworld *world, u32 x, u32 y, Tower_Type type) {
    Tower *tower = cast(Tower *) malloc(sizeof(Tower));
    tower->x = x;
    tower->y = y;

    world->grid[x][y].destroyed = false;
    world->grid[x][y].tower = tower;
    tower->destroyed = 0;

    tower->type = type;
    switch (type) {
        case TowerType_Fire:      { tower->damage_output = 10; } break;
        case TowerType_Electric:  { tower->damage_output = 12; } break;
        case TowerType_Explosion: { tower->damage_output = 18; } break;
        case TowerType_Ice:       { tower->damage_output = 8; } break;
    }
    tower->tower_type_bits = TowerBitsFromType(type);

    if (type == TowerType_Ice) {
        tower->speed_modifier = 0.5f;
    }
    else {
        tower->speed_modifier = 1;
    }

    tower->projectile_count = 0;

    world->active_tower_count++;
    tower->enemy_limit = 5;
    tower->next = world->towers;
    if (world->towers) world->towers->prev = tower;
    world->towers = tower;

    ForList(Tower, tower->next) {
        if (item->destroyed) continue;

        s32 x = Abs(cast(s32) (tower->x - item->x));
        s32 y = Abs(cast(s32) (tower->y - item->y));

        //v2 position = { 30 + (60.0f * item->x), 30 + (60.0f * item->y) };
        if (x <= 3 && y <= 3) {
            if (item->type >= tower->type) continue;
            CombineTowers(tower, item);
            item->destroyed = true;
            world->active_tower_count--;
            // @Hack: To prevent placement from seeing this as a vaild tower
            world->grid[item->x][item->y].destroyed = 1;
        }
    }
}

internal void UpdateTowers(Overworld *world, Play_State *state, f32 dt) {
    sf::RectangleShape tower_display;
    tower_display.setSize(v2(60, 120));
    ForList(Tower, world->towers) {
        if (!item->destroyed) {
            tower_display.setTexture(assets.tower_textures + item->type);

            // Go through all of the enemies and see if they are damaged
            // @Speed: This is really slow
            u32 enemies_attacked = 0;
            Enemy *enemy = state->enemies;
            while (enemy) {
                bool killed = false;
                if (InsideTowerRaidus(item, enemy->animation.position)) {
                    v2 offset = { 10 * RandomUnilateral(), 10 * RandomUnilateral() };
                    v2 position = { item->x * 60.0f + 30, item->y * 60.0f };
                    v2 direction = Normalise(enemy->animation.position - position + offset);

                    /*
                    if (item->type != TowerType_Ice || item->tower_type_bits !=
                            TowerTypeFlags_Ice) */
                    {
                        // This can be upped by upgrades to give the illusion of damage output
                        if (item->projectile_count < 40) {
                            AddProjectile(state, position, direction, item);
                            item->projectile_count++;
                        }
                    }
                    /*
                    else {
                        DamageEnemy(item, enemy);
                    }
                    */

                    enemies_attacked++;
                }

#if 0
                // Can only attack a certain amount of enemies per turn
                if (enemies_attacked >= item->enemy_limit) {
                    break;
                }
#endif
                enemy = enemy->next;
            }
        }
        else {
            tower_display.setTexture(&assets.destroyed_tower_texture);
        }

        tower_display.setPosition(item->x * 60.0f, 60.0f * ((cast(s32) (item->y)) - 1));
        window.draw(tower_display);
    }

    u32 explosions_animated = 0;
    for (u32 i = 0; i < 25; ++i) {
        Animation *next = state->explosions + i;
        if (!next->valid) continue;

        explosions_animated++;
        if (DrawAnimation(next, dt)) next->valid = false;
    }
}
