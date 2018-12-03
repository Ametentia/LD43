internal void LoadContent(Assets *assets) {
    Assert(assets->enemy_textures[0].loadFromFile("EnemyNormal.png"));
    Assert(assets->enemy_textures[1].loadFromFile("EnemyFire.png"));
    Assert(assets->enemy_textures[2].loadFromFile("EnemyElectric.png"));
    Assert(assets->enemy_textures[3].loadFromFile("EnemyBomb.png"));
    Assert(assets->enemy_textures[4].loadFromFile("EnemyIce.png"));

    Assert(assets->slowed_texture.loadFromFile("EnemyFrozen.png"));

    Assert(assets->dirt_textures[0].loadFromFile("Dirt.png"));
    Assert(assets->dirt_textures[1].loadFromFile("Dirt2.png"));
    Assert(assets->dirt_textures[2].loadFromFile("Dirt3.png"));

    Assert(assets->gradient.loadFromFile("Gradient.png"));

    Assert(assets->fullscreen_button.loadFromFile("Fullscreen.png"));
    Assert(assets->minimise_button.loadFromFile("Minimise.png"));

    Assert(assets->house.loadFromFile("Town.png"));
    Assert(assets->tower_bar.loadFromFile("TowerBar.png"));

    Assert(assets->grass_textures[0].loadFromFile("Grass.png"));
    Assert(assets->grass_textures[1].loadFromFile("Grass2.png"));
    Assert(assets->grass_textures[2].loadFromFile("Grass3.png"));
    Assert(assets->grass_textures[3].loadFromFile("Grass4.png"));
    Assert(assets->grass_textures[4].loadFromFile("Grass5.png"));
    Assert(assets->grass_textures[5].loadFromFile("Grass6.png"));
    Assert(assets->grass_textures[6].loadFromFile("Grass7.png"));

    Assert(assets->destroyed_tower_texture.loadFromFile("TowerDestroyed.png"));
    Assert(assets->tower_textures[0].loadFromFile("TowerFire.png"));
    Assert(assets->tower_textures[1].loadFromFile("TowerElectric.png"));
    Assert(assets->tower_textures[2].loadFromFile("TowerIce.png"));
    Assert(assets->tower_textures[3].loadFromFile("TowerBomb.png"));

    Assert(assets->explosion.loadFromFile("Explosion.png"));
    Assert(assets->ice_explosion.loadFromFile("IceExplosion.png"));

    Assert(assets->projectile_textures[0].loadFromFile("ProjectileFire.png"));
    Assert(assets->projectile_textures[1].loadFromFile("ProjectileElectric.png"));
    Assert(assets->projectile_textures[2].loadFromFile("ProjectileIce.png"));
    Assert(assets->projectile_textures[3].loadFromFile("ProjectileBomb.png"));

    Assert(assets->ui_font.loadFromFile("Ubuntu.ttf"));

    Assert(assets->title_screen.loadFromFile("Title.png"));
    Assert(assets->exit_button[0].loadFromFile("ExitUnselected.png"));
    Assert(assets->exit_button[1].loadFromFile("ExitSelected.png"));
    Assert(assets->play_button[0].loadFromFile("PlayUnselected.png"));
    Assert(assets->play_button[1].loadFromFile("PlaySelected.png"));
}

internal Animation CreateAnimation(sf::Texture *texture, u32 rows, u32 columns, v2 position,
        f32 time_per_frame = 0.1f, u32 frame_offset = 0)
{
    Animation result;

    result.texture = texture;
    result.valid = true;

    result.rows = rows;
    result.columns = columns;
    result.total_frames = rows * columns;
    result.position = position;

    result.width = result.texture->getSize().x / columns;
    result.height = result.texture->getSize().y / rows;

    result.current_frame = frame_offset;
    result.time_per_frame = time_per_frame;
    result.accumulator = 0;

    result.playing = true;

    return result;
}

internal bool DrawAnimation(Animation *animation, f32 dt) {
    if (!animation->playing) return true;

    bool result = false;
    animation->accumulator += dt;
    if (animation->accumulator >= animation->time_per_frame) {
        result = animation->current_frame + 1 >= animation->total_frames;
        animation->current_frame = (animation->current_frame + 1) % animation->total_frames;
        animation->accumulator = 0;
    }

    sf::Sprite sprite(*animation->texture);
    sprite.setPosition(animation->position);
    sprite.setOrigin(animation->width / 2, animation->height / 2);

    u32 row = cast(u32) ((cast(f32) animation->current_frame) / (cast(f32)
                animation->columns));
    u32 column = animation->current_frame % animation->columns;

    sprite.setTextureRect(sf::IntRect(column * animation->width, row * animation->height,
                animation->width, animation->height));
    window.draw(sprite);

    return result;
}
