#include "geometry/detectors/model11/geometry/DetectorModel11.hh"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <sstream>
#include <utility>

#include "G4Box.hh"
#include "G4DisplacedSolid.hh"
#include "G4Exception.hh"
#include "G4GeometryTolerance.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4MultiUnion.hh"
#include "G4Polyhedron.hh"
#include "G4PolyhedronArbitrary.hh"
#include "G4PVPlacement.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4ReflectedSolid.hh"
#include "G4ScaledSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4TessellatedSolid.hh"
#include "G4TriangularFacet.hh"
#include "G4UnionSolid.hh"
#include "G4VGraphicsScene.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "Randomize.hh"

#ifdef MDSIM1_HAS_VTK_MODEL11_SPLIT
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkClipClosedSurface.h>
#include <vtkCleanPolyData.h>
#include <vtkFeatureEdges.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTriangle.h>
#include <vtkTriangleFilter.h>
#endif

#include "geometry/detectors/model11/messenger/DetectorModel11Messenger.hh"
#include "geometry/gdml/GDMLColorCodec.hh"
#include "geometry/gdml/GeometryAuxiliaryRegistry.hh"

namespace {

G4String TrimVolumeName(const G4String& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == G4String::npos) {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

G4String JoinVolumeNames(const std::set<G4String>& names) {
    std::ostringstream stream;
    G4bool first = true;
    for (const auto& name : names) {
        if (!first) {
            stream << ", ";
        }
        stream << name;
        first = false;
    }
    return stream.str();
}

G4GDMLAuxListType FlattenAuxiliaryList(const G4GDMLAuxListType& auxiliaries) {
    G4GDMLAuxListType flatCopy;
    flatCopy.reserve(auxiliaries.size());
    for (const auto& aux : auxiliaries) {
        flatCopy.push_back(G4GDMLAuxStructType{aux.type, aux.value, aux.unit, nullptr});
    }
    return flatCopy;
}

struct Bounds3D {
    G4ThreeVector min;
    G4ThreeVector max;
    G4bool valid = false;
};

enum class SplitRegion {
    InsideWater,
    OutsideWater,
    CrossingInterface
};

enum class SplitHalf {
    Water,
    Air
};

struct SplitPlane {
    G4ThreeVector origin;
    G4ThreeVector normal;
};

struct ImportedPartRuntimePlacement {
    const MD1::GDMLAssemblyPart* part = nullptr;
    G4String sourcePhysicalName;
    G4String rootPhysicalName;
    G4Transform3D localTransform;
    G4Transform3D waterTransform;
    G4Transform3D worldTransform;
    SplitRegion splitRegion = SplitRegion::InsideWater;
};

constexpr G4double kModel11SplitRotationTolerance = 1e-6;
constexpr G4double kModel11SplitMinimumFacetEdgeLength = 1.e-4 * mm;
constexpr G4double kModel11SplitMinimumFacetDoubleArea = 1.e-6 * mm * mm;

G4double GetSplitClipMargin() {
    const auto* geometryTolerance = G4GeometryTolerance::GetInstance();
    const G4double surfaceTolerance =
        (geometryTolerance != nullptr) ? geometryTolerance->GetSurfaceTolerance() : 0.;
    return std::max(10. * surfaceTolerance, 1.e-3 * mm);
}

G4double GetSplitClipSearchStep() {
    return GetSplitClipMargin();
}

void ExpandBounds(Bounds3D& bounds, const G4ThreeVector& point) {
    if (!bounds.valid) {
        bounds.min = point;
        bounds.max = point;
        bounds.valid = true;
        return;
    }

    bounds.min.setX(std::min(bounds.min.x(), point.x()));
    bounds.min.setY(std::min(bounds.min.y(), point.y()));
    bounds.min.setZ(std::min(bounds.min.z(), point.z()));
    bounds.max.setX(std::max(bounds.max.x(), point.x()));
    bounds.max.setY(std::max(bounds.max.y(), point.y()));
    bounds.max.setZ(std::max(bounds.max.z(), point.z()));
}

void ExpandBoundsWithSolid(const G4VSolid* solid,
                           const G4Transform3D& transform,
                           Bounds3D& bounds) {
    if (solid == nullptr) {
        return;
    }

    G4ThreeVector localMin;
    G4ThreeVector localMax;
    solid->BoundingLimits(localMin, localMax);

    const std::array<G4double, 2> xs = {localMin.x(), localMax.x()};
    const std::array<G4double, 2> ys = {localMin.y(), localMax.y()};
    const std::array<G4double, 2> zs = {localMin.z(), localMax.z()};
    for (const auto x : xs) {
        for (const auto y : ys) {
            for (const auto z : zs) {
                ExpandBounds(bounds,
                             transform.getTranslation() +
                                 transform.getRotation() * G4ThreeVector(x, y, z));
            }
        }
    }
}

void ExpandBounds(Bounds3D& target, const Bounds3D& source) {
    if (!source.valid) {
        return;
    }

    ExpandBounds(target, source.min);
    ExpandBounds(target, source.max);
}

void AccumulateSubtreeBounds(const G4LogicalVolume* logicalVolume,
                             const G4Transform3D& transform,
                             Bounds3D& bounds) {
    if (logicalVolume == nullptr) {
        return;
    }

    ExpandBoundsWithSolid(logicalVolume->GetSolid(), transform, bounds);
    for (G4int daughterIndex = 0; daughterIndex < logicalVolume->GetNoDaughters(); ++daughterIndex) {
        const auto* daughter = logicalVolume->GetDaughter(daughterIndex);
        if (daughter == nullptr) {
            continue;
        }

        const G4Transform3D daughterTransform(daughter->GetObjectRotationValue(),
                                              daughter->GetObjectTranslation());
        AccumulateSubtreeBounds(daughter->GetLogicalVolume(), transform * daughterTransform, bounds);
    }
}

SplitRegion ClassifyBoundsRelativeToWaterBox(const Bounds3D& bounds,
                                             const G4Box& waterBoxSolid) {
    const G4double waterMinX = -waterBoxSolid.GetXHalfLength();
    const G4double waterMaxX = waterBoxSolid.GetXHalfLength();
    const G4double waterMinY = -waterBoxSolid.GetYHalfLength();
    const G4double waterMaxY = waterBoxSolid.GetYHalfLength();
    const G4double waterMinZ = -waterBoxSolid.GetZHalfLength();
    const G4double interfaceZ = waterBoxSolid.GetZHalfLength();

    const G4bool fullyInsideWater =
        bounds.min.x() >= waterMinX && bounds.max.x() <= waterMaxX &&
        bounds.min.y() >= waterMinY && bounds.max.y() <= waterMaxY &&
        bounds.min.z() >= waterMinZ && bounds.max.z() <= interfaceZ;
    if (fullyInsideWater) {
        return SplitRegion::InsideWater;
    }

    const G4bool fullyOutsideWater =
        bounds.max.x() <= waterMinX || bounds.min.x() >= waterMaxX ||
        bounds.max.y() <= waterMinY || bounds.min.y() >= waterMaxY ||
        bounds.max.z() <= waterMinZ || bounds.min.z() >= interfaceZ;
    if (fullyOutsideWater) {
        return SplitRegion::OutsideWater;
    }

    return SplitRegion::CrossingInterface;
}

G4String ResolveRuntimePhysicalName(const MD1::GDMLAssemblyPart& part,
                                    const MD1::GDMLImportedAssembly& assembly) {
    if (!part.runtimePhysicalName.empty()) {
        return part.runtimePhysicalName;
    }
    if (part.physicalVolume != nullptr && !part.physicalVolume->GetName().empty()) {
        return part.physicalVolume->GetName();
    }
    if (!part.name.empty()) {
        return part.name;
    }
    return assembly.GetRootVolumeName();
}

G4ThreeVector TransformPointToLocal(const G4Transform3D& transform,
                                    const G4ThreeVector& globalPoint) {
    const auto inverseRotation = transform.getRotation().inverse();
    return inverseRotation * (globalPoint - transform.getTranslation());
}

G4ThreeVector TransformDirectionToLocal(const G4Transform3D& transform,
                                        const G4ThreeVector& globalDirection) {
    return transform.getRotation().inverse() * globalDirection;
}

SplitPlane BuildPartLocalSplitPlane(const G4Transform3D& partWaterTransform, const G4double interfaceZ) {
    SplitPlane plane;
    plane.origin = TransformPointToLocal(partWaterTransform, G4ThreeVector(0., 0., interfaceZ));
    plane.normal = TransformDirectionToLocal(partWaterTransform, G4ThreeVector(0., 0., 1.));

    const G4double magnitude = plane.normal.mag();
    if (magnitude <= 0.) {
        G4Exception("DetectorModel11::BuildPartLocalSplitPlane",
                    "DetectorModel11SplitPlaneInvalid",
                    FatalException,
                    "model11 split-at-interface produced an invalid clipping normal.");
        return {};
    }

    plane.normal /= magnitude;
    return plane;
}

#ifdef MDSIM1_HAS_VTK_MODEL11_SPLIT

vtkSmartPointer<vtkPolyData> BuildPolyDataFromSolidOrThrow(const G4VSolid& solid,
                                                           const G4String& solidName) {
    auto* polyhedron = solid.GetPolyhedron();
    if (polyhedron == nullptr || polyhedron->GetNoFacets() <= 0) {
        G4Exception("DetectorModel11::BuildPolyDataFromSolidOrThrow",
                    "DetectorModel11SplitPolyhedronUnavailable",
                    FatalException,
                    ("model11 split-at-interface could not obtain a polyhedron for solid '" +
                     solidName + "'.")
                        .c_str());
        return nullptr;
    }

    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(polyhedron->GetNoVertices());
    for (G4int vertexIndex = 1; vertexIndex <= polyhedron->GetNoVertices(); ++vertexIndex) {
        const auto vertex = polyhedron->GetVertex(vertexIndex);
        points->SetPoint(vertexIndex - 1, vertex.x(), vertex.y(), vertex.z());
    }

    auto polygons = vtkSmartPointer<vtkCellArray>::New();
    for (G4int facetIndex = 1; facetIndex <= polyhedron->GetNoFacets(); ++facetIndex) {
        G4int vertexCount = 0;
        G4int nodeIndices[4] = {0, 0, 0, 0};
        polyhedron->GetFacet(facetIndex, vertexCount, nodeIndices);

        if (vertexCount < 3 || vertexCount > 4) {
            G4Exception("DetectorModel11::BuildPolyDataFromSolidOrThrow",
                        "DetectorModel11SplitFacetUnsupported",
                        FatalException,
                        ("model11 split-at-interface found a facet with unsupported vertex count " +
                         std::to_string(vertexCount) + " in solid '" + solidName + "'.")
                            .c_str());
            return nullptr;
        }

        vtkIdType pointIds[4] = {0, 0, 0, 0};
        for (G4int nodeIndex = 0; nodeIndex < vertexCount; ++nodeIndex) {
            pointIds[nodeIndex] = nodeIndices[nodeIndex] - 1;
        }
        polygons->InsertNextCell(vertexCount, pointIds);
    }

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polygons);

    auto clean = vtkSmartPointer<vtkCleanPolyData>::New();
    clean->SetInputData(polyData);
    clean->Update();

    auto output = vtkSmartPointer<vtkPolyData>::New();
    output->DeepCopy(clean->GetOutput());
    return output;
}

int CountFeatureEdges(vtkPolyData* polyData,
                      const bool countBoundaryEdges,
                      const bool countNonManifoldEdges) {
    auto featureEdges = vtkSmartPointer<vtkFeatureEdges>::New();
    featureEdges->SetInputData(polyData);
    featureEdges->FeatureEdgesOff();
    featureEdges->ManifoldEdgesOff();
    if (countBoundaryEdges) {
        featureEdges->BoundaryEdgesOn();
    } else {
        featureEdges->BoundaryEdgesOff();
    }
    if (countNonManifoldEdges) {
        featureEdges->NonManifoldEdgesOn();
    } else {
        featureEdges->NonManifoldEdgesOff();
    }
    featureEdges->Update();
    return featureEdges->GetOutput()->GetNumberOfCells();
}

vtkSmartPointer<vtkPolyData> ClipClosedSurface(vtkPolyData* source,
                                               const SplitPlane& plane,
                                               const G4double normalScale,
                                               const bool insideOut,
                                               const G4double offset,
                                               const G4double cleanTolerance) {
    const G4ThreeVector shiftedOrigin = plane.origin + offset * plane.normal;
    const G4ThreeVector shiftedNormal = normalScale * plane.normal;

    auto vtkPlaneObject = vtkSmartPointer<vtkPlane>::New();
    vtkPlaneObject->SetOrigin(shiftedOrigin.x(), shiftedOrigin.y(), shiftedOrigin.z());
    vtkPlaneObject->SetNormal(shiftedNormal.x(), shiftedNormal.y(), shiftedNormal.z());

    auto planes = vtkSmartPointer<vtkPlaneCollection>::New();
    planes->AddItem(vtkPlaneObject);

    auto clipper = vtkSmartPointer<vtkClipClosedSurface>::New();
    clipper->SetInputData(source);
    clipper->SetClippingPlanes(planes);
    clipper->SetGenerateClipFaceOutput(true);
    clipper->SetInsideOut(insideOut);
    clipper->Update();

    auto clean = vtkSmartPointer<vtkCleanPolyData>::New();
    clean->SetInputConnection(clipper->GetOutputPort());
    clean->ToleranceIsAbsoluteOn();
    clean->SetAbsoluteTolerance(cleanTolerance);
    clean->Update();

    auto triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInputConnection(clean->GetOutputPort());
    triangleFilter->Update();

    auto cleanTriangles = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanTriangles->SetInputConnection(triangleFilter->GetOutputPort());
    cleanTriangles->ToleranceIsAbsoluteOn();
    cleanTriangles->SetAbsoluteTolerance(cleanTolerance);
    cleanTriangles->Update();

    auto normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputConnection(cleanTriangles->GetOutputPort());
    normals->ConsistencyOn();
    normals->AutoOrientNormalsOn();
    normals->SplittingOff();
    normals->ComputeCellNormalsOn();
    normals->ComputePointNormalsOff();
    normals->Update();

    auto output = vtkSmartPointer<vtkPolyData>::New();
    output->DeepCopy(normals->GetOutput());
    return output;
}

bool IsWatertightClosedSurface(vtkPolyData* polyData) {
    if (polyData == nullptr) {
        return false;
    }

    if (polyData->GetNumberOfPoints() <= 0 || polyData->GetNumberOfPolys() <= 0) {
        return false;
    }

    return CountFeatureEdges(polyData, true, false) == 0 &&
           CountFeatureEdges(polyData, false, true) == 0;
}

struct VisualizationTriangle {
    G4ThreeVector v0;
    G4ThreeVector v1;
    G4ThreeVector v2;
    G4double area = 0.;
};

struct PreparedTriangle {
    vtkIdType pointId0 = -1;
    vtkIdType pointId1 = -1;
    vtkIdType pointId2 = -1;
    G4ThreeVector v0;
    G4ThreeVector v1;
    G4ThreeVector v2;
    G4double area = 0.;
};

struct PreparedSplitSurface {
    std::vector<PreparedTriangle> triangles;
    std::map<vtkIdType, G4int> vertexRemap;
    std::vector<G4ThreeVector> remappedVertices;
    G4double surfaceArea = 0.;
};

struct SplitVisualizationData {
    vtkSmartPointer<vtkPolyData> clippedPolyData;
    std::unique_ptr<G4TessellatedSolid> tessellatedSolid;
    std::unique_ptr<G4Polyhedron> polyhedron;
    std::vector<VisualizationTriangle> surfaceTriangles;
    G4double surfaceArea = 0.;
};

struct OwnedSolidTree {
    G4VSolid* root = nullptr;
    std::vector<G4VSolid*> ownedSolids;

    OwnedSolidTree() = default;
    OwnedSolidTree(const OwnedSolidTree&) = delete;
    OwnedSolidTree& operator=(const OwnedSolidTree&) = delete;

    OwnedSolidTree(OwnedSolidTree&& other) noexcept
        : root(std::exchange(other.root, nullptr)),
          ownedSolids(std::move(other.ownedSolids)) {}

    OwnedSolidTree& operator=(OwnedSolidTree&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        Reset();
        root = std::exchange(other.root, nullptr);
        ownedSolids = std::move(other.ownedSolids);
        return *this;
    }

    ~OwnedSolidTree() {
        Reset();
    }

    void Reset() {
        if (root != nullptr) {
            delete root;
        }
        ownedSolids.clear();
        root = nullptr;
    }

    G4VSolid* ReleaseRoot() {
        auto* releasedRoot = root;
        root = nullptr;
        ownedSolids.clear();
        return releasedRoot;
    }
};

G4VSolid* DeepCloneSolidTreeNodeOrThrow(const G4VSolid& solid, OwnedSolidTree& tree) {
    auto adoptClone = [&tree](G4VSolid* clonedSolid) -> G4VSolid* {
        if (clonedSolid == nullptr) {
            G4Exception("DetectorModel11::DeepCloneSolidTreeNodeOrThrow",
                        "DetectorModel11SolidCloneFailed",
                        FatalException,
                        "model11 split-at-interface could not clone a required Geant4 solid.");
            return nullptr;
        }

        tree.ownedSolids.push_back(clonedSolid);
        return clonedSolid;
    };

    if (const auto* displacedSolid = dynamic_cast<const G4DisplacedSolid*>(&solid);
        displacedSolid != nullptr) {
        auto* movedSolid =
            DeepCloneSolidTreeNodeOrThrow(*displacedSolid->GetConstituentMovedSolid(), tree);
        return adoptClone(
            new G4DisplacedSolid(solid.GetName(), movedSolid, displacedSolid->GetDirectTransform()));
    }

    if (const auto* scaledSolid = dynamic_cast<const G4ScaledSolid*>(&solid);
        scaledSolid != nullptr) {
        auto* unscaledSolid =
            DeepCloneSolidTreeNodeOrThrow(*scaledSolid->GetUnscaledSolid(), tree);
        return adoptClone(
            new G4ScaledSolid(solid.GetName(), unscaledSolid, scaledSolid->GetScaleTransform()));
    }

    if (const auto* reflectedSolid = dynamic_cast<const G4ReflectedSolid*>(&solid);
        reflectedSolid != nullptr) {
        auto* movedSolid =
            DeepCloneSolidTreeNodeOrThrow(*reflectedSolid->GetConstituentMovedSolid(), tree);
        return adoptClone(new G4ReflectedSolid(
            solid.GetName(), movedSolid, reflectedSolid->GetDirectTransform3D()));
    }

    if (const auto* multiUnionSolid = dynamic_cast<const G4MultiUnion*>(&solid);
        multiUnionSolid != nullptr) {
        auto* clonedMultiUnion = new G4MultiUnion(solid.GetName());
        tree.ownedSolids.push_back(clonedMultiUnion);

        const auto solidCount = multiUnionSolid->GetNumberOfSolids();
        for (G4int solidIndex = 0; solidIndex < solidCount; ++solidIndex) {
            auto* clonedChildSolid =
                DeepCloneSolidTreeNodeOrThrow(*multiUnionSolid->GetSolid(solidIndex), tree);
            clonedMultiUnion->AddNode(clonedChildSolid,
                                      multiUnionSolid->GetTransformation(solidIndex));
        }
        clonedMultiUnion->Voxelize();
        return clonedMultiUnion;
    }

    if (const auto* intersectionSolid = dynamic_cast<const G4IntersectionSolid*>(&solid);
        intersectionSolid != nullptr) {
        auto* solidA =
            DeepCloneSolidTreeNodeOrThrow(*intersectionSolid->GetConstituentSolid(0), tree);
        auto* solidB =
            DeepCloneSolidTreeNodeOrThrow(*intersectionSolid->GetConstituentSolid(1), tree);
        return adoptClone(new G4IntersectionSolid(solid.GetName(), solidA, solidB));
    }

    if (const auto* subtractionSolid = dynamic_cast<const G4SubtractionSolid*>(&solid);
        subtractionSolid != nullptr) {
        auto* solidA =
            DeepCloneSolidTreeNodeOrThrow(*subtractionSolid->GetConstituentSolid(0), tree);
        auto* solidB =
            DeepCloneSolidTreeNodeOrThrow(*subtractionSolid->GetConstituentSolid(1), tree);
        return adoptClone(new G4SubtractionSolid(solid.GetName(), solidA, solidB));
    }

    if (const auto* unionSolid = dynamic_cast<const G4UnionSolid*>(&solid);
        unionSolid != nullptr) {
        auto* solidA = DeepCloneSolidTreeNodeOrThrow(*unionSolid->GetConstituentSolid(0), tree);
        auto* solidB = DeepCloneSolidTreeNodeOrThrow(*unionSolid->GetConstituentSolid(1), tree);
        return adoptClone(new G4UnionSolid(solid.GetName(), solidA, solidB));
    }

    return adoptClone(solid.Clone());
}

OwnedSolidTree BuildDeepClonedSolidTreeOrThrow(const G4VSolid& solid) {
    OwnedSolidTree tree;
    tree.root = DeepCloneSolidTreeNodeOrThrow(solid, tree);
    return tree;
}

PreparedSplitSurface PrepareSplitSurfaceFromPolyData(vtkPolyData* polyData) {
    PreparedSplitSurface preparedSurface;
    if (polyData == nullptr || polyData->GetNumberOfPoints() <= 0 ||
        polyData->GetNumberOfPolys() <= 0) {
        return preparedSurface;
    }

    auto* polygons = polyData->GetPolys();
    polygons->InitTraversal();
    auto* cellNormals = polyData->GetCellData()->GetNormals();
    preparedSurface.triangles.reserve(static_cast<std::size_t>(polyData->GetNumberOfPolys()));

    vtkIdType pointCount = 0;
    const vtkIdType* pointIds = nullptr;
    vtkIdType cellIndex = 0;
    while (polygons->GetNextCell(pointCount, pointIds)) {
        if (pointCount != 3) {
            ++cellIndex;
            continue;
        }

        double p0[3] = {0., 0., 0.};
        double p1[3] = {0., 0., 0.};
        double p2[3] = {0., 0., 0.};
        polyData->GetPoint(pointIds[0], p0);
        polyData->GetPoint(pointIds[1], p1);
        polyData->GetPoint(pointIds[2], p2);

        PreparedTriangle preparedTriangle;
        preparedTriangle.pointId0 = pointIds[0];
        preparedTriangle.pointId1 = pointIds[1];
        preparedTriangle.pointId2 = pointIds[2];
        preparedTriangle.v0 = G4ThreeVector(p0[0], p0[1], p0[2]);
        preparedTriangle.v1 = G4ThreeVector(p1[0], p1[1], p1[2]);
        preparedTriangle.v2 = G4ThreeVector(p2[0], p2[1], p2[2]);

        if (cellNormals != nullptr && cellNormals->GetNumberOfTuples() > cellIndex) {
            double normalTuple[3] = {0., 0., 0.};
            cellNormals->GetTuple(cellIndex, normalTuple);
            const G4ThreeVector expectedNormal(normalTuple[0], normalTuple[1], normalTuple[2]);
            if (((preparedTriangle.v1 - preparedTriangle.v0)
                     .cross(preparedTriangle.v2 - preparedTriangle.v0))
                    .dot(expectedNormal) < 0.) {
                std::swap(preparedTriangle.pointId1, preparedTriangle.pointId2);
                std::swap(preparedTriangle.v1, preparedTriangle.v2);
            }
        }

        const G4double edge01 = (preparedTriangle.v1 - preparedTriangle.v0).mag();
        const G4double edge12 = (preparedTriangle.v2 - preparedTriangle.v1).mag();
        const G4double edge20 = (preparedTriangle.v0 - preparedTriangle.v2).mag();
        const G4double doubleArea =
            ((preparedTriangle.v1 - preparedTriangle.v0)
                 .cross(preparedTriangle.v2 - preparedTriangle.v0))
                .mag();
        if (std::min({edge01, edge12, edge20}) <= kModel11SplitMinimumFacetEdgeLength ||
            doubleArea <= kModel11SplitMinimumFacetDoubleArea) {
            ++cellIndex;
            continue;
        }

        preparedTriangle.area = 0.5 * doubleArea;
        preparedSurface.surfaceArea += preparedTriangle.area;
        preparedSurface.triangles.push_back(preparedTriangle);
        for (const auto [pointId, vertex] : {std::pair{preparedTriangle.pointId0, preparedTriangle.v0},
                                             std::pair{preparedTriangle.pointId1, preparedTriangle.v1},
                                             std::pair{preparedTriangle.pointId2, preparedTriangle.v2}}) {
            if (preparedSurface.vertexRemap.find(pointId) ==
                preparedSurface.vertexRemap.end()) {
                const auto remappedIndex =
                    static_cast<G4int>(preparedSurface.remappedVertices.size()) + 1;
                preparedSurface.vertexRemap.emplace(pointId, remappedIndex);
                preparedSurface.remappedVertices.push_back(vertex);
            }
        }

        ++cellIndex;
    }

    return preparedSurface;
}

SplitVisualizationData BuildVisualizationDataFromPolyData(vtkPolyData* polyData,
                                                          const G4String& solidName) {
    (void)solidName;
    SplitVisualizationData data;
    const auto preparedSurface = PrepareSplitSurfaceFromPolyData(polyData);
    if (preparedSurface.triangles.empty() || preparedSurface.remappedVertices.empty()) {
        return {};
    }

    auto arbitraryPolyhedron = std::make_unique<G4PolyhedronArbitrary>(
        static_cast<G4int>(preparedSurface.remappedVertices.size()),
        static_cast<G4int>(preparedSurface.triangles.size()));
    for (const auto& vertex : preparedSurface.remappedVertices) {
        arbitraryPolyhedron->AddVertex(vertex);
    }

    for (const auto& preparedTriangle : preparedSurface.triangles) {
        arbitraryPolyhedron->AddFacet(preparedSurface.vertexRemap.at(preparedTriangle.pointId0),
                                      preparedSurface.vertexRemap.at(preparedTriangle.pointId1),
                                      preparedSurface.vertexRemap.at(preparedTriangle.pointId2));
        data.surfaceTriangles.push_back(
            VisualizationTriangle{preparedTriangle.v0,
                                  preparedTriangle.v1,
                                  preparedTriangle.v2,
                                  preparedTriangle.area});
    }

    data.surfaceArea = preparedSurface.surfaceArea;
    data.polyhedron = std::move(arbitraryPolyhedron);
    return data;
}

std::unique_ptr<G4TessellatedSolid> TryBuildTessellatedSolidFromPolyData(
    vtkPolyData* polyData,
    const G4String& solidName,
    G4int* structureState = nullptr) {
    if (polyData == nullptr || polyData->GetNumberOfPoints() <= 0 ||
        polyData->GetNumberOfPolys() <= 0) {
        return nullptr;
    }

    const auto preparedSurface = PrepareSplitSurfaceFromPolyData(polyData);
    if (preparedSurface.triangles.empty()) {
        return nullptr;
    }

    auto tessellatedSolid = std::make_unique<G4TessellatedSolid>(solidName);
    G4int facetCount = 0;
    for (const auto& preparedTriangle : preparedSurface.triangles) {
        auto* facet = new G4TriangularFacet(
            preparedTriangle.v0, preparedTriangle.v1, preparedTriangle.v2, ABSOLUTE);
        if (!facet->IsDefined() || !tessellatedSolid->AddFacet(facet)) {
            delete facet;
            return nullptr;
        }

        ++facetCount;
    }

    if (facetCount <= 0) {
        return nullptr;
    }

    tessellatedSolid->SetSolidClosed(true);
    const auto candidateStructureState = tessellatedSolid->CheckStructure();
    if (structureState != nullptr) {
        *structureState = candidateStructureState;
    }
    if (candidateStructureState != 0) {
        return nullptr;
    }

    return tessellatedSolid;
}

std::unique_ptr<G4TessellatedSolid> BuildTessellatedSolidFromPolyDataOrThrow(
    vtkPolyData* polyData,
    const G4String& solidName) {
    G4int structureState = 0;
    auto tessellatedSolid =
        TryBuildTessellatedSolidFromPolyData(polyData, solidName, &structureState);
    if (tessellatedSolid != nullptr) {
        return tessellatedSolid;
    }

    G4Exception("DetectorModel11::BuildTessellatedSolidFromPolyDataOrThrow",
                "DetectorModel11SplitTessellatedInvalid",
                FatalException,
                ("model11 split-at-interface produced an invalid tessellated solid '" +
                 solidName + "' with structure code " + std::to_string(structureState) + ".")
                    .c_str());
    return nullptr;
}

SplitVisualizationData BuildSplitVisualizationDataFromVTKOrThrow(const G4VSolid& sourceSolid,
                                                                 const G4String& sourceSolidName,
                                                                 const G4String& outputSolidName,
                                                                 const SplitPlane& plane,
                                                                 const SplitHalf half) {
    auto sourcePolyData = BuildPolyDataFromSolidOrThrow(sourceSolid, sourceSolidName);
    const G4double cleanToleranceBase = std::max(GetSplitClipMargin(), 1.e-3 * mm);

    const G4double step = GetSplitClipSearchStep();
    const std::array<G4double, 15> offsets = {
        0.,
        step,
        -step,
        2. * step,
        -2. * step,
        5. * step,
        -5. * step,
        10. * step,
        -10. * step,
        20. * step,
        -20. * step,
        50. * step,
        -50. * step,
        100. * step,
        -100. * step};
    const std::array<G4double, 5> cleanTolerances = {
        0.,
        cleanToleranceBase,
        2.5 * cleanToleranceBase,
        5. * cleanToleranceBase,
        10. * cleanToleranceBase};

    const bool keepNegativeHalf = (half == SplitHalf::Water);
    for (const auto offset : offsets) {
        for (const auto normalScale : {1., -1.}) {
            const bool insideOut =
                (normalScale > 0.) ? keepNegativeHalf : !keepNegativeHalf;
            for (const auto cleanTolerance : cleanTolerances) {
                auto clipped = ClipClosedSurface(
                    sourcePolyData, plane, normalScale, insideOut, offset, cleanTolerance);
                if (!IsWatertightClosedSurface(clipped)) {
                    continue;
                }

                auto visualizationData =
                    BuildVisualizationDataFromPolyData(clipped, outputSolidName);
                if (visualizationData.polyhedron == nullptr ||
                    visualizationData.surfaceTriangles.empty()) {
                    continue;
                }

                visualizationData.clippedPolyData = clipped;
                return visualizationData;
            }
        }
    }

    G4Exception("DetectorModel11::BuildSplitVisualizationDataFromVTKOrThrow",
                "DetectorModel11SplitVTKClipFailed",
                FatalException,
                ("model11 split-at-interface could not build a visualization polyhedron for "
                 "solid '" +
                 sourceSolidName + "'.")
                    .c_str());
    return {};
}

#endif

class SplitIntersectionSolid final : public G4IntersectionSolid {
public:
    SplitIntersectionSolid(const G4String& name,
                           G4VSolid* sourceRoot,
                           G4VSolid* clipRoot,
                           const G4Transform3D& clipTransform,
                           std::unique_ptr<G4Polyhedron> visualizationPolyhedron,
                           std::vector<VisualizationTriangle> surfaceTriangles,
                           const G4double surfaceArea)
        : G4IntersectionSolid(name, sourceRoot, clipRoot, clipTransform),
          fClipTransform(clipTransform),
          fVisualizationPolyhedron(std::move(visualizationPolyhedron)),
          fSurfaceTriangles(std::move(surfaceTriangles)),
          fSurfaceArea(surfaceArea) {
    }

    G4double GetSurfaceArea() override {
        return (fSurfaceArea > 0.) ? fSurfaceArea : G4IntersectionSolid::GetSurfaceArea();
    }

    G4ThreeVector GetPointOnSurface() const override {
        if (fSurfaceTriangles.empty() || fSurfaceArea <= 0.) {
            return G4IntersectionSolid::GetPointOnSurface();
        }

        const G4double sampledArea = G4UniformRand() * fSurfaceArea;
        const VisualizationTriangle* selectedTriangle = &fSurfaceTriangles.back();
        G4double accumulatedArea = 0.;
        for (const auto& triangle : fSurfaceTriangles) {
            accumulatedArea += triangle.area;
            if (sampledArea <= accumulatedArea) {
                selectedTriangle = &triangle;
                break;
            }
        }

        const G4double barycentricU = std::sqrt(G4UniformRand());
        const G4double barycentricV = G4UniformRand();
        return (1. - barycentricU) * selectedTriangle->v0 +
               barycentricU * (1. - barycentricV) * selectedTriangle->v1 +
               barycentricU * barycentricV * selectedTriangle->v2;
    }

    G4Polyhedron* CreatePolyhedron() const override {
        if (fVisualizationPolyhedron == nullptr) {
            return G4IntersectionSolid::CreatePolyhedron();
        }
        return new G4Polyhedron(*fVisualizationPolyhedron);
    }

    G4VSolid* Clone() const override {
        auto sourceTree = BuildDeepClonedSolidTreeOrThrow(*GetConstituentSolid(0));
        auto clipTree = BuildDeepClonedSolidTreeOrThrow(*GetConstituentSolid(1));
        if (sourceTree.root == nullptr || clipTree.root == nullptr) {
            return nullptr;
        }

        std::unique_ptr<G4Polyhedron> clonedVisualizationPolyhedron;
        if (fVisualizationPolyhedron != nullptr) {
            clonedVisualizationPolyhedron = std::make_unique<G4Polyhedron>(*fVisualizationPolyhedron);
        }

        return new SplitIntersectionSolid(GetName(),
                                          sourceTree.ReleaseRoot(),
                                          clipTree.ReleaseRoot(),
                                          fClipTransform,
                                          std::move(clonedVisualizationPolyhedron),
                                          fSurfaceTriangles,
                                          fSurfaceArea);
    }

private:
    G4Transform3D fClipTransform;
    std::unique_ptr<G4Polyhedron> fVisualizationPolyhedron;
    std::vector<VisualizationTriangle> fSurfaceTriangles;
    G4double fSurfaceArea = 0.;
};

class SplitSubtractionSolid final : public G4SubtractionSolid {
public:
    SplitSubtractionSolid(const G4String& name,
                          G4VSolid* firstRoot,
                          G4VSolid* secondRoot,
                          std::unique_ptr<G4Polyhedron> visualizationPolyhedron,
                          std::vector<VisualizationTriangle> surfaceTriangles,
                          const G4double surfaceArea)
        : G4SubtractionSolid(name, firstRoot, secondRoot),
          fVisualizationPolyhedron(std::move(visualizationPolyhedron)),
          fSurfaceTriangles(std::move(surfaceTriangles)),
          fSurfaceArea(surfaceArea) {
    }

    G4double GetSurfaceArea() override {
        return (fSurfaceArea > 0.) ? fSurfaceArea : G4SubtractionSolid::GetSurfaceArea();
    }

    G4ThreeVector GetPointOnSurface() const override {
        if (fSurfaceTriangles.empty() || fSurfaceArea <= 0.) {
            return G4SubtractionSolid::GetPointOnSurface();
        }

        const G4double sampledArea = G4UniformRand() * fSurfaceArea;
        const VisualizationTriangle* selectedTriangle = &fSurfaceTriangles.back();
        G4double accumulatedArea = 0.;
        for (const auto& triangle : fSurfaceTriangles) {
            accumulatedArea += triangle.area;
            if (sampledArea <= accumulatedArea) {
                selectedTriangle = &triangle;
                break;
            }
        }

        const G4double barycentricU = std::sqrt(G4UniformRand());
        const G4double barycentricV = G4UniformRand();
        return (1. - barycentricU) * selectedTriangle->v0 +
               barycentricU * (1. - barycentricV) * selectedTriangle->v1 +
               barycentricU * barycentricV * selectedTriangle->v2;
    }

    G4Polyhedron* CreatePolyhedron() const override {
        if (fVisualizationPolyhedron == nullptr) {
            return G4SubtractionSolid::CreatePolyhedron();
        }
        return new G4Polyhedron(*fVisualizationPolyhedron);
    }

    G4VSolid* Clone() const override {
        auto firstTree = BuildDeepClonedSolidTreeOrThrow(*GetConstituentSolid(0));
        auto secondTree = BuildDeepClonedSolidTreeOrThrow(*GetConstituentSolid(1));
        if (firstTree.root == nullptr || secondTree.root == nullptr) {
            return nullptr;
        }

        std::unique_ptr<G4Polyhedron> clonedVisualizationPolyhedron;
        if (fVisualizationPolyhedron != nullptr) {
            clonedVisualizationPolyhedron = std::make_unique<G4Polyhedron>(*fVisualizationPolyhedron);
        }

        return new SplitSubtractionSolid(GetName(),
                                         firstTree.ReleaseRoot(),
                                         secondTree.ReleaseRoot(),
                                         std::move(clonedVisualizationPolyhedron),
                                         fSurfaceTriangles,
                                         fSurfaceArea);
    }

private:
    std::unique_ptr<G4Polyhedron> fVisualizationPolyhedron;
    std::vector<VisualizationTriangle> fSurfaceTriangles;
    G4double fSurfaceArea = 0.;
};

Bounds3D GetSolidLocalBounds(const G4VSolid& solid) {
    Bounds3D bounds;
    G4ThreeVector localMin;
    G4ThreeVector localMax;
    solid.BoundingLimits(localMin, localMax);
    bounds.min = localMin;
    bounds.max = localMax;
    bounds.valid = true;
    return bounds;
}

std::pair<std::unique_ptr<G4Box>, G4Transform3D> BuildSplitClipBox(
    const G4String& clipBoxName,
    const G4VSolid& sourceSolid,
    const SplitPlane& plane,
    const SplitHalf half) {
    const auto bounds = GetSolidLocalBounds(sourceSolid);
    const G4double margin = GetSplitClipMargin();

    if (!bounds.valid) {
        G4Exception("DetectorModel11::BuildSplitClipBox",
                    "DetectorModel11SplitBoundsUnavailable",
                    FatalException,
                    "model11 split-at-interface could not determine local solid bounds.");
        return {};
    }

    if (std::abs(plane.normal.x()) > kModel11SplitRotationTolerance ||
        std::abs(plane.normal.z()) > kModel11SplitRotationTolerance ||
        std::abs(std::abs(plane.normal.y()) - 1.) > kModel11SplitRotationTolerance) {
        G4Exception("DetectorModel11::BuildSplitClipBox",
                    "DetectorModel11SplitPlaneOrientationUnsupported",
                    FatalException,
                    "model11 split-at-interface only supports clip planes aligned with local Y.");
        return {};
    }

    const G4double clipExtent =
        std::max({std::abs(bounds.min.x()),
                  std::abs(bounds.max.x()),
                  std::abs(bounds.min.y()),
                  std::abs(bounds.max.y()),
                  std::abs(bounds.min.z()),
                  std::abs(bounds.max.z()),
                  std::abs(plane.origin.y())}) +
        20. * margin;
    const G4bool negativeHalfIsLowerY = plane.normal.y() >= 0.;
    const G4bool keepNegativeHalf = (half == SplitHalf::Water);
    const G4bool keepLowerHalf =
        (keepNegativeHalf && negativeHalfIsLowerY) || (!keepNegativeHalf && !negativeHalfIsLowerY);

    const G4double lowerY = keepLowerHalf ? -clipExtent : plane.origin.y();
    const G4double upperY = keepLowerHalf ? plane.origin.y() : clipExtent;
    if (upperY - lowerY <= margin) {
        G4Exception("DetectorModel11::BuildSplitClipBox",
                    "DetectorModel11SplitClipBoxInvalid",
                    FatalException,
                    "model11 split-at-interface produced a degenerate local clip box.");
        return {};
    }

    const G4double halfLengthY = 0.5 * (upperY - lowerY);
    const G4double centerY = 0.5 * (upperY + lowerY);
    auto clipBox =
        std::make_unique<G4Box>(clipBoxName, clipExtent, halfLengthY, clipExtent);
    return {std::move(clipBox), G4Translate3D(0., centerY, 0.)};
}

std::unique_ptr<G4VSolid> BuildSplitBodySolidOrThrow(
    const G4VSolid& sourceSolid,
    const G4String& outputSolidName,
    const SplitPlane& plane,
    const SplitHalf half,
    SplitVisualizationData visualizationData) {
    const auto* bodySolid = dynamic_cast<const G4SubtractionSolid*>(&sourceSolid);
    if (bodySolid == nullptr) {
        G4Exception("DetectorModel11::BuildSplitBodySolidOrThrow",
                    "DetectorModel11SplitBodyUnexpectedSolid",
                    FatalException,
                    "model11 split-at-interface expected LV_Body to be a subtraction solid.");
        return nullptr;
    }

    OwnedSolidTree outerTree =
        BuildDeepClonedSolidTreeOrThrow(*bodySolid->GetConstituentSolid(0));
    auto [outerClipBox, outerClipTransform] =
        BuildSplitClipBox(outputSolidName + "_outer_clip_box",
                          *bodySolid->GetConstituentSolid(0),
                          plane,
                          half);
    auto* outerClipBoxRaw = outerClipBox.release();
    outerTree.ownedSolids.push_back(outerClipBoxRaw);
    auto* clippedOuter = new G4IntersectionSolid(
        outputSolidName + "_outer_clip", outerTree.root, outerClipBoxRaw, outerClipTransform);
    outerTree.ownedSolids.push_back(clippedOuter);
    outerTree.root = clippedOuter;

    OwnedSolidTree cavityTree =
        BuildDeepClonedSolidTreeOrThrow(*bodySolid->GetConstituentSolid(1));

    return std::make_unique<SplitSubtractionSolid>(outputSolidName,
                                                   outerTree.ReleaseRoot(),
                                                   cavityTree.ReleaseRoot(),
                                                   std::move(visualizationData.polyhedron),
                                                   std::move(visualizationData.surfaceTriangles),
                                                   visualizationData.surfaceArea);
}

std::unique_ptr<G4VSolid> BuildSplitSolidOrThrow(const G4VSolid& sourceSolid,
                                                 const G4String& sourceSolidName,
                                                 const G4String& outputSolidName,
                                                 const SplitPlane& plane,
                                                 const SplitHalf half) {
#ifdef MDSIM1_HAS_VTK_MODEL11_SPLIT
    auto visualizationData = BuildSplitVisualizationDataFromVTKOrThrow(sourceSolid,
                                                                       sourceSolidName,
                                                                       outputSolidName,
                                                                       plane,
                                                                       half);
    if (sourceSolidName == "LV_Body") {
        return BuildSplitBodySolidOrThrow(
            sourceSolid, outputSolidName, plane, half, std::move(visualizationData));
    }

    auto sourceTree = BuildDeepClonedSolidTreeOrThrow(sourceSolid);
    auto [clipBox, clipTransform] =
        BuildSplitClipBox(outputSolidName + "_clip_box", sourceSolid, plane, half);

    OwnedSolidTree clipTree;
    clipTree.root = clipBox.release();
    clipTree.ownedSolids.push_back(clipTree.root);

    return std::make_unique<SplitIntersectionSolid>(outputSolidName,
                                                    sourceTree.ReleaseRoot(),
                                                    clipTree.ReleaseRoot(),
                                                    clipTransform,
                                                    std::move(visualizationData.polyhedron),
                                                    std::move(visualizationData.surfaceTriangles),
                                                    visualizationData.surfaceArea);
#else
    (void)sourceSolid;
    (void)sourceSolidName;
    (void)outputSolidName;
    (void)plane;
    (void)half;
    G4Exception("DetectorModel11::BuildSplitSolidOrThrow",
                "DetectorModel11SplitRequiresVTK",
                FatalException,
                "model11 split-at-interface across the WaterBox surface requires a VTK-enabled build.");
    return nullptr;
#endif
}

} // namespace

DetectorModel11::DetectorModel11()
    : fDetectorModel11Messenger(nullptr),
      fSensitiveDetector(nullptr) {
    geometryName = "DetectorModel11";
    det_origin = G4ThreeVector(0., 0., 10. * cm);
    fDetectorModel11Messenger = new DetectorModel11Messenger(this);
}

DetectorModel11::~DetectorModel11() {
    fImportedGDMLCache.Clear();
    delete fDetectorModel11Messenger;
    fDetectorModel11Messenger = nullptr;
}

void DetectorModel11::DefineMaterials() {}

void DetectorModel11::DefineVolumes() {
    fAreVolumensDefined = true;
}

Model11DetectorConfig& DetectorModel11::EnsureDetectorConfig(G4int detectorID) {
    const auto it = fDetectorConfigs.find(detectorID);
    if (it != fDetectorConfigs.end()) {
        return it->second;
    }

    auto [insertedIt, inserted] = fDetectorConfigs.emplace(detectorID, fDefaultConfig);
    (void)inserted;
    return insertedIt->second;
}

void DetectorModel11::SetSplitAtInterface(G4int detectorID, G4bool splitAtInterface) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.splitAtInterface = splitAtInterface;
    fDefaultConfig = config;
}

void DetectorModel11::SetScintillationYield(G4int detectorID, G4double scintillationYieldPerMeV) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.scintillationYield = scintillationYieldPerMeV / MeV;
    fDefaultConfig = config;
}

void DetectorModel11::SetBirksConstant(G4int detectorID, G4double birksConstantInMmPerMeV) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.birksConstant = birksConstantInMmPerMeV * mm / MeV;
    fDefaultConfig = config;
}

void DetectorModel11::SetLightCollectionEfficiency(G4int detectorID, G4double lightCollectionEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.lightCollectionEfficiency = lightCollectionEfficiency;
    fDefaultConfig = config;
}

void DetectorModel11::SetDecayTime(G4int detectorID, G4double decayTime) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.decayTime = decayTime;
    fDefaultConfig = config;
}

void DetectorModel11::SetTransportDelay(G4int detectorID, G4double transportDelay) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.transportDelay = transportDelay;
    fDefaultConfig = config;
}

void DetectorModel11::SetTimeJitter(G4int detectorID, G4double timeJitter) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.timeJitter = timeJitter;
    fDefaultConfig = config;
}

void DetectorModel11::SetResolutionScale(G4int detectorID, G4double resolutionScale) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.resolutionScale = resolutionScale;
    fDefaultConfig = config;
}

void DetectorModel11::SetPhotosensorType(G4int detectorID, Model11PhotosensorType photosensorType) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.photosensorType = photosensorType;
    fDefaultConfig = config;
}

void DetectorModel11::SetPMTQuantumEfficiency(G4int detectorID, G4double quantumEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.quantumEfficiency = quantumEfficiency;
    fDefaultConfig = config;
}

void DetectorModel11::SetPMTDynodeCollectionEfficiency(G4int detectorID,
                                                       G4double dynodeCollectionEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.dynodeCollectionEfficiency = dynodeCollectionEfficiency;
    fDefaultConfig = config;
}

void DetectorModel11::SetPMTTransitTime(G4int detectorID, G4double transitTime) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.transitTime = transitTime;
    fDefaultConfig = config;
}

void DetectorModel11::SetPMTTransitTimeSpread(G4int detectorID, G4double transitTimeSpread) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.pmtParameters.transitTimeSpread = transitTimeSpread;
    fDefaultConfig = config;
}

void DetectorModel11::SetSiPMPDE(G4int detectorID, G4double photoDetectionEfficiency) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.photoDetectionEfficiency = photoDetectionEfficiency;
    fDefaultConfig = config;
}

void DetectorModel11::SetSiPMMicrocellCount(G4int detectorID, G4double microcellCount) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.microcellCount = microcellCount;
    fDefaultConfig = config;
}

void DetectorModel11::SetSiPMExcessNoiseFactor(G4int detectorID, G4double excessNoiseFactor) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.excessNoiseFactor = excessNoiseFactor;
    fDefaultConfig = config;
}

void DetectorModel11::SetSiPMAvalancheTime(G4int detectorID, G4double avalancheTime) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.avalancheTime = avalancheTime;
    fDefaultConfig = config;
}

void DetectorModel11::SetSiPMAvalancheTimeSpread(G4int detectorID, G4double avalancheTimeSpread) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.readoutParameters.sipmParameters.avalancheTimeSpread = avalancheTimeSpread;
    fDefaultConfig = config;
}

void DetectorModel11::SetDoseCalibrationFactor(G4int detectorID,
                                               G4double doseCalibrationFactorInGyPerPhotoelectron) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.calibrationParameters.doseCalibrationFactor = doseCalibrationFactorInGyPerPhotoelectron;
    fDefaultConfig = config;
}

void DetectorModel11::SetDoseCalibrationFactorError(
    G4int detectorID,
    G4double doseCalibrationFactorErrorInGyPerPhotoelectron) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.calibrationParameters.doseCalibrationFactorError =
        doseCalibrationFactorErrorInGyPerPhotoelectron;
    fDefaultConfig = config;
}

void DetectorModel11::SetImportedGeometryGDMLPath(G4int detectorID, const G4String& gdmlPath) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryGDMLPath = gdmlPath;
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::SetImportedGeometryRootLogicalName(G4int detectorID, const G4String& rootName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryRootSelector =
        MD1::GDMLRootSelector{MD1::GDMLRootSelectorType::Logical, TrimVolumeName(rootName)};
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::SetImportedGeometryRootPhysicalName(G4int detectorID, const G4String& rootName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryRootSelector =
        MD1::GDMLRootSelector{MD1::GDMLRootSelectorType::Physical, TrimVolumeName(rootName)};
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::SetImportedGeometryRootAssemblyName(G4int detectorID, const G4String& rootName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryRootSelector =
        MD1::GDMLRootSelector{MD1::GDMLRootSelectorType::Assembly, TrimVolumeName(rootName)};
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::SetImportedGeometryValidate(G4int detectorID, G4bool validate) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryReadOptions.validate = validate;
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::SetImportedGeometrySchema(G4int detectorID, const G4String& schemaPath) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.importedGeometryReadOptions.schemaPath = TrimVolumeName(schemaPath);
    fDefaultConfig = config;
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::AddSensitiveVolume(G4int detectorID, const G4String& logicalVolumeName) {
    const auto normalizedName = TrimVolumeName(logicalVolumeName);
    if (normalizedName.empty()) {
        G4Exception("DetectorModel11::AddSensitiveVolume",
                    "DetectorModel11InvalidSensitiveVolume",
                    FatalException,
                    "Sensitive volume name for model11 cannot be empty.");
        return;
    }

    auto& config = EnsureDetectorConfig(detectorID);
    config.sensitiveVolumeNames.insert(normalizedName);
    fDefaultConfig = config;
}

void DetectorModel11::RemoveSensitiveVolume(G4int detectorID, const G4String& logicalVolumeName) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.sensitiveVolumeNames.erase(TrimVolumeName(logicalVolumeName));
    fDefaultConfig = config;
}

void DetectorModel11::ClearSensitiveVolumes(G4int detectorID) {
    auto& config = EnsureDetectorConfig(detectorID);
    config.sensitiveVolumeNames.clear();
    fDefaultConfig = config;
}

const Model11DetectorConfig& DetectorModel11::GetDetectorConfig(G4int detectorID) const {
    const auto it = fDetectorConfigs.find(detectorID);
    return (it != fDetectorConfigs.end()) ? it->second : fDefaultConfig;
}

std::map<G4int, Model11ReadoutParameters> DetectorModel11::GetReadoutParametersByDetector() const {
    std::map<G4int, Model11ReadoutParameters> parametersByDetector;
    for (const auto copyNo : GetPlacementCopyNumbers()) {
        parametersByDetector.emplace(copyNo, GetDetectorConfig(copyNo).readoutParameters);
    }
    return parametersByDetector;
}

Model11CalibrationParameters DetectorModel11::GetCalibrationParameters(G4int detectorID) const {
    return GetDetectorConfig(detectorID).calibrationParameters;
}

G4bool DetectorModel11::HasImportedGeometry(G4int detectorID) const {
    return ShouldBuildImportedGeometry(GetDetectorConfig(detectorID));
}

std::size_t DetectorModel11::GetImportedGeometryPartCount(G4int detectorID) const {
    const auto& config = GetDetectorConfig(detectorID);
    if (!ShouldBuildImportedGeometry(config)) {
        return 0;
    }

    return LoadImportedGDMLAssembly(config)->GetPartCount();
}

G4String DetectorModel11::GetImportedGeometryRootVolumeName(G4int detectorID) const {
    const auto& config = GetDetectorConfig(detectorID);
    if (!ShouldBuildImportedGeometry(config)) {
        return "none";
    }

    return LoadImportedGDMLAssembly(config)->GetRootVolumeName();
}

std::vector<G4String> DetectorModel11::GetSensitiveVolumeNames(G4int detectorID) const {
    const auto sensitiveNames = ResolveSensitiveVolumeNames(GetDetectorConfig(detectorID));
    return std::vector<G4String>(sensitiveNames.begin(), sensitiveNames.end());
}

G4bool DetectorModel11::ShouldBuildImportedGeometry(const Model11DetectorConfig& config) const {
    return !config.importedGeometryGDMLPath.empty();
}

G4bool DetectorModel11::ShouldActivateSensitiveImportedVolumes(
    const std::set<G4String>& sensitiveVolumeNames) const {
    return !sensitiveVolumeNames.empty();
}

std::shared_ptr<const MD1::GDMLImportedAssembly> DetectorModel11::LoadImportedGDMLAssembly(
    const Model11DetectorConfig& config) const {
    if (!ShouldBuildImportedGeometry(config)) {
        return nullptr;
    }

    return fImportedGDMLCache.Load(config.importedGeometryGDMLPath,
                                   config.importedGeometryRootSelector,
                                   config.importedGeometryReadOptions);
}

std::set<G4String> DetectorModel11::ResolveSensitiveVolumeNames(
    const Model11DetectorConfig& config) const {
    if (!config.sensitiveVolumeNames.empty()) {
        return config.sensitiveVolumeNames;
    }

    if (!ShouldBuildImportedGeometry(config)) {
        return {};
    }

    const auto assembly = LoadImportedGDMLAssembly(config);
    if (assembly == nullptr) {
        return {};
    }

    const auto auxSensitiveNames = assembly->GetAuxSensitiveVolumeNames();
    return std::set<G4String>(auxSensitiveNames.begin(), auxSensitiveNames.end());
}

void DetectorModel11::ValidateSensitiveVolumeSelection(
    const MD1::GDMLImportedAssembly& importedAssembly,
    const std::set<G4String>& sensitiveVolumeNames) const {
    if (!ShouldActivateSensitiveImportedVolumes(sensitiveVolumeNames)) {
        return;
    }

    std::set<G4String> availableVolumeNames;
    for (const auto& availableName : importedAssembly.GetAvailableVolumeNames()) {
        availableVolumeNames.insert(availableName);
    }

    std::set<G4String> unresolvedVolumeNames;
    for (const auto& selectedName : sensitiveVolumeNames) {
        if (availableVolumeNames.find(selectedName) == availableVolumeNames.end()) {
            unresolvedVolumeNames.insert(selectedName);
        }
    }

    if (!unresolvedVolumeNames.empty()) {
        G4Exception("DetectorModel11::ValidateSensitiveVolumeSelection",
                    "DetectorModel11SensitiveVolumeNotFound",
                    FatalException,
                    ("The following model11 sensitive volumes were not found in GDML " +
                     importedAssembly.GetSourcePath() + ": " +
                     JoinVolumeNames(unresolvedVolumeNames) +
                     ". Available logical/physical names: " +
                     JoinVolumeNames(availableVolumeNames))
                        .c_str());
    }
}

G4LogicalVolume* DetectorModel11::CreateImportedLogicalClone(
    G4LogicalVolume* sourceLogicalVolume,
    const G4String& sourcePhysicalName,
    const MD1::GDMLImportedAssembly& importedAssembly,
    const std::set<G4String>& sensitiveVolumeNames,
    G4VSolid* solid,
    const G4String& clonedLogicalName,
    PlacementOwnedResources& resources,
    G4bool ownSolid) {
    if (sourceLogicalVolume == nullptr) {
        G4Exception("DetectorModel11::CreateImportedLogicalClone",
                    "DetectorModel11ImportedGeometryInvalidRoot",
                    FatalException,
                    "model11 cannot clone a null GDML logical volume.");
        return nullptr;
    }
    if (solid == nullptr) {
        G4Exception("DetectorModel11::CreateImportedLogicalClone",
                    "DetectorModel11ImportedGeometryNullSolid",
                    FatalException,
                    ("model11 logical volume '" + sourceLogicalVolume->GetName() +
                     "' cannot be cloned with a null solid.")
                        .c_str());
        return nullptr;
    }

    if (ownSolid) {
        resources.solids.push_back(solid);
    }

    const auto logicalName =
        clonedLogicalName.empty() ? sourceLogicalVolume->GetName() : clonedLogicalName;
    const auto isSensitive =
        IsSensitiveVolumeSelected(sensitiveVolumeNames,
                                  sourceLogicalVolume->GetName(),
                                  sourcePhysicalName);
    auto* clonedLogicalVolume =
        new G4LogicalVolume(solid, sourceLogicalVolume->GetMaterial(), logicalName);
    resources.logicalVolumes.push_back(clonedLogicalVolume);
    if (ownSolid) {
        // Split fragments are few and rebuilt often. Disabling smart voxelisation
        // avoids stale optimisation state across post-initialisation rebuilds.
        clonedLogicalVolume->SetOptimisation(false);
    }

    if (const auto* sourceVisAttributes = sourceLogicalVolume->GetVisAttributes();
        sourceVisAttributes != nullptr) {
        auto* clonedVisAttributes = new G4VisAttributes(*sourceVisAttributes);
        resources.visAttributes.push_back(clonedVisAttributes);
        clonedLogicalVolume->SetVisAttributes(clonedVisAttributes);
    } else if (const auto* sourceAuxiliaries = importedAssembly.GetAuxiliaryInfo(sourceLogicalVolume);
               sourceAuxiliaries != nullptr) {
        G4String colorError;
        G4VisAttributes* clonedVisAttributes = nullptr;
        if (MD1::GDMLColorCodec::TryCreateVisAttributesFromColorAux(
                *sourceAuxiliaries, clonedVisAttributes, &colorError)) {
            resources.visAttributes.push_back(clonedVisAttributes);
            clonedLogicalVolume->SetVisAttributes(clonedVisAttributes);
        } else if (MD1::GDMLColorCodec::FindColorAuxiliary(*sourceAuxiliaries) != nullptr) {
            const G4String message = "Unsupported GDML Color value on logical volume '" +
                                     sourceLogicalVolume->GetName() + "': " + colorError;
            G4Exception("DetectorModel11::CreateImportedLogicalClone",
                        "DetectorModel11UnsupportedGDMLColor",
                        FatalException,
                        message.c_str());
            return nullptr;
        }
    }

    if (const auto* sourceAuxiliaries = importedAssembly.GetAuxiliaryInfo(sourceLogicalVolume);
        sourceAuxiliaries != nullptr) {
        MD1::GeometryAuxiliaryRegistry::GetInstance()->Register(
            clonedLogicalVolume, FlattenAuxiliaryList(*sourceAuxiliaries));
    }

    if (isSensitive) {
        resources.sensitiveLogicalVolumes.push_back(clonedLogicalVolume);
        if (auto* sensitiveDetector = GetCurrentSensitiveDetector(); sensitiveDetector != nullptr) {
            clonedLogicalVolume->SetSensitiveDetector(sensitiveDetector);
        }
    }

    return clonedLogicalVolume;
}

G4bool DetectorModel11::IsSensitiveVolumeSelected(const std::set<G4String>& sensitiveVolumeNames,
                                                  const G4String& logicalVolumeName,
                                                  const G4String& physicalVolumeName) const {
    return sensitiveVolumeNames.find(logicalVolumeName) != sensitiveVolumeNames.end() ||
           sensitiveVolumeNames.find(physicalVolumeName) != sensitiveVolumeNames.end();
}

std::set<G4String> DetectorModel11::CollectReferencedImportedGDMLKeys() const {
    std::set<G4String> referencedKeys;

    for (const auto& [detectorID, config] : fDetectorConfigs) {
        (void)detectorID;
        if (ShouldBuildImportedGeometry(config)) {
            referencedKeys.insert(MD1::GDMLAssemblyCache::BuildCacheKey(
                config.importedGeometryGDMLPath,
                config.importedGeometryRootSelector,
                config.importedGeometryReadOptions));
        }
    }

    if (ShouldBuildImportedGeometry(fDefaultConfig)) {
        for (const auto& [copyNo, motherName] : detMotherVolumeNames) {
            (void)motherName;
            if (fDetectorConfigs.find(copyNo) == fDetectorConfigs.end()) {
                referencedKeys.insert(MD1::GDMLAssemblyCache::BuildCacheKey(
                    fDefaultConfig.importedGeometryGDMLPath,
                    fDefaultConfig.importedGeometryRootSelector,
                    fDefaultConfig.importedGeometryReadOptions));
                break;
            }
        }
    }

    for (const auto& [copyNo, cacheKey] : fPlacementImportedGDMLKeys) {
        (void)copyNo;
        if (!cacheKey.empty()) {
            referencedKeys.insert(cacheKey);
        }
    }

    return referencedKeys;
}

void DetectorModel11::PruneUnusedImportedGDMLAssemblies() {
    fImportedGDMLCache.RetainOnly(CollectReferencedImportedGDMLKeys());
}

G4LogicalVolume* DetectorModel11::CloneImportedSubtree(
    G4LogicalVolume* sourceLogicalVolume,
    const G4String& sourcePhysicalName,
    const MD1::GDMLImportedAssembly& importedAssembly,
    const std::set<G4String>& sensitiveVolumeNames,
    G4int copyNo,
    PlacementOwnedResources& resources) {
    if (sourceLogicalVolume == nullptr) {
        G4Exception("DetectorModel11::CloneImportedSubtree",
                    "DetectorModel11ImportedGeometryInvalidRoot",
                    FatalException,
                    "model11 cannot clone a null GDML logical volume.");
        return nullptr;
    }

    auto* clonedLogicalVolume =
        CreateImportedLogicalClone(sourceLogicalVolume,
                                   sourcePhysicalName,
                                   importedAssembly,
                                   sensitiveVolumeNames,
                                   sourceLogicalVolume->GetSolid(),
                                   sourceLogicalVolume->GetName(),
                                   resources,
                                   false);

    for (G4int daughterIndex = 0; daughterIndex < sourceLogicalVolume->GetNoDaughters(); ++daughterIndex) {
        auto* sourceDaughterPhysical = sourceLogicalVolume->GetDaughter(daughterIndex);
        auto* clonedDaughterLogical =
            CloneImportedSubtree(sourceDaughterPhysical->GetLogicalVolume(),
                                 sourceDaughterPhysical->GetName(),
                                 importedAssembly,
                                 sensitiveVolumeNames,
                                 copyNo,
                                 resources);
        const auto daughterPhysicalName = sourceDaughterPhysical->GetName().empty()
                                              ? sourceDaughterPhysical->GetLogicalVolume()->GetName()
                                              : sourceDaughterPhysical->GetName();
        auto* clonedDaughterPhysical =
            new G4PVPlacement(G4Transform3D(sourceDaughterPhysical->GetObjectRotationValue(),
                                            sourceDaughterPhysical->GetObjectTranslation()),
                              clonedDaughterLogical,
                              daughterPhysicalName,
                              clonedLogicalVolume,
                              false,
                              copyNo,
                              true);
        resources.nestedPhysicalVolumes.push_back(clonedDaughterPhysical);
    }

    return clonedLogicalVolume;
}

G4bool DetectorModel11::HasSupportedSplitRotation(const G4RotationMatrix& rotation) const {
    G4RotationMatrix expectedRotation;
    expectedRotation.rotateX(90. * deg);

    for (G4int row = 0; row < 3; ++row) {
        for (G4int column = 0; column < 3; ++column) {
            if (std::abs(rotation(row, column) - expectedRotation(row, column)) >
                kModel11SplitRotationTolerance) {
                return false;
            }
        }
    }

    return true;
}

void DetectorModel11::ValidateSplitPlacementSupport(const MD1::GDMLImportedAssembly& importedAssembly,
                                                    G4LogicalVolume* motherVolume,
                                                    const G4Transform3D& finalTransform,
                                                    G4int copyNo) const {
    if (motherVolume == nullptr || motherVolume->GetName() != "WaterBox") {
        G4Exception("DetectorModel11::ValidateSplitPlacementSupport",
                    "DetectorModel11SplitMotherNotSupported",
                    FatalException,
                    "split at interface is only supported when model11 is added to WaterBox.");
        return;
    }

    auto* waterBoxSolid = dynamic_cast<G4Box*>(motherVolume->GetSolid());
    if (waterBoxSolid == nullptr) {
        G4Exception("DetectorModel11::ValidateSplitPlacementSupport",
                    "DetectorModel11SplitMotherShapeNotSupported",
                    FatalException,
                    "split at interface for model11 requires WaterBox to be a G4Box.");
        return;
    }

    Bounds3D detectorBounds;
    for (const auto& part : importedAssembly.GetParts()) {
        AccumulateSubtreeBounds(part.logicalVolume,
                                finalTransform * G4Transform3D(part.rotation, part.translation),
                                detectorBounds);
    }

    if (!detectorBounds.valid) {
        G4Exception("DetectorModel11::ValidateSplitPlacementSupport",
                    "DetectorModel11SplitBoundsUnavailable",
                    FatalException,
                    ("Could not determine the imported model11 bounds for detectorID " +
                     std::to_string(copyNo) + ".")
                        .c_str());
        return;
    }

    if (ClassifyBoundsRelativeToWaterBox(detectorBounds, *waterBoxSolid) !=
        SplitRegion::CrossingInterface) {
        return;
    }

    if (!HasSupportedSplitRotation(finalTransform.getRotation())) {
        G4Exception("DetectorModel11::ValidateSplitPlacementSupport",
                    "DetectorModel11SplitRotationNotSupported",
                    FatalException,
                    "split at interface for model11 only supports rotateX 90 deg when the placement crosses the WaterBox interface.");
        return;
    }

    const G4double waterMinX = -waterBoxSolid->GetXHalfLength();
    const G4double waterMaxX = waterBoxSolid->GetXHalfLength();
    const G4double waterMinY = -waterBoxSolid->GetYHalfLength();
    const G4double waterMaxY = waterBoxSolid->GetYHalfLength();
    const G4double waterMinZ = -waterBoxSolid->GetZHalfLength();
    const G4double interfaceZ = waterBoxSolid->GetZHalfLength();

    if (detectorBounds.min.z() < waterMinZ) {
        G4Exception("DetectorModel11::ValidateSplitPlacementSupport",
                    "DetectorModel11SplitBottomFaceNotSupported",
                    FatalException,
                    "split at interface for model11 only supports crossing the top face of WaterBox.");
        return;
    }

    if (detectorBounds.min.x() < waterMinX || detectorBounds.max.x() > waterMaxX ||
        detectorBounds.min.y() < waterMinY || detectorBounds.max.y() > waterMaxY) {
        G4Exception("DetectorModel11::ValidateSplitPlacementSupport",
                    "DetectorModel11SplitLateralFaceNotSupported",
                    FatalException,
                    "split at interface for model11 does not support crossing the lateral faces of WaterBox.");
        return;
    }

    if (detectorBounds.min.z() < interfaceZ && detectorBounds.max.z() > interfaceZ) {
        return;
    }

    G4Exception("DetectorModel11::ValidateSplitPlacementSupport",
                "DetectorModel11SplitPlacementNotSupported",
                FatalException,
                "split at interface for model11 encountered an unsupported placement relative to WaterBox.");
}

void DetectorModel11::AddGeometry(G4LogicalVolume* motherVolume, G4int copyNo) {
    G4Transform3D transform;
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorModel11::AddGeometry(G4LogicalVolume* motherVolume,
                                  const G4ThreeVector& position,
                                  G4RotationMatrix* rotation,
                                  G4int copyNo) {
    G4RotationMatrix identityRotation;
    const G4RotationMatrix& appliedRotation = (rotation != nullptr) ? *rotation : identityRotation;
    G4Transform3D transform(appliedRotation, position);
    AddGeometry(motherVolume, &transform, copyNo);
}

void DetectorModel11::AddGeometry(G4LogicalVolume* motherVolume,
                                  G4Transform3D* transformation,
                                  G4int copyNo) {
    if (!fAreVolumensDefined) {
        DefineVolumes();
    }

    const auto& config = GetDetectorConfig(copyNo);
    fPlacementImportedGDMLKeys.erase(copyNo);

    if (config.importedGeometryGDMLPath.empty()) {
        G4Exception("DetectorModel11::AddGeometry",
                    "DetectorModel11ImportedGeometryGDMLMissing",
                    FatalException,
                    "model11 requires a GDML path because the full geometry is defined by the imported model.");
        return;
    }

    const auto assembly = LoadImportedGDMLAssembly(config);
    if (assembly == nullptr || assembly->GetParts().empty()) {
        G4Exception("DetectorModel11::AddGeometry",
                    "DetectorModel11ImportedGeometryEmpty",
                    FatalException,
                    "Imported geometry GDML did not provide any reusable parts for model11.");
        return;
    }

    const auto effectiveSensitiveVolumeNames = ResolveSensitiveVolumeNames(config);
    ValidateSensitiveVolumeSelection(*assembly, effectiveSensitiveVolumeNames);

    // Imported model11 geometry is flattened around the GDML assembly origin.
    // Keep that local origin as the runtime pivot by translating the anchor first
    // and then rotating the flattened parts around it.
    const G4ThreeVector centerRelativeToMother = transformation->getTranslation() + det_origin;
    const G4Transform3D finalTransform =
        G4Translate3D(centerRelativeToMother) * G4Rotate3D(transformation->getRotation());
    const G4RotationMatrix rotation = finalTransform.getRotation().inverse();
    StoreRotation(copyNo, rotation);

    PlacementOwnedResources resources;
    G4LogicalVolume* worldMother = nullptr;
    G4Box* waterBoxSolid = nullptr;
    G4ThreeVector waterWorldTranslation;
    const G4bool splitAtWaterInterface = config.splitAtInterface;

    if (splitAtWaterInterface) {
        ValidateSplitPlacementSupport(*assembly, motherVolume, finalTransform, copyNo);
        waterBoxSolid = dynamic_cast<G4Box*>(motherVolume->GetSolid());

        worldMother = G4LogicalVolumeStore::GetInstance()->GetVolume("world_log", false);
        if (worldMother == nullptr) {
            G4Exception("DetectorModel11::AddGeometry",
                        "DetectorModel11SplitWorldLogicalVolumeNotFound",
                        FatalException,
                        "Could not resolve world_log while splitting model11 at the WaterBox interface.");
            return;
        }

        auto* waterPhysicalVolume = GetWaterPhysicalVolume();
        if (waterPhysicalVolume == nullptr) {
            return;
        }
        waterWorldTranslation = waterPhysicalVolume->GetTranslation();
    }

    G4VPhysicalVolume* primaryPlacement = nullptr;
    const auto registerPlacement = [&](G4VPhysicalVolume* placement) {
        if (placement == nullptr) {
            G4Exception("DetectorModel11::AddGeometry",
                        "DetectorModel11PlacementCreationFailed",
                        FatalException,
                        "Model11 failed to create any physical placement for the requested geometry.");
            return;
        }

        if (primaryPlacement == nullptr) {
            primaryPlacement = placement;
            SetPrimaryFrameVolume(copyNo, placement);
        } else {
            AddAuxiliaryFrameVolume(copyNo, placement);
        }
    };

    const G4double interfaceZ =
        (waterBoxSolid != nullptr) ? waterBoxSolid->GetZHalfLength() : 0.;
    std::vector<ImportedPartRuntimePlacement> partPlacements;
    partPlacements.reserve(assembly->GetParts().size());
    G4bool hasCrossingPart = false;

    for (const auto& part : assembly->GetParts()) {
        ImportedPartRuntimePlacement placement;
        placement.part = &part;
        placement.sourcePhysicalName =
            (part.physicalVolume != nullptr) ? part.physicalVolume->GetName() : part.name;
        placement.rootPhysicalName = ResolveRuntimePhysicalName(part, *assembly);
        placement.localTransform = G4Transform3D(part.rotation, part.translation);
        placement.waterTransform = finalTransform * placement.localTransform;
        placement.worldTransform = G4Translate3D(waterWorldTranslation) * placement.waterTransform;

        if (splitAtWaterInterface) {
            Bounds3D partBounds;
            AccumulateSubtreeBounds(part.logicalVolume, placement.waterTransform, partBounds);
            placement.splitRegion = ClassifyBoundsRelativeToWaterBox(partBounds, *waterBoxSolid);
            hasCrossingPart =
                hasCrossingPart || placement.splitRegion == SplitRegion::CrossingInterface;
        }

        partPlacements.push_back(placement);
    }

    const auto placeDirectPart = [&](const ImportedPartRuntimePlacement& placement,
                                     G4LogicalVolume* targetMother,
                                     const G4Transform3D& targetTransform) {
        auto* clonedRootLogical = CloneImportedSubtree(placement.part->logicalVolume,
                                                       placement.sourcePhysicalName,
                                                       *assembly,
                                                       effectiveSensitiveVolumeNames,
                                                       copyNo,
                                                       resources);
        auto* physicalPlacement = new G4PVPlacement(targetTransform,
                                                    clonedRootLogical,
                                                    placement.rootPhysicalName,
                                                    targetMother,
                                                    false,
                                                    copyNo,
                                                    true);
        registerPlacement(physicalPlacement);
    };

    if (!splitAtWaterInterface || !hasCrossingPart) {
        for (const auto& placement : partPlacements) {
            if (!splitAtWaterInterface || placement.splitRegion == SplitRegion::InsideWater) {
                placeDirectPart(placement, motherVolume, placement.waterTransform);
                continue;
            }

            placeDirectPart(placement, worldMother, placement.worldTransform);
        }
    } else {
        for (const auto& placement : partPlacements) {
            const auto& part = *placement.part;
            if (placement.splitRegion == SplitRegion::InsideWater) {
                placeDirectPart(placement, motherVolume, placement.waterTransform);
                continue;
            }

            if (placement.splitRegion == SplitRegion::OutsideWater) {
                placeDirectPart(placement, worldMother, placement.worldTransform);
                continue;
            }

            if (part.logicalVolume->GetNoDaughters() > 0) {
                G4Exception("DetectorModel11::AddGeometry",
                            "DetectorModel11SplitNestedSubtreeUnsupported",
                            FatalException,
                            ("split at interface for model11 requires a flat imported assembly "
                             "when a part crosses the WaterBox interface; logical volume '" +
                             part.logicalVolume->GetName() +
                             "' still contains nested daughters.")
                                .c_str());
                return;
            }

            auto* sourceSolid = part.logicalVolume->GetSolid();
            if (sourceSolid == nullptr) {
                G4Exception("DetectorModel11::AddGeometry",
                            "DetectorModel11SplitNullSolid",
                            FatalException,
                            ("split at interface for model11 cannot clip logical volume '" +
                             part.logicalVolume->GetName() +
                             "' because it does not own a solid.")
                                .c_str());
                return;
            }

            const auto splitPlane = BuildPartLocalSplitPlane(placement.waterTransform, interfaceZ);

            auto waterSolid = BuildSplitSolidOrThrow(*sourceSolid,
                                                     part.logicalVolume->GetName(),
                                                     part.logicalVolume->GetName() + "_water_solid",
                                                     splitPlane,
                                                     SplitHalf::Water);
            auto* waterLogical = CreateImportedLogicalClone(part.logicalVolume,
                                                            placement.sourcePhysicalName,
                                                            *assembly,
                                                            effectiveSensitiveVolumeNames,
                                                            waterSolid.release(),
                                                            part.logicalVolume->GetName() + "_water",
                                                            resources,
                                                            true);
            auto* waterPlacement = new G4PVPlacement(placement.waterTransform,
                                                     waterLogical,
                                                     placement.rootPhysicalName + "_water",
                                                     motherVolume,
                                                     false,
                                                     copyNo,
                                                     false);
            registerPlacement(waterPlacement);

            auto airSolid = BuildSplitSolidOrThrow(*sourceSolid,
                                                   part.logicalVolume->GetName(),
                                                   part.logicalVolume->GetName() + "_air_solid",
                                                   splitPlane,
                                                   SplitHalf::Air);
            auto* airLogical = CreateImportedLogicalClone(part.logicalVolume,
                                                          placement.sourcePhysicalName,
                                                          *assembly,
                                                          effectiveSensitiveVolumeNames,
                                                          airSolid.release(),
                                                          part.logicalVolume->GetName() + "_air",
                                                          resources,
                                                          true);
            auto* airPlacement = new G4PVPlacement(placement.worldTransform,
                                                   airLogical,
                                                   placement.rootPhysicalName + "_air",
                                                   worldMother,
                                                   false,
                                                   copyNo,
                                                   false);
            registerPlacement(airPlacement);
        }
    }

    if (primaryPlacement == nullptr) {
        G4Exception("DetectorModel11::AddGeometry",
                    "DetectorModel11PlacementCreationFailed",
                    FatalException,
                    "Model11 failed to create any physical placement for the requested geometry.");
        return;
    }

    fPlacementImportedGDMLKeys[copyNo] =
        MD1::GDMLAssemblyCache::BuildCacheKey(config.importedGeometryGDMLPath,
                                              config.importedGeometryRootSelector,
                                              config.importedGeometryReadOptions);
    fPlacementResources[copyNo] = std::move(resources);
    fAreVolumensAssembled = true;
    detPosition[copyNo] = centerRelativeToMother;
}

G4VPhysicalVolume* DetectorModel11::GetWaterPhysicalVolume() const {
    auto* waterPhysicalVolume = G4PhysicalVolumeStore::GetInstance()->GetVolume("WaterBox", false);
    if (waterPhysicalVolume == nullptr) {
        G4Exception("DetectorModel11::GetWaterPhysicalVolume",
                    "DetectorModel11WaterBoxPhysicalVolumeNotFound",
                    FatalException,
                    "Could not resolve the WaterBox physical volume while splitting model11.");
        return nullptr;
    }

    return waterPhysicalVolume;
}

void DetectorModel11::AttachSensitiveDetector(G4VSensitiveDetector* sensitiveDetector) {
    if (sensitiveDetector == nullptr) {
        return;
    }

    fSensitiveDetector = sensitiveDetector;
    for (auto& [copyNo, resources] : fPlacementResources) {
        (void)copyNo;
        for (auto* logicalVolume : resources.sensitiveLogicalVolumes) {
            if (logicalVolume != nullptr) {
                logicalVolume->SetSensitiveDetector(sensitiveDetector);
            }
        }
    }
}

G4bool DetectorModel11::RequiresPlacementRebuild(const G4int& copyNo) const {
    return ShouldBuildImportedGeometry(GetDetectorConfig(copyNo));
}

void DetectorModel11::ReleaseDetachedPlacementFrames(
    const G4int& /*copyNo*/,
    std::vector<G4VPhysicalVolume*>& detachedFrames) {
    for (auto* frameVolume : detachedFrames) {
        if (frameVolume != nullptr) {
            fRetiredPlacementFrames.push_back(frameVolume);
        }
    }
    detachedFrames.clear();
}

void DetectorModel11::OnAfterPlacementRemoval(const G4int& copyNo) {
    ReleasePlacementResources(copyNo);
    fPlacementImportedGDMLKeys.erase(copyNo);
    PruneUnusedImportedGDMLAssemblies();
}

void DetectorModel11::ReleasePlacementResources(const G4int& copyNo) {
    const auto resourcesIt = fPlacementResources.find(copyNo);
    if (resourcesIt == fPlacementResources.end()) {
        return;
    }

    auto& resources = resourcesIt->second;
    for (auto* nestedPlacement : resources.nestedPhysicalVolumes) {
        if (nestedPlacement != nullptr) {
            if (auto* motherLogical = nestedPlacement->GetMotherLogical(); motherLogical != nullptr) {
                motherLogical->RemoveDaughter(nestedPlacement);
            }
        }
    }
    resources.sensitiveLogicalVolumes.clear();
    // In MT Geant4 can keep worker-side references to the logical volumes and
    // solids used in the previous run until the next geometry propagation is
    // completed. Retain the removed logical/solid/physical resources for the
    // rest of the process so rebuilds do not free geometry objects that workers
    // may still traverse.
    fRetiredPlacementResources.push_back(std::move(resources));
    fPlacementResources.erase(resourcesIt);
}
