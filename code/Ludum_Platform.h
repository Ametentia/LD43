#if !defined(LUDUM_PLATFORM_H_)
#define LUDUM_PLATFORM_H_

#include <stdint.h>
#include <float.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

#define cast
#define Assert(exp) \
    do { if (!(exp)) { printf("Assertion Failed: %s (%s:%d)\n", #exp, __FILE__, __LINE__); asm("int3"); } } while (0)


#define internal static
#define local static
#define global static

typedef sf::Vector2f v2;
typedef sf::Vector3f v3;
typedef sf::Color v4;

struct Game_Button {
    bool is_pressed;
    u32 transition;
};

struct Game_Input {
    bool requested_quit = false;
    union {
        struct {
            Game_Button left_mouse;
            Game_Button right_mouse;

            Game_Button refresh;
            Game_Button next_tower;
            Game_Button prev_tower;

            Game_Button one;
            Game_Button two;
            Game_Button three;
            Game_Button four;
        };
        Game_Button buttons[9];
    };

    v2 mouse_position; // Unprojected
};

inline bool GameButtonJustPressed(Game_Button button) {
    bool result = button.is_pressed && button.transition > 0;
    return result;
}

inline bool GameButtonPressed(Game_Button button) {
    bool result = button.is_pressed;
    return result;
}

inline bool GameButtonReleased(Game_Button button) {
    bool result = !button.is_pressed && button.transition > 0;
    return result;
}

// Maximums
#define U8_MAX 255
#define U16_MAX 65535
#define U32_MAX (cast(u32) -1)
#define U64_MAX (cast(u64) -1)
#define F32_MAX FLT_MAX
#define F64_MAX DBL_MAX

#define PI32  3.1415927410125732421875f
#define TAU32 6.2831854820251464843750f

#define Degrees(rad) ((rad) * (180.0f / PI32))
#define Radians(deg) ((deg) * (PI32 / 180.0f))

#define Abs(x) (x) < 0 ? -(x) : (x)
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Clamp(x, min, max) Max(min, Min(x, max))

#define Swap(type, a, b) { type tmp = a; a = b; b = tmp; }
#define ArrayCount(a) (sizeof(a) / (sizeof((a)[0])))

#define ForList(Type, list) for (Type *item = list; item; item = item->next)
#define WhileList(type, list) type *item = list; while (item)
#define WhileAdvance item = item->next

#endif  // LUDUM_PLATFORM_H_
