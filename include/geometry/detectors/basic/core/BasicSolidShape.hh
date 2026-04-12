#pragma once

namespace MD1 {

enum class BasicSolidShape {
    Cube,
    Cylinder,
    Sphere,
};

const char* BasicSolidShapeToString(BasicSolidShape shape);

} // namespace MD1
