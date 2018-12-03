#if !defined(LUDUM_H_)
#define LUDUM_H_

struct Animation {
    sf::Texture *texture;
    v2 position;
    u32 rows, columns;
    u32 width, height;
    u32 total_frames;

    bool valid;
    bool playing;

    u32 current_frame;
    f32 time_per_frame;

    f32 accumulator;
};


enum Tower_Type {
    TowerType_Fire = 0,
    TowerType_Electric,
    TowerType_Ice,
    TowerType_Explosion,

    TowerType_Count
};

enum Tower_Type_Flags {
    TowerTypeFlags_Fire = 0x1,
    TowerTypeFlags_Electric = 0x2,
    TowerTypeFlags_Ice = 0x04,
    TowerTypeFlags_Explosion = 0x8
};

/*
   Tower Types:
    - Normal: Standard damage
    - Fire: Fire damage, better against enemies with wood armour
    - Electric: Electric damage, better against enemies with metal armour
    - Explosion: Explosive damage, AoE damage
    - Ice: Ice damage, slows enemies
 */

struct Tower {
    Tower_Type type; // Main type for rendering
    f32 damage_output;

    bool destroyed;

    // Grid position
    u32 x, y;

    u32 enemy_limit;

    u32 tower_type_bits; // For upgrades

    f32 speed_modifier;

    u32 projectile_count;

    Tower *next;
    Tower *prev;
};

struct Projectile {
    bool is_sprite;
    sf::Texture *texture;
    Animation animation;

    v2 position;

    f32 speed;
    f32 angular_vel;
    f32 angle;
    v2 direction;

    // A back pointer here is really bad but towers are never removed so I guess it doesn't
    // really matter
    Tower *parent_tower;

    Projectile *next;
    Projectile *prev;
};


struct Grid_Square {
    bool occupied = false;
    bool road = false;
    u32 destroyed;

    bool house = false;
    // If the square is occupied by a tower then this can be used
    Tower *tower = 0;

    u32 texture_index = 0;

    // Grid position
    u32 x = 0, y = 0;

    Grid_Square *next = 0;
    Grid_Square *prev = 0;
};

#define GRID_WIDTH 32
#define GRID_HEIGHT 18
struct Overworld {
    // We want to be able in index this using x, y
    Grid_Square grid[GRID_WIDTH][GRID_HEIGHT];

    // Need to know the path seperately so the enemies can follow
    Grid_Square *path;

    u32 active_tower_count;
    Tower *towers;
};

enum Enemy_Type {
    EnemyType_Normal,
    EnemyType_Fire,
    EnemyType_Electric,
    EnemyType_Explosive,
    EnemyType_Ice,

    EnemyType_Count
};

struct Enemy {
    Enemy_Type type;
    Animation animation;

    // Which path they should be going to next
    Grid_Square *next_square;
    v2 direction;

    // If they encounter a villager to slow them down
    bool delayed;
    f32 delay_timer;

    s32 health;

    // 1 if normal speed, slowed by ice towers
    f32 speed_mod;
    f32 slow_time;
    bool slowed;

    // Modifiers on the amount of damage done by towers
    f32 fire_mod;
    f32 electric_mod;
    f32 ice_mod;
    f32 explosion_mod;

    // Linked list for easy iteration
    Enemy *next;
    Enemy *prev;
};

struct Assets {
    sf::Font ui_font;

    sf::Texture fullscreen_button;
    sf::Texture minimise_button;

    sf::Texture tower_bar;
    sf::Texture house;

    sf::Texture explosion, ice_explosion;
    sf::Texture projectile_textures[TowerType_Count];

    sf::Texture destroyed_tower_texture;
    sf::Texture tower_textures[TowerType_Count];

    sf::Texture gradient;
    sf::Texture grass_textures[7];
    sf::Texture dirt_textures[3];

    sf::Texture slowed_texture;
    sf::Texture enemy_textures[EnemyType_Count];

    sf::Texture title_screen;
    sf::Texture exit_button[2];
    sf::Texture play_button[2];
};

global u32 tower_costs[TowerType_Count] = {
    200, 300, 400, 500
};

struct Play_State {
    Overworld world;

    Tower_Type current_tower_type;

    bool alive;
    s32 health;
    f32 money;
    f32 money_per_enemy;

    f32 last_time_check = 0;

    Enemy *enemies;
    u32 enemies_spawned;
    u32 total_enemies_spawned;

    u32 normal_killed;
    u32 fire_killed;
    u32 electric_killed;
    u32 explosive_killed;
    u32 ice_killed;

    Projectile *projectiles;

    Animation explosions[25];
    u32 next_explosion;

    f32 time = 0;
    f32 time_to_spawn;
    f32 spawn_timer;
};

struct Menu_State {
};

struct Game_Over_State {
    u32 normal_killed = 0;
    u32 fire_killed = 0;
    u32 electric_killed = 0;
    u32 explosive_killed = 0;
    u32 ice_killed = 0;
    f32 time_survived = 0;

    u32 total_score = 0;

    bool high_score = false;
    u32 character_size = 60;
    bool smaller = true;
    v2 origin = {};
};


enum State_Type {
    StateType_Menu,
    StateType_Play,
    StateType_GameOver,
};

struct State {
    State_Type type;
    union {
        Play_State play;
        Menu_State menu;
        Game_Over_State game_over;
    };

    State *next;
    State *prev;
};

struct Game_State {
    State *current_state = 0;
    f32 dt;
    bool show_fps;
    bool fullscreen = false;

    u32 high_score = 0;
};

inline f32 Dot(v2 a, v2 b) {
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

inline f32 Length(v2 v) {
    f32 len_sq = Dot(v, v);
    return sqrtf(len_sq);
}

inline v2 Normalise(v2 v) {
    f32 len = Length(v);
    v2 result = {};
    if (len != 0) {
        result = { v.x / len, v.y / len };
    }
    return result;
}

inline Tower_Type_Flags TowerBitsFromType(Tower_Type type) {
    switch (type) {
        case TowerType_Fire:      return TowerTypeFlags_Fire;
        case TowerType_Electric:  return TowerTypeFlags_Electric;
        case TowerType_Explosion: return TowerTypeFlags_Explosion;
        case TowerType_Ice:       return TowerTypeFlags_Ice;
        defaut: Assert(false);
    }

    return TowerTypeFlags_Fire;
}

inline char *TowerTypeNameFromType(Tower_Type type) {
    switch (type) {
        case TowerType_Fire:      return cast(char *) "Fire";
        case TowerType_Electric:  return cast(char *) "Electric";
        case TowerType_Explosion: return cast(char *) "Explosive";
        case TowerType_Ice:       return cast(char *) "Ice";
        defaut: Assert(false);
    }

    return cast(char *) "UNKNOWN";
}

inline u32 NextRandomInt() {
    u32 result = rand();
    return result;
}

inline f32 RandomUnilateral() {
    f32 result = NextRandomInt() / cast(f32) RAND_MAX;
    return result;
}

inline u32 RandomInt(u32 min, u32 max) {
    f32 rnd = RandomUnilateral();
    u32 result = min + (cast(u32) (rnd * (max - min)));
    return result;
}

inline u32 RandomChoice(u32 choices) {
    u32 result = NextRandomInt() % choices;
    return result;
}

inline bool InsideTowerRaidus(Tower *tower, v2 position) {
    v2 tower_pos = { 60.0f * tower->x, 60.0f * tower->y };


    v2 dist = tower_pos - position;
    f32 len_sq = Dot(dist, dist);

    return len_sq <= (250 * 250);
}

#endif  // LUDUM_H_
