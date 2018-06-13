#include "Engine.hpp"

Subsystem::Subsystem(Engine* engine) : Object(engine) {}

Object::Object(Engine* engine) : engine{engine}, reg{engine->getReg()} {}
