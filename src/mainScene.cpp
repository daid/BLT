#include "mainScene.h"
#include "ingameMenu.h"
#include "main.h"

#include <array>
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
#include <sp2/collision/2d/box.h>
#include <sp2/audio/sound.h>
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
        if (!render_data.mesh)
            render_data.mesh = sp::MeshData::create(std::move(vertices), std::move(indices));
        else
            render_data.mesh->update(std::move(vertices), std::move(indices));
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
        sp::collision::Box2D shape{0.8, 2.0, 0, -0.5};
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
                sp::audio::Sound::play("explosion.wav");
                score += score_multiply;
                score_multiply += 1.0;
                camera_shake_time = 0.2;
            } else {
                int sub = score_multiply / 10 + 1;
                score_multiply = std::max(1.0, score_multiply - sub);
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
static void spawnImage(sp::string name) {
    sp::Image img;
    img.loadFromStream(sp::io::ResourceProvider::get(name));
    for(int y=0; y<img.getSize().y; y++) {
        for(int x=0; x<img.getSize().x; x++) {
            auto c = img.getPtr()[x+y*img.getSize().x];
            if (c & 0xFF000000) {
                spawn({double(x) - img.getSize().x * .5, double(img.getSize().y - y - 1)}, (c & 0xFF) ? 1 : 0);
            }
        }
    }
}
static void showMessage(sp::string msg, bool lower) {
    auto s = sp::P<Scene>(sp::Scene::get("MAIN"));
    s->message_overlay.destroy();
    s->message_overlay = new sp::Tilemap(s->getRoot(), "gui/theme/font.png", 1.0, 1.0, 16, 6);
    s->message_overlay->setTilemapSpacingMargin(0.01, 0.0);
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
    if (lower)
        s->message_overlay->setPosition(sp::Vector2d(width * -.5, -10 + y * -.5));
    else
        s->message_overlay->setPosition(sp::Vector2d(width * -.5, 10 + y * -.5));
}
static void victory() {
    auto s = sp::P<Scene>(sp::Scene::get("MAIN"));
    auto victory_display = new sp::Tilemap(s->getRoot(), "gui/theme/font.png", 2.3, 2.3, 16, 6);
    victory_display->setTilemapSpacingMargin(0.01, 0.0);
    victory_display->setPosition({2.3*-.5, 0});
    victory_display->setRotation(sp::random(-20, 20));
    int x = -3;
    for(auto c : "VICTORY")
        victory_display->setTile({x++, 0}, c - 0x20);
    s->exit_level_timer.start(5.0);
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

Scene::Scene(sp::string level_script)
: sp::Scene("MAIN")
{
    //sp::Scene::get("INGAME_MENU")->enable();

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

    auto lines = new sp::Tilemap(getRoot(), "gui/theme/font.png", 1.0, 1.0, 16, 6);
    lines->setTilemapSpacingMargin(0.01, 0.0);
    for(int x=-10; x<10; x++) {
        lines->setTile({x, 1}, '_' - 0x20);
    }
    lines->setPosition({0, -18.4});

    bg_overlay = new sp::Tilemap(getRoot(), "gui/theme/font.png", 1.0, 1.0, 16, 6);
    bg_overlay->setTilemapSpacingMargin(0.01, 0.0);
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

    score_display = new sp::Tilemap(getRoot(), "gui/theme/font.png", 1.8, 1.8, 16, 6);
    score_display->setTilemapSpacingMargin(0.01, 0.0);
    score_display->setRotation(80);
    score_display->setPosition({-8, 17});
    score_display->setTile({0, 0}, 16);
    multiplier_display = new sp::Tilemap(getRoot(), "gui/theme/font.png", 1.2, 1.2, 16, 6);
    multiplier_display->setTilemapSpacingMargin(0.01, 0.0);
    multiplier_display->setRotation(80);
    multiplier_display->setPosition({-6.5, 17});
    multiplier_display->setTile({0, 0}, 'x' - 0x20);
    multiplier_display->setTile({-1, 0}, 16);

    env = new sp::script::Environment();
    env->setGlobal("yield", [](lua_State* L) { return lua_yield(L, 0); });
    env->setGlobal("setOverlayAlpha", setOverlayAlpha);
    env->setGlobal("setFallSpeed", setFallSpeed);
    env->setGlobal("spawn", spawn);
    env->setGlobal("spawnImage", spawnImage);
    env->setGlobal("getMinY", getMinY);
    env->setGlobal("getMaxY", getMaxY);
    env->setGlobal("getInput", getInput);
    env->setGlobal("clearInput", clearInput);
    env->setGlobal("showMessage", showMessage);
    env->setGlobal("victory", victory);
    env->setGlobal("random", sp::random);
    env->setGlobal("irandom", sp::irandom);

    coroutine = env->loadCoroutine(level_script).value();
}

Scene::~Scene()
{
    //sp::Scene::get("INGAME_MENU")->disable();
}

void Scene::addInput(int n)
{
    sp::audio::Sound::play(sp::string(n) + ".wav");
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
    if (inputs.empty()) return;
    sp::audio::Sound::play("shoot.wav");
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
    if (score_display_value < score) {
        score_display_value += 1;
        score_display_value += (score - score_display_value) / 20;
        score_display_pulse_timer.start(0.3);
        int num = score_display_value;
        int x = 0;
        do {
            score_display->setTile({x, 0}, (num % 10) + 16);
            num /= 10;
            x -= 1;
        } while(num > 0);
    }
    score_display_pulse_timer.isExpired();
    if (score_display_pulse_timer.isRunning()) {
        auto f = sp::Tween<float>::easeOutCubic(score_display_pulse_timer.getProgress(), 0.0, 1.0, sp::random(2.0, 1.0), 1.0);
        score_display->render_data.scale = {f, f, f};
    } else {
        score_display->render_data.scale = {1.0f, 1.0f, 1.0f};
    }

    if (multiplier_display_value != int(score_multiply)) {
        if (multiplier_display_value < score_multiply)
            multiplier_display_pulse_timer.start(0.3);
        multiplier_display_value = score_multiply;
        multiplier_display->setTile({0, 0}, 'x' - 0x20);
        int num = multiplier_display_value;
        int x = -1;
        do {
            multiplier_display->setTile({x, 0}, (num % 10) + 16);
            num /= 10;
            x -= 1;
        } while(num > 0);
        for(int n=0; n<4; n++)
            multiplier_display->setTile({x--, 0}, -1);
    }
    multiplier_display_pulse_timer.isExpired();
    if (multiplier_display_pulse_timer.isRunning()) {
        auto f = sp::Tween<float>::easeOutCubic(multiplier_display_pulse_timer.getProgress(), 0.0, 1.0, sp::random(2.0, 1.0), 1.0);
        multiplier_display->render_data.scale = {f, f, f};
    } else {
        multiplier_display->render_data.scale = {1.0f, 1.0f, 1.0f};
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
            sp::audio::Sound::play("explosion.wav");
            for(sp::P<FallingDigit> fd : getRoot()->getChildren()) {
                if (fd)
                    new Trail(getRoot(), fd);
            }
            for(auto i : inputs)
                i.destroy();
        }
    } else {
        int left = 0;
        for(sp::P<FallingDigit> fd : getRoot()->getChildren()) {
            if (fd)
                left += 1;
        }
        if (!left && !game_over_display) {
            game_over_display = new sp::Tilemap(getRoot(), "gui/theme/font.png", 2.3, 2.3, 16, 6);
            game_over_display->setTilemapSpacingMargin(0.01, 0.0);
            game_over_display->setPosition({2.3*-.5, 0});
            game_over_display->setRotation(sp::random(-20, 20));
            int x = -4;
            for(auto c : "GAME OVER")
                game_over_display->setTile({x++, 0}, c - 0x20);
            exit_level_timer.start(5.0);
        }
    }
    if (exit_level_timer.isExpired()) {
        sp::Scene::get("LEVEL_SELECT")->enable();
        delete this;
        return;
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