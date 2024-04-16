#include "mainScene.h"
#include "ingameMenu.h"
#include "main.h"

#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/mesh/obj.h>
#include <sp2/random.h>
#include <sp2/tween.h>
#include <sp2/engine.h>
#include <sp2/scene/camera.h>
#include <sp2/scene/particleEmitter.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/collision/2d/circle.h>
#include <sp2/io/cameraCapture.h>
#include <sp2/scene/tilemap.h>



class Trail : public sp::Node
{
public:
    Trail(sp::P<sp::Node> parent, sp::P<sp::Node> _target) : sp::Node(parent), target(_target)
    {
        render_data.type = sp::RenderData::Type::Additive;
        render_data.texture = sp::texture_manager.get("gui/theme/pixel.png");
        render_data.shader = sp::Shader::get("internal:color_by_normal.shader");
        if (target)
            previous_position = target->getGlobalPosition2D();
    }

    void onUpdate(float delta) override
    {
        for(auto& point : point_list)
            point.lifetime -= delta * timetime_speed;
        if (!point_list.empty() && point_list.front().lifetime <= 0.0f)
            point_list.erase(point_list.begin());
        if (target) {
            auto p = target->getGlobalPosition2D();
            auto diff = p - previous_position;
            if (diff.dot(diff) > 0.1) {
                previous_position = p;
                point_list.push_back({sp::Vector2f(p), 1.0f});
            }
        } else {
            if (point_list.empty()) {
                delete this;
                return;
            }
        }

        auto pls = int(point_list.size());
        if (pls < 2) return;
        std::vector<sp::Vector2f> normals;
        for(int idx=0; idx<pls-1; idx++) {
            normals.push_back((point_list[idx+1].position - point_list[idx].position).normalized() * width);
        }
        normals.push_back(normals.back());

        sp::MeshData::Indices indices;
        sp::MeshData::Vertices vertices;

        for(int idx=0; idx<pls; idx++) {
            auto n = sp::Vector2f(normals[idx].y, -normals[idx].x);
            auto f = point_list[idx].lifetime;
            auto c = sp::Vector3f(f, f, f);
            vertices.emplace_back(sp::Vector3f(point_list[idx].position.x + n.x, point_list[idx].position.y + n.y, 0.0f), c);
            vertices.emplace_back(sp::Vector3f(point_list[idx].position.x - n.x, point_list[idx].position.y - n.y, 0.0f), c);
        }

        for(int idx=0; idx<pls-1; idx++) {
            indices.push_back(idx*2);
            indices.push_back(idx*2+2);
            indices.push_back(idx*2+1);

            indices.push_back(idx*2+3);
            indices.push_back(idx*2+1);
            indices.push_back(idx*2+2);
        }
        
        render_data.mesh = sp::MeshData::create(std::move(vertices), std::move(indices));
    }

private:
    float timetime_speed = 5.0f;
    float width = 0.2f;

    sp::P<sp::Node> target;
    sp::Vector2d previous_position;
    struct Point {
        sp::Vector2f position;
        float lifetime;
    };
    std::vector<Point> point_list;
};



bool game_over = false;
double falling_y_max = 20.0;
double falling_y_min = 20.0;
double score_multiply = 1.0;
double score = 0.0;
double falling_speed = 0.4;
float camera_shake_time = 0.0f;
std::array<std::shared_ptr<sp::MeshData>, 4> digit_meshes;

class FallingDigit : public sp::Node
{
public:
    FallingDigit(sp::P<sp::Node> parent, int num) : sp::Node(parent) {
        this->num = num;
        sp::collision::Circle2D shape(0.4);
        shape.type = sp::collision::Shape::Type::Sensor;
        setCollisionShape(shape);

        if (!digit_meshes[num]) {
            float u = float(num) / 16.0f;
            float v = 1.0f / 6.0f;
            digit_meshes[num] = sp::MeshData::createQuad({1.0f, 1.0f}, {u, v}, {u+1.0f/16.0f, v+1.0f/6.0f});
        }
        visual = new sp::Node(this);
        visual->render_data.mesh = digit_meshes[num];
        visual->render_data.type = sp::RenderData::Type::Normal;
        visual->render_data.texture = sp::texture_manager.get("gui/theme/font.png");
        visual->render_data.shader = sp::Shader::get("internal:basic.shader");

        game_over_velocity = sp::Vector2d(sp::random(-30, 30), sp::random(-30, 30));
    }

    void onUpdate(float delta) override {
        if (game_over) {
            setPosition(getPosition2D() + game_over_velocity * double(delta));
            game_over_velocity.y -= delta * 30.0;
            if (getPosition2D().y < -22)
                delete this;
        } else {
            setPosition(getPosition2D() + sp::Vector2d(0, -delta*falling_speed));
            auto shake = sp::Tween<double>::easeInCubic(falling_y_min, -4.0f, -18.0f, 0.0, 0.3);
            if (shake > 0)
                visual->setPosition({sp::random(-shake, shake), sp::random(-shake, shake)});
            else
                visual->setPosition({0.0, 0.0});
        }
    }

    sp::P<sp::Node> visual;
    int num;
    sp::Vector2d game_over_velocity;
};

class InputDigit : public sp::Node
{
public:
    InputDigit(sp::P<sp::Node> parent, int num)
    : sp::Node(parent) {
        this->num = num;
        if (!digit_meshes[num]) {
            float u = float(num) / 16.0f;
            float v = 1.0f / 6.0f;
            digit_meshes[num] = sp::MeshData::createQuad({1.0f, 1.0f}, {u, v}, {u+1.0f/16.0f, v+1.0f/6.0f});
        }
        render_data.mesh = digit_meshes[num];
        render_data.type = sp::RenderData::Type::Normal;
        render_data.texture = sp::texture_manager.get("gui/theme/font.png");
        render_data.shader = sp::Shader::get("internal:basic.shader");
    }

    void fire() {
        sp::collision::Circle2D shape(0.4);
        shape.type = sp::collision::Shape::Type::Sensor;
        setCollisionShape(shape);
        setLinearVelocity(sp::Vector2d(0, initial_velocity));
        new Trail(getParent(), this);
        fired = true;
    }

    void onUpdate(float delta) override {
        if (!fired) return;
        auto v = getLinearVelocity2D();
        v.y += delta * 40.0;
        setLinearVelocity(v);
        auto p = getPosition2D();
        if (p.y > 24.0) delete this;
    }

    void onCollision(sp::CollisionInfo& info) override
    {
        sp::P<FallingDigit> d = info.other;
        if (d) {
            if (d->num == num) {
                auto pe = new sp::ParticleEmitter(getParent(), sp::string(num) + ".explosion.particles.txt");
                pe->setPosition(d->getPosition2D());
                d.destroy();
                score += score_multiply;
                score_multiply += 1.0;
                camera_shake_time = 0.2;
            } else {
                score_multiply = std::min(1.0, score_multiply - 1.0);
            }
            delete this;
        }
    }

    static constexpr double initial_velocity = 4.0;
    bool fired = false;
    int num;
};


static void setOverlayAlpha(float alpha) { sp::P<Scene>(sp::Scene::get("MAIN"))->bg_overlay->render_data.color.a = alpha; }
static void setFallSpeed(double speed) { falling_speed = speed; }
static double getMinY() { return falling_y_min; }
static double getMaxY() { return falling_y_max; }
static void spawn(sp::Vector2d pos, int num) {
    (new FallingDigit(sp::Scene::get("MAIN")->getRoot(), num))->setPosition(pos);
    falling_y_min = std::min(falling_y_min, pos.y);
    falling_y_max = std::max(falling_y_max, pos.y);
}
static void showMessage(sp::string msg) {
    auto s = sp::P<Scene>(sp::Scene::get("MAIN"));
    s->message_overlay.destroy();
    s->message_overlay = new sp::Tilemap(s->getRoot(), "gui/theme/font.png", 1.0, 1.0, 16, 6);
    int x = 0;
    int y = 0;
    int width = 0;
    for(auto c : msg) {
        if (c == '\n') {
            y -= 1;
            x = 0;
        } else {
            s->message_overlay->setTile({x++, y}, c - 0x20);
            width = std::max(width, x);
        }
    }
    s->message_overlay->setPosition(sp::Vector2d(width * -.5, 10 + y * -.5));
}
static sp::string getInput() {
    auto s = sp::P<Scene>(sp::Scene::get("MAIN"));
    sp::string result;
    for(auto i : s->inputs)
        result += sp::string(i->num);
    return result;
}
static void clearInput() {
    auto s = sp::P<Scene>(sp::Scene::get("MAIN"));
    for(auto i : s->inputs)
        i.destroy();
}

Scene::Scene()
: sp::Scene("MAIN")
{
    sp::Scene::get("INGAME_MENU")->enable();

    auto camera = new sp::Camera(getRoot());
    camera->setOrtographic(20.0);
    setDefaultCamera(camera);

    /*
    for(int y=0; y<32; y++) {
        for(int x=0; x<8; x++) {
            auto fd = new FallingDigit(getRoot(), sp::irandom(0, 1));
            fd->setPosition({x - 4.0, y + 10.0});
            fd->setPosition({x - 4.0, y - 8.0});
            //fd->setPosition({x - 4.0, y + 38.0});
        }
    }
    */

    auto bg = new sp::Tilemap(getRoot(), "gui/theme/font.png", 1.0, 1.0, 16, 6);
    bg->render_data.color = sp::HsvColor(120, 70, 20);
    sp::Image img;
    img.loadFromStream(sp::io::ResourceProvider::get("bg.png"));
    for(int y=0; y<img.getSize().y; y++)
        for(int x=0; x<img.getSize().x; x++)
            bg->setTile({x, img.getSize().y - y - 1}, (img.getPtr()[x+y*img.getSize().x] & 0xFF) != 0 ? 17 : 16);
    bg->setPosition(sp::Vector2d(-img.getSize().x / 2, -img.getSize().y / 2));
    bg->render_data.order = -2;

    bg_overlay = new sp::Tilemap(getRoot(), "gui/theme/font.png", 1.0, 1.0, 16, 6);
    bg_overlay->render_data.color.a = 0.0;
    img.loadFromStream(sp::io::ResourceProvider::get("bg_intro.png"));
    for(int y=0; y<img.getSize().y; y++)
        for(int x=0; x<img.getSize().x; x++)
            if ((img.getPtr()[x+y*img.getSize().x] & 0xFF) != 0)
                bg_overlay->setTile({x, img.getSize().y - y - 1}, 127);
    bg_overlay->setPosition(sp::Vector2d(-img.getSize().x / 2, -img.getSize().y / 2));
    auto draw_str = [this](const std::string_view s, int x, int y) {
        for(auto c : s)
            bg_overlay->setTile({x++, y}, c - 0x20);
    };
    draw_str("BINARY", 4, 29);
    draw_str("LANGUAGE", 9, 28);
    draw_str("TUTOR", 16, 27);

    game_over = false;
    score = 0.0;
    score_multiply = 1.0;
    camera_shake_time = 0.0;

    env = new sp::script::Environment();
    env->setGlobal("yield", [](lua_State* L) { return lua_yield(L, 0); });
    env->setGlobal("setOverlayAlpha", setOverlayAlpha);
    env->setGlobal("setFallSpeed", setFallSpeed);
    env->setGlobal("spawn", spawn);
    env->setGlobal("getMinY", getMinY);
    env->setGlobal("getMaxY", getMaxY);
    env->setGlobal("getInput", getInput);
    env->setGlobal("clearInput", clearInput);
    env->setGlobal("showMessage", showMessage);
    env->setGlobal("random", sp::random);
    env->setGlobal("irandom", sp::irandom);

    coroutine = env->loadCoroutine("tutorial.lua").value();
}

Scene::~Scene()
{
    sp::Scene::get("INGAME_MENU")->disable();
}

void Scene::addInput(int n)
{
    inputs.add(new InputDigit(getRoot(), n));
    double x = (inputs.size() - 1) * -.5;
    double y = -18;
    for(auto i : inputs) {
        i->setPosition(sp::Vector2d(x, y));
        x += 1.0;
    }
}

void Scene::fireString()
{
    for(auto i : inputs) {
        i->fire();
    }
    inputs.clear();
}

void Scene::onFixedUpdate()
{
    if (coroutine) {
        if (!coroutine->resume().value())
            coroutine = nullptr;
    }
}

void Scene::onUpdate(float delta)
{
    if (camera_shake_time > 0.0) {
        camera_shake_time -= delta;
        auto f = sp::Tween<double>::linear(camera_shake_time, 0.0f, 0.2f, 0.0, 0.3);
        getCamera()->setPosition(sp::Vector2d(sp::random(-f, f), sp::random(-f, f)));
    }
    if (!game_over) {
        if (controller.zero.getDown())
            addInput(0);
        if (controller.one.getDown())
            addInput(1);
        if (controller.fire.getDown())
            fireString();
        falling_y_min = 20.0;
        falling_y_max = -20.0;
        for(sp::P<FallingDigit> fd : getRoot()->getChildren()) {
            if (fd) {
                falling_y_min = std::min(falling_y_min, fd->getPosition2D().y);
                falling_y_max = std::max(falling_y_max, fd->getPosition2D().y);
            }
        }
        if (falling_y_min < -17.0) {
            game_over = true;
            for(sp::P<FallingDigit> fd : getRoot()->getChildren()) {
                if (fd)
                    new Trail(getRoot(), fd);
            }
            for(auto i : inputs)
                i.destroy();
        }
    }
}

bool Scene::onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id)
{
    return true;
}

void Scene::onPointerDrag(sp::Ray3d ray, int id)
{
}

void Scene::onPointerUp(sp::Ray3d ray, int id)
{
}

/*
https://opengameart.org/content/technological-messup
https://opengameart.org/content/neon-action-ii
https://opengameart.org/content/driving-in-the-rain
https://opengameart.org/content/riding-the-supernova
*/