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
#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "Randomize.hh"
#include "QBBC.hh"
#include "G4PhysListFactory.hh"
#include "G4VModularPhysicsList.hh"
#include "G4ProductionCutsTable.hh"
#include "G4RegionStore.hh"
#include "G4Region.hh"
#include "G4MaterialCutsCouple.hh"
#include "G4UserLimits.hh"
#include "G4ScoringManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4GenericBiasingPhysics.hh"
#include "FTFP_BERT.hh"

// MultiDetector Headers
#include "MD1DetectorConstruction.hh"
#include "MD1ActionInitialization.hh"
#include "MD1Control.hh"

using namespace MD1;

void PrintUsage() {
	G4cerr << " Usage: " << G4endl;
	G4cerr << " ./Multidetector1 [-m macro ] "
			<< " [-v visualization {'on','off'}]"
			<< " [-vm vis_macro ]"
			<< " [-b biasing {'on','off'}]"
			<< " [-n numberOfEvent ]"
			<< "\n or\n ./Multidetector1 [macro.mac]"
			<< G4endl;
}

int main(int argc,char** argv)
{
  // Detect interactive mode (if no arguments) and define UI session
  //
  G4UIExecutive* ui = 0;

	// Evaluate arguments
	//
	if ( argc > 10 ) {
		PrintUsage();
		return 1;
	}

	G4String macro("");
	G4String vis_macro("");
	G4String onOffBiasing("");
	G4String onOffVisualization("");
	G4int numberOfEvent(0);

	if (argc == 1) {
		ui = new G4UIExecutive(argc, argv);
		onOffVisualization="on";
	}
	for ( G4int i=1; i<argc; i=i+2 )
	{
		if ( G4String(argv[i]) == "-m" ) macro = argv[i+1];
		else if (G4String(argv[i]) == "-v" ) {
			onOffVisualization=argv[i+1];
			if(onOffVisualization=="on"){
				ui = new G4UIExecutive(argc, argv);
			}
		}
		else if ( G4String(argv[i]) == "-vm" ) {
			if(!ui) ui = new G4UIExecutive(argc, argv);
			if(G4String(argv[i]) == "-vm") vis_macro = argv[i+1];
			onOffVisualization="on";
		}
		else if ( G4String(argv[i]) == "-b" ) onOffBiasing	= argv[i+1];
		else if ( G4String(argv[i]) == "-n" ) numberOfEvent = G4UIcommand::ConvertToInt(argv[i+1]);
		else{
			PrintUsage();
			return 1;
		}
	}

	if(macro == "") macro = "mac/init.mac";
	if(vis_macro == "") vis_macro = "mac/vis.mac";
	if(onOffBiasing == "") onOffBiasing = "off";
	if(onOffVisualization == "") onOffVisualization = "off";

  // Construct Control and ControlMessenger
  MD1Control::GetInstance();

  // Optionally: choose a different Random engine...
  G4Random::setTheEngine(new CLHEP::MTwistEngine);
  G4int seed = time( NULL );
  G4Random::setTheSeed( seed );

  //use G4SteppingVerboseWithUnits
  G4int precision = 4;
  G4SteppingVerbose::UseBestUnit(precision);

  // Construct the default run manager
  //
  auto* runManager =
    G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

  // Activate UI-command base scorer
  G4ScoringManager * scoringManager = G4ScoringManager::GetScoringManager();

  scoringManager->SetVerboseLevel(1);

  // Set mandatory initialization classes
  //
  // Detector construction
  runManager->SetUserInitialization(new MD1DetectorConstruction());

  // Physics list
  G4PhysListFactory factory;
  G4VModularPhysicsList* physicsList = new FTFP_BERT;//factory.GetReferencePhysList("QGSP_BIC_EMY");
	physicsList->SetDefaultCutValue(1*mm);
	//physicsList->SetCutValue(4*um,"e-");
	//physicsList->SetCutValue(4*um,"e+");
	//physicsList->SetCutValue(4*um,"proton");
	//physicsList->SetCutValue(0.01*mm,"gamma");
	physicsList->DumpCutValuesTable();
  
  G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics();
	if ( onOffBiasing == "on" )
	{
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
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

	// Process macro or start UI session
	//
	if ( macro != "" )
	{
		G4String command = "/control/execute ";
		UImanager->ApplyCommand(command+macro);
	}

	if (onOffVisualization=="on"){
		// interactive mode
		G4String command = "/control/execute ";
		UImanager->ApplyCommand(command+vis_macro);
		ui->SessionStart();
		delete ui;
	}else{
		if ( numberOfEvent >= 0 ) runManager->BeamOn(numberOfEvent);
	}

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted
  // in the main() program !

  delete visManager;
  delete runManager;
}
