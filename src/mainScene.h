#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <sp2/scene/scene.h>
#include <sp2/scene/tilemap.h>
#include <sp2/script/environment.h>


class InputDigit;
class Scene : public sp::Scene
{
public:
    Scene();
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
    sp::PList<InputDigit> inputs;
    sp::P<sp::script::Environment> env;
    sp::script::CoroutinePtr coroutine;
};

#endif//MAIN_SCENE_H
