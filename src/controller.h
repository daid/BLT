#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <sp2/io/keybinding.h>

class Controller : sp::NonCopyable
{
public:
    Controller();

    sp::io::Keybinding zero;
    sp::io::Keybinding one;
    sp::io::Keybinding fire;

    sp::PList<sp::io::Keybinding> all;
};

#endif//CONTROLLER_H
