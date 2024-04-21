#pragma once

#include <sp2/scene/scene.h>
#include <sp2/scene/tilemap.h>
#include <sp2/script/environment.h>
#include <sp2/timer.h>


class LevelSelectScene : public sp::Scene
{
public:
    LevelSelectScene();
    ~LevelSelectScene();

    void onFixedUpdate() override;

    sp::P<sp::Tilemap> level_select;
    std::vector<sp::string> levels;
    int level_index = 0;
};
