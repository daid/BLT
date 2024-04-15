#include "controller.h"


Controller::Controller()
: zero("ZERO"), one("ONE"), fire("FIRE")
{
    zero.setKeys({"0", "z", "left shift", "gamecontroller:0:button:a"});
    one.setKeys({"1", "x", "right shift", "gamecontroller:0:button:x"});
    fire.setKeys({"space", "gamecontroller:0:button:b"});

    all.add(&zero);
    all.add(&one);
    all.add(&fire);
}
