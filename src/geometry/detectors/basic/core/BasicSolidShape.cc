#include "geometry/detectors/basic/core/BasicSolidShape.hh"

namespace MD1 {

const char* BasicSolidShapeToString(BasicSolidShape shape) {
    switch (shape) {
    case BasicSolidShape::Cube:
        return "cube";
    case BasicSolidShape::Cylinder:
        return "cylinder";
    case BasicSolidShape::Sphere:
        return "sphere";
    }
    return "unknown";
}

} // namespace MD1
