#if LUDUM_WINDOWS
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include <SFML/Graphics.hpp>


#include "Ludum_Platform.h"

global sf::RenderWindow window;

#include "Ludum.h"
global Assets assets;

#include "Ludum_Asset.cpp"

#include "Ludum_Generation.cpp"
#include "Ludum_Enemy.cpp"
#include "Ludum_Projectile.cpp"
#include "Ludum_Tower.cpp"

#include "Ludum.cpp"

void SFMLHandleKey(Game_Button *cur, Game_Button *prev, sf::Mouse::Button button) {
    cur->is_pressed = sf::Mouse::isButtonPressed(button);
    cur->transition = cur->is_pressed != prev->is_pressed ? 1 : 0;
}

void SFMLHandleKey(Game_Button *cur, Game_Button *prev, sf::Keyboard::Key key) {
    cur->is_pressed = sf::Keyboard::isKeyPressed(key);
    cur->transition = cur->is_pressed != prev->is_pressed ? 1 : 0;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
// int main(int argc, char **argv)

    srand(time(0));

    window.create(sf::VideoMode(1280, 720), "The Cost of Defence", sf::Style::Close);
    window.setFramerateLimit(60);

    f32 zoom = 1.0;
    f32 zoom_speed = 10;
    sf::View main_view(v2(960, 540), v2(1920, 1080));
    window.setView(main_view);

    // This will change to Game_State at some point
    Game_State _state = {};
    Game_State *state = &_state;
    state->dt = 1.0f / 60.0f;

#if LUDUM_WINDOWS
    SetCurrentDirectory("..\\data");
#endif


    State *new_state = cast(State *) malloc(sizeof(State));
    new_state->type = StateType_Menu;
    PushState(state, new_state);

    LoadContent(&assets);

    Game_Input input[2] = {};
    Game_Input *cur_input = &input[0];
    Game_Input *prev_input = &input[1];

    sf::Clock timer;
    f32 accumulator = 0;
    u32 fps = 0;
    bool paused = false;
    while (window.isOpen())
    {

        f32 elapsed = timer.getElapsedTime().asSeconds();
        accumulator += elapsed;
        timer.restart();

        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                {
                    window.close();
                }
                break;
                case sf::Event::LostFocus: {
                    paused = true;
                }
                break;
                case sf::Event::GainedFocus: {
                    paused = false;
                }
                break;
            }
        }

        if (paused) continue;
        // Input handling
        SFMLHandleKey(&cur_input->next_tower, &prev_input->next_tower, sf::Keyboard::Key::E);
        SFMLHandleKey(&cur_input->prev_tower, &prev_input->prev_tower, sf::Keyboard::Key::Q);
        SFMLHandleKey(&cur_input->refresh, &prev_input->refresh, sf::Keyboard::Key::R);

        SFMLHandleKey(&cur_input->one,   &prev_input->one,   sf::Keyboard::Key::Num1);
        SFMLHandleKey(&cur_input->two,   &prev_input->two,   sf::Keyboard::Key::Num2);
        SFMLHandleKey(&cur_input->three, &prev_input->three, sf::Keyboard::Key::Num3);
        SFMLHandleKey(&cur_input->four,  &prev_input->four,  sf::Keyboard::Key::Num4);

        SFMLHandleKey(&cur_input->left_mouse,
                &prev_input->left_mouse, sf::Mouse::Button::Left);
        SFMLHandleKey(&cur_input->right_mouse,
                &prev_input->left_mouse, sf::Mouse::Button::Right);

        sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
        cur_input->mouse_position = window.mapPixelToCoords(mouse_pos);

        accumulator = Min(accumulator, 0.2f);

        window.clear(sf::Color::Magenta);
        GameUpdateRender(state, cur_input);

        if (state->show_fps) {
            fps = cast(u32) (0.5f + (1.0f / elapsed));
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "FPS: %d\n", fps);
            sf::Text text(buffer, assets.ui_font, 25);
            text.setPosition(10, 10);

            window.draw(text);
        }

        window.display();
        accumulator -= state->dt;

        if (cur_input->requested_quit) {
            window.close();
        }

        Swap(Game_Input *, cur_input, prev_input);
    }
}
