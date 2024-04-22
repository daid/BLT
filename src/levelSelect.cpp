#include "levelSelect.h"
#include "main.h"
#include "mainScene.h"
#include <sp2/scene/camera.h>


LevelSelectScene::LevelSelectScene()
: sp::Scene("LEVEL_SELECT")
{
    auto camera = new sp::Camera(getRoot());
    camera->setOrtographic(20.0);
    setDefaultCamera(camera);

    auto bg = new sp::Tilemap(getRoot(), "gui/theme/font.png", 1.0, 1.0, 16, 6);
    bg->setTilemapSpacingMargin(0.01, 0.0);
    bg->render_data.color = sp::HsvColor(120, 70, 20);
    sp::Image img;
    img.loadFromStream(sp::io::ResourceProvider::get("bg.png"));
    for(int y=0; y<img.getSize().y; y++)
        for(int x=0; x<img.getSize().x; x++)
            bg->setTile({x, img.getSize().y - y - 1}, (img.getPtr()[x+y*img.getSize().x] & 0xFF) != 0 ? 17 : 16);
    bg->setPosition(sp::Vector2d(-img.getSize().x / 2, -img.getSize().y / 2));
    bg->render_data.order = -2;

    auto up_down_indicators = new sp::Tilemap(getRoot(), "gui/theme/font.png", 2.0, 2.0, 16, 6);
    up_down_indicators->setTilemapSpacingMargin(0.01, 0.0);
    up_down_indicators->setTile({0, 1}, '0' - 0x20);
    up_down_indicators->setTile({0, -1}, '1' - 0x20);
    up_down_indicators->setPosition({-1, -0.5});

    auto lines = new sp::Tilemap(getRoot(), "gui/theme/font.png", 1.0, 1.0, 16, 6);
    lines->setTilemapSpacingMargin(0.01, 0.0);
    for(int x=-10; x<10; x++) {
        lines->setTile({x, 1}, '_' - 0x20);
        lines->setTile({x, -1}, '_' - 0x20);
    }
    lines->setPosition({0, .5});

    auto play_indicator = new sp::Tilemap(getRoot(), "gui/theme/font.png", 1.0, 1.0, 16, 6);
    play_indicator->setTilemapSpacingMargin(0.01, 0.0);
    int x = 0;
    for(auto c : "[Space]->Play")
        play_indicator->setTile({x++, 0}, c - 0x20);
    play_indicator->setPosition({-7, -18});

    level_select = new sp::Tilemap(getRoot(), "gui/theme/font.png", 1.0, 1.0, 16, 6);
    level_select->setTilemapSpacingMargin(0.01, 0.0);

    levels = sp::io::ResourceProvider::find("*.lua");
    for(auto& level : levels) {
        level = level.substr(0, level.find_last_of("."));
    }
    std::sort(levels.begin(), levels.end());
    int y = 0;
    for(const auto& level : levels) {
        x = -int(level.size()) / 2;
        for(auto c : level) {
            level_select->setTile({x++, y}, c - 0x20);
        }
        y -= 4;
    }
}

LevelSelectScene::~LevelSelectScene()
{
}

void LevelSelectScene::onFixedUpdate() {
    if (controller.one.getDown())
        level_index = (level_index + 1) % levels.size();
    if (controller.zero.getDown())
        level_index = (level_index + levels.size() - 1) % levels.size();
    if (controller.fire.getDown()) {
        disable();
        new ::Scene(levels[level_index] + ".lua");
    }
    double target_y = level_index * 4;
    auto p = level_select->getPosition2D();
    level_select->setPosition(p + sp::Vector2d(0, (target_y - p.y) * .1));
}
