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

// Geant4 Headers
#include <cerrno>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string>

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "Randomize.hh"
#include "G4VModularPhysicsList.hh"
#include "G4ScoringManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4GenericBiasingPhysics.hh"
#include "FTFP_BERT.hh"

#ifdef G4UI_USE_QT
#include <QApplication>
#include <QFont>
#include <QLoggingCategory>
#include <QStringList>
#include <QThread>
#endif

// MultiDetector Headers
#include "MD1DetectorConstruction.hh"
#include "MD1ActionInitialization.hh"
#include "MD1Control.hh"
#include "MD1PhspSourceConfig.hh"
#include "geometry/base/DetectorRegistry.hh"
#include "geometry/gdml/MD1GeometryExport.hh"

using namespace MD1;

namespace {

struct ProgramOptions {
  G4String macro = "mac/init.mac";
  G4String visMacro = "mac/vis_vtk.mac";
  G4String biasing = "off";
  G4String visualization = "off";
  G4int numberOfEvents = 0;
  bool interactive = false;
};

bool IsOnOffValue(const G4String& value) {
  return value == "on" || value == "off";
}

bool ParseNonNegativeInt(const G4String& value, G4int& parsedValue) {
  errno = 0;
  char* end = nullptr;
  const long converted = std::strtol(value.c_str(), &end, 10);
  if (errno != 0 || end == value.c_str() || *end != '\0' || converted < 0) {
    return false;
  }

  parsedValue = static_cast<G4int>(converted);
  return true;
}

bool ParseCommandLine(int argc, char** argv, ProgramOptions& options) {
  if (argc == 1) {
    options.interactive = true;
    options.visualization = "on";
    return true;
  }

  if (((argc - 1) % 2) != 0) {
    G4cerr << "Error: command-line options must be provided as -flag value pairs." << G4endl;
    return false;
  }

  for (G4int i = 1; i < argc; i += 2) {
    const G4String flag = argv[i];
    const G4String value = argv[i + 1];

    if (flag == "-m") {
      options.macro = value;
    } else if (flag == "-v") {
      if (!IsOnOffValue(value)) {
        G4cerr << "Error: -v expects 'on' or 'off'." << G4endl;
        return false;
      }
      options.visualization = value;
    } else if (flag == "-vm") {
      options.visMacro = value;
      options.visualization = "on";
    } else if (flag == "-b") {
      if (!IsOnOffValue(value)) {
        G4cerr << "Error: -b expects 'on' or 'off'." << G4endl;
        return false;
      }
      options.biasing = value;
    } else if (flag == "-n") {
      G4int parsedEvents = 0;
      if (!ParseNonNegativeInt(value, parsedEvents)) {
        G4cerr << "Error: -n expects a non-negative integer." << G4endl;
        return false;
      }
      options.numberOfEvents = parsedEvents;
    } else {
      G4cerr << "Error: unknown option '" << flag << "'." << G4endl;
      return false;
    }
  }

  if (options.visualization == "on") {
    options.interactive = true;
  }

  return true;
}

bool ApplyCommandOrReportFailure(G4UImanager* uiManager,
                                 const G4String& command,
                                 const G4String& description) {
  const G4int status = uiManager->ApplyCommand(command);
  if (status == 0) {
    return true;
  }

  G4cerr << "Error: failed to execute " << description
         << " (status " << status << "): " << command << G4endl;
  return false;
}

#ifdef G4UI_USE_QT
void ConfigureQtFontSubstitutions() {
#ifdef __APPLE__
  const QStringList monospaceFallbacks{"Menlo", "Monaco", "SF Mono", "Monospace"};
  const QStringList sansFallbacks{"Helvetica Neue", "Helvetica", "Arial"};
#else
  const QStringList monospaceFallbacks{"DejaVu Sans Mono", "Liberation Mono", "Monospace"};
  const QStringList sansFallbacks{"DejaVu Sans", "Liberation Sans", "Arial"};
#endif

  QFont::insertSubstitutions("Courier", monospaceFallbacks);
  QFont::insertSubstitutions("courier", monospaceFallbacks);
  QFont::insertSubstitutions("Sans Serif", sansFallbacks);
  QFont::insertSubstitutions("sans-serif", sansFallbacks);
}

void ConfigureQtLoggingRules() {
  QLoggingCategory::setFilterRules(QStringLiteral("qt.qpa.fonts.warning=false"));

  const char* existingRules = std::getenv("QT_LOGGING_RULES");
  if (existingRules == nullptr || *existingRules == '\0') {
    setenv("QT_LOGGING_RULES", "qt.qpa.fonts.warning=false", 0);
    return;
  }

  const std::string currentRules(existingRules);
  if (currentRules.find("qt.qpa.fonts") != std::string::npos) {
    return;
  }

  const std::string combinedRules = currentRules + ";qt.qpa.fonts.warning=false";
  setenv("QT_LOGGING_RULES", combinedRules.c_str(), 1);
}

#endif

}  // namespace

void PrintUsage() {
	G4cerr << " Usage: " << G4endl;
	G4cerr << " ./MultiDetector1"
			<< " [-m macro]"
			<< " [-v on|off]"
			<< " [-vm vis_macro]"
			<< " [-b on|off]"
			<< " [-n numberOfEvents]"
			<< G4endl;
}

int main(int argc,char** argv)
{
  ProgramOptions options;
  if (!ParseCommandLine(argc, argv, options)) {
    PrintUsage();
    return 1;
  }

  std::unique_ptr<G4UIExecutive> ui;
  if (options.interactive) {
#ifdef G4UI_USE_QT
    ConfigureQtLoggingRules();
    ConfigureQtFontSubstitutions();
#endif
    ui = std::make_unique<G4UIExecutive>(argc, argv);
  }

  // Construct Control and ControlMessenger
  MD1Control::GetInstance();
  MD1PhspSourceConfig::GetInstance();
  DetectorRegistry::GetInstance();
  MD1GeometryExport::GetInstance();

  // Optionally: choose a different Random engine...
  G4Random::setTheEngine(new CLHEP::MTwistEngine);
  G4int seed = static_cast<G4int>(std::time(nullptr));
  G4Random::setTheSeed( seed );

  //use G4SteppingVerboseWithUnits
  G4int precision = 4;
  G4SteppingVerbose::UseBestUnit(precision);

  // Construct the default run manager
  //
  auto runManager =
    std::unique_ptr<G4RunManager>(G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default));

  // Activate UI-command base scorer
  G4ScoringManager * scoringManager = G4ScoringManager::GetScoringManager();

  scoringManager->SetVerboseLevel(1);

  // Set mandatory initialization classes
  //
  // Detector construction
  runManager->SetUserInitialization(new MD1DetectorConstruction());

  // Physics list
  G4VModularPhysicsList* physicsList = new FTFP_BERT;
	physicsList->SetDefaultCutValue(1*mm);
	//physicsList->SetCutValue(4*um,"e-");
	//physicsList->SetCutValue(4*um,"e+");
	//physicsList->SetCutValue(4*um,"proton");
	//physicsList->SetCutValue(0.01*mm,"gamma");
	physicsList->DumpCutValuesTable();
  
	if ( options.biasing == "on" )
	{
    auto* biasingPhysics = new G4GenericBiasingPhysics();
		//biasingPhysics->NonPhysicsBiasAllCharged();
		biasingPhysics->Bias("gamma");
		//biasingPhysics->NonPhysicsBias("gamma");
		physicsList->RegisterPhysics(biasingPhysics);
		G4cout << "      ********************************************************* " << G4endl;
		G4cout << "      ********** processes are wrapped for biasing ************ " << G4endl;
		G4cout << "      ********************************************************* " << G4endl;
	}
	else
	{
		G4cout << "      ************************************************* " << G4endl;
		G4cout << "      ********** processes are not wrapped ************ " << G4endl;
		G4cout << "      ************************************************* " << G4endl;
	}

  runManager->SetUserInitialization(physicsList);

  // User action initialization
  runManager->SetUserInitialization(new MD1ActionInitialization());

  // Initialize visualization
  //
  std::unique_ptr<G4VisManager> visManager;
  if (options.interactive) {
    visManager = std::make_unique<G4VisExecutive>();
    visManager->Initialize();
  }

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  auto shutdown = [&]() {
    ui.reset();
    visManager.reset();
    runManager.reset();
    MD1GeometryExport::Kill();
    DetectorRegistry::Kill();
    MD1PhspSourceConfig::Kill();
    MD1Control::Kill();
  };

  if (options.interactive && ui && ui->IsGUI()) {
    if (!ApplyCommandOrReportFailure(UImanager,
                                     "/control/execute mac/qt_ui.mac",
                                     "Qt UI macro")) {
      shutdown();
      return 1;
    }
  }

	// Process macro or start UI session
	//
	if (!options.macro.empty())
	{
		G4String command = "/control/execute ";
		if (!ApplyCommandOrReportFailure(UImanager, command + options.macro, "input macro")) {
      shutdown();
      return 1;
    }
	}

	if (options.interactive) {
		// interactive mode
		G4String command = "/control/execute ";
		if (!ApplyCommandOrReportFailure(UImanager, command + options.visMacro, "visualization macro")) {
      shutdown();
      return 1;
    }
		ui->SessionStart();
	}else{
		runManager->BeamOn(options.numberOfEvents);
	}

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted
  // in the main() program !

  shutdown();
}
