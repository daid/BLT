#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <sp2/scene/scene.h>


class InputDigit;
class Scene : public sp::Scene
{
public:
    Scene();
    ~Scene();

    void updateCamera();

    virtual void onUpdate(float delta) override;

    virtual bool onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id) override;
    virtual void onPointerDrag(sp::Ray3d ray, int id) override;
    virtual void onPointerUp(sp::Ray3d ray, int id) override;

    void addInput(int n);
    void fireString();

    sp::PList<InputDigit> inputs;
};

#endif//MAIN_SCENE_H
