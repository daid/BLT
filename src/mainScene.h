#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <sp2/scene/scene.h>
#include <sp2/scene/tilemap.h>
#include <sp2/script/environment.h>
#include <sp2/timer.h>


class InputDigit;
class Scene : public sp::Scene
{
public:
    Scene(sp::string level_script);
    ~Scene();

    void updateCamera();

    void onFixedUpdate() override;
    void onUpdate(float delta) override;

    bool onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id) override;
    void onPointerDrag(sp::Ray3d ray, int id) override;
    void onPointerUp(sp::Ray3d ray, int id) override;

    void addInput(int n);
    void fireString();

    sp::P<sp::Tilemap> bg_overlay;
    sp::P<sp::Tilemap> message_overlay;
    sp::P<sp::Tilemap> score_display;
    sp::P<sp::Tilemap> multiplier_display;
    sp::P<sp::Tilemap> game_over_display;
    sp::PList<InputDigit> inputs;
    sp::P<sp::script::Environment> env;
    sp::script::CoroutinePtr coroutine;

    int score_display_value = 0;
    sp::Timer score_display_pulse_timer;
    int multiplier_display_value = 0;
    sp::Timer multiplier_display_pulse_timer;

    sp::Timer exit_level_timer;
};

#endif//MAIN_SCENE_H
