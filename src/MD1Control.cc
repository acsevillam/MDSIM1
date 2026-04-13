/*
 *
 * Geant4 MultiDetector Simulation v1
 * Copyright (c) 2024 Andrés Camilo Sevilla
 * acsevillam@eafit.edu.co  - acsevillam@gmail.com
 * All Rights Reserved.
 *
 * Use and copying of these libraries and preparation of derivative works
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * San Sebastian, Spain.
 *
 */

#include "MD1Control.hh"
#include "MD1ControlMessenger.hh"

#include <algorithm>
#include <string>

#include "G4AxesModel.hh"
#include "G4Exception.hh"
#include "G4Scene.hh"
#include "G4SystemOfUnits.hh"
#include "G4VViewer.hh"
#include "G4VVisManager.hh"
#include "G4ViewParameters.hh"
#include "G4VisManager.hh"

namespace MD1 {

namespace {

constexpr const char* kFocusAxesModelTag = "MD1FocusAxes";
constexpr const char* kFocusAxesModelDescription = "MDSIM1 focus axes";

struct CurrentVisualizationContext {
    G4VisManager* visManager = nullptr;
    G4Scene* scene = nullptr;
    G4VViewer* viewer = nullptr;
};

CurrentVisualizationContext ResolveCurrentVisualizationContext(const char* caller) {
    CurrentVisualizationContext context;

    auto* concreteVisManager = G4VVisManager::GetConcreteInstance();
    context.visManager = dynamic_cast<G4VisManager*>(concreteVisManager);
    if (context.visManager == nullptr) {
        G4ExceptionDescription description;
        description << "No active Geant4 viewer is available.";
        G4Exception(caller, "MD1ViewUnavailable", JustWarning, description);
        return context;
    }

    context.scene = context.visManager->GetCurrentScene();
    context.viewer = context.visManager->GetCurrentViewer();
    if (context.scene == nullptr || context.viewer == nullptr) {
        G4ExceptionDescription description;
        description << "The current scene or viewer is not available.";
        G4Exception(caller, "MD1SceneUnavailable", JustWarning, description);
        context.scene = nullptr;
        context.viewer = nullptr;
    }

    return context;
}

G4bool IsFocusAxesModel(const G4Scene::Model& model) {
    return model.fpModel != nullptr && model.fpModel->GetGlobalTag() == kFocusAxesModelTag;
}

G4Point3D GetAbsoluteTargetPoint(const G4Scene& scene, const G4ViewParameters& viewParameters) {
    const auto& standardTarget = scene.GetStandardTargetPoint();
    const auto& currentTargetOffset = viewParameters.GetCurrentTargetPoint();
    return G4Point3D(standardTarget.x() + currentTargetOffset.x(),
                     standardTarget.y() + currentTargetOffset.y(),
                     standardTarget.z() + currentTargetOffset.z());
}

G4Point3D GetRelativeTargetPoint(const G4Scene& scene, const G4Point3D& absoluteTargetPoint) {
    const auto& standardTarget = scene.GetStandardTargetPoint();
    return G4Point3D(absoluteTargetPoint.x() - standardTarget.x(),
                     absoluteTargetPoint.y() - standardTarget.y(),
                     absoluteTargetPoint.z() - standardTarget.z());
}

G4double ComputeFocusAxesLength(const G4Scene& scene) {
    (void)scene;
    return 5. * cm;
}

G4bool SceneHasFocusAxes(const G4Scene& scene) {
    const auto& models = scene.GetRunDurationModelList();
    return std::any_of(models.begin(), models.end(), IsFocusAxesModel);
}

void RemoveFocusAxesFromScene(G4Scene& scene) {
    auto& models = scene.SetRunDurationModelList();
    std::vector<G4Scene::Model> keptModels;
    keptModels.reserve(models.size());
    std::vector<G4VModel*> removedModels;

    for (const auto& model : models) {
        if (IsFocusAxesModel(model)) {
            removedModels.push_back(model.fpModel);
            continue;
        }

        keptModels.push_back(model);
    }

    models.swap(keptModels);

    for (auto* model : removedModels) {
        delete model;
    }
}

void AddFocusAxesToScene(G4Scene& scene, const G4Point3D& absoluteTargetPoint) {
    auto* axesModel = new G4AxesModel(absoluteTargetPoint.x(),
                                      absoluteTargetPoint.y(),
                                      absoluteTargetPoint.z(),
                                      ComputeFocusAxesLength(scene),
                                      1.,
                                      "auto",
                                      "",
                                      true,
                                      10.);
    axesModel->SetGlobalTag(kFocusAxesModelTag);
    axesModel->SetGlobalDescription(kFocusAxesModelDescription);
    scene.AddRunDurationModel(axesModel);
}

void RedrawScene(CurrentVisualizationContext& context, G4bool sceneChanged) {
    if (context.viewer == nullptr || context.visManager == nullptr) {
        return;
    }

    if (sceneChanged) {
        context.viewer->NeedKernelVisit();
        context.visManager->NotifyHandlers();
        context.viewer->UpdateGUISceneTree();
    }

    context.viewer->RefreshView();
    context.viewer->UpdateGUIControlWidgets();
}

} // namespace

// Define Static Variables
MD1Control* MD1Control::instance = nullptr;

MD1Control::MD1Control() {
	fMD1ControlMessenger = new MD1ControlMessenger(this);
}

MD1Control::~MD1Control()
{
	delete fMD1ControlMessenger;
	fMD1ControlMessenger = nullptr;
}

MD1Control* MD1Control::GetInstance() {

	if (instance == nullptr) instance =  new MD1Control();
	return instance ;

}

void MD1Control::Kill() {

	if(instance!=nullptr){
		delete instance ;
		instance = nullptr ;
	}
}

void MD1Control::SetPrimaryGeneratorType(G4int aPrimaryGeneratorType) {
	if (aPrimaryGeneratorType != 1 && aPrimaryGeneratorType != 2) {
		G4Exception("MD1Control::SetPrimaryGeneratorType",
		            "InvalidPrimaryGeneratorType",
		            FatalException,
		            ("Unsupported primary generator type " +
		             std::to_string(aPrimaryGeneratorType) +
		             ". Use 1 for IAEA phase-space or 2 for GPS.")
		                .c_str());
	}

	fPrimaryGeneratorType = aPrimaryGeneratorType;
}

void MD1Control::CenterViewOnZAxis() {
    auto context = ResolveCurrentVisualizationContext("MD1Control::CenterViewOnZAxis");
    if (context.scene == nullptr || context.viewer == nullptr) {
        return;
    }

    auto viewParameters = context.viewer->GetViewParameters();
    const auto currentAbsoluteTarget = GetAbsoluteTargetPoint(*context.scene, viewParameters);
    const G4Point3D centeredAbsoluteTarget(0., 0., currentAbsoluteTarget.z());
    viewParameters.SetCurrentTargetPoint(GetRelativeTargetPoint(*context.scene, centeredAbsoluteTarget));
    context.viewer->SetViewParameters(viewParameters);

    const G4bool focusAxesWereVisible = SceneHasFocusAxes(*context.scene);
    if (focusAxesWereVisible) {
        RemoveFocusAxesFromScene(*context.scene);
        AddFocusAxesToScene(*context.scene, centeredAbsoluteTarget);
    }

    RedrawScene(context, focusAxesWereVisible);
}

void MD1Control::ToggleFocusAxes() {
    auto context = ResolveCurrentVisualizationContext("MD1Control::ToggleFocusAxes");
    if (context.scene == nullptr || context.viewer == nullptr) {
        return;
    }

    const G4bool focusAxesVisible = SceneHasFocusAxes(*context.scene);
    if (focusAxesVisible) {
        RemoveFocusAxesFromScene(*context.scene);
    } else {
        const auto absoluteTargetPoint =
            GetAbsoluteTargetPoint(*context.scene, context.viewer->GetViewParameters());
        AddFocusAxesToScene(*context.scene, absoluteTargetPoint);
    }

    RedrawScene(context, true);
}

}
