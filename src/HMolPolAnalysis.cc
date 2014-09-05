/********************************************
 * \author <b>Programmer:</b> Valerie Gray
 * \author <b>Assisted By:</b> Wouter Deconinck
 *
 * \brief <b>Purpose:</b> This file is in charge
 *      of getting everything set up so that the
 *      simulation will have results in a ROOT file.
 *
 * \date <b>Date:</b> 07-11-2013
 * \date <b>Modified:</b> 09-03-2014
 *
 * \note <b>Entry Conditions:</b>
 *
 * \todo GET User info to print to tree
 *
 ********************************************/

// geant4 includes
#include <G4Run.hh>

// HMolPol includes
#include "HMolPolAnalysis.hh"
#include "HMolPolEventUnits.hh"
#include "HMolPolEventPrimary.hh"
#include "HMolPolEventGenericDetector.hh"
#include "HMolPolRunInformation.hh"

/// \todo make this so the file names can change with things like generator etc
/// ie HMolPol_eleastic HMolPol_Moller, etc

/********************************************
 * Programmer: Valerie Gray
 * Function: HMolPolAnalysis():
 *      fRootFileStem("HMolPol"),fRootFileName("")
 *
 * Purpose:  To initialize the default Root file
 *      name and everything in the
 *      Tree to null.
 *
 *      Can do this initialization following the : after the
 *      function name, only since this is the constructor.
 *
 * Global:
 * Entry Conditions: fRootFileStem("HMolPol")
 *                   f(RootFileName("")
 * Return:
 * Called By:
 * Date: 07-11-2013
 * Modified: 09-03-2014
 ********************************************/
HMolPolAnalysis::HMolPolAnalysis()
: fRootTree(0),fRootFile(0),
  fRootFileStem("HMolPol"),fRootFileName(""),
  fRunNumber(0),fBeamEnergy(0),
  fUnits(0),fUnitsBranch(0),
  fPrimary(0),fPrimaryBranch(0),
  pUserRunInformation(0)
{
  // starting the anaylsis
  G4cout << "###### Calling HMolPolAnalysis::HMolPolAnalysis()" << G4endl;

}

/********************************************
 * Programmer: Valerie Gray
 * Function: ~HMolPolAnalysis()
 *
 * Purpose:  To deconsturuct the default Root file
 *      name and everything in the Tree.
 *
 * Global:
 * Entry Conditions:
 * Return:
 * Called By:
 * Date: 07-11-2013
 * Modified: 09-03-2014
 ********************************************/
HMolPolAnalysis::~HMolPolAnalysis()
{
  // Delete ROOT objects
  if (fPrimary)        delete fPrimary;
  if (fRootTree)     delete fRootTree;
  if (fPrimaryBranch)   delete fPrimaryBranch;
  if (fRootFile)     delete fRootFile;

}

/********************************************
 * Programmer: Valerie Gray
 * Function: BeginOfRun
 *
 * Purpose:  Set up what should be done at the
 *      beginning of a run.
 *      - Set up the output ROOT file name
 *      - Create ROOT file with that name
 *
 * \define run is a set of events for which a
 *      whole root file is produced.
 *      i.e. if one has /run/beamOn/ 1000 then
 *      a run is 1000 fired electrons
 *
 * Global:
 * Entry Conditions: const G4Run* aRun
 * Return:
 * Called By:
 * Date: 07-11-2013
 * Modified:
 ********************************************/
void HMolPolAnalysis::BeginOfRun(const G4Run* aRun)
{
  G4cout << "At begin of run" << G4endl;

  // Get run number
  G4int runID = aRun->GetRunID();

  // Construct file name if not set explicitly
  // This is from the default name and the number of times one had
  // simulated a run in the present session of HMolPol (starting with 0)
  if (fRootFileName.size() == 0)
    fRootFileName = fRootFileStem + "_" + Form("%d",runID) + ".root";

  // Create  NEW ROOT file
  G4cout << "###### Analysis: creating ROOT file " << fRootFileName << G4endl;
  fRootFile = new TFile(fRootFileName,"RECREATE","HMolPol ROOT file");
  ConstructRootTree();  //calls the function that constructs the ROOT tree
}

/********************************************
 * Programmer: Valerie Gray
 * Function: EndOfRun
 *
 * Purpose:  Say you are at the end of a run
 *      Save and close the ROOT file
 *
 * Global:
 * Entry Conditions: const G4Run* aRun
 * Return:
 * Called By:
 * Date: 07-11-2013
 * Modified:
 ********************************************/
void HMolPolAnalysis::EndOfRun(const G4Run* aRun)
{
  G4cout << "At end of run" << G4endl;

  // Autosave one last time
  AutoSaveRootTree();

  // Write the data to the ROOT file, closes the file safely
  G4cout << "###### Analysis: closing ROOT file "
      << fRootFileName << G4endl;
  fRootFile->Write(0,TObject::kOverwrite);
  fRootFile->Close();
  fRootFileName = "";
}

/********************************************
 * Programmer: Valerie Gray
 * Function: ConstructRootTree
 *
 * Purpose:  Construct the structure (skeleton) of the ROOT tree in
 *      the ROOT file that was created earlier.
 *
 *      This add each branch to the ROOT file separately
 *              - the Units branch - connected to the
 *                      HMolPolEventUnits files. Has
 *                      all the needed units
 *              - the Primary branch - connected to the
 *                      HMolPolEventPrimary. Has all the
 *                      information about the primary electron
 *              - the Detector branch - connected to the
 *                      HMolPolEventGenericDetectorHit
 *                      Has all the information about
 *                      the hits in the detectors.
 *                      Each detector is in its own
 *                      sub-branch of this branch
 *
 *
 * Global:
 * Entry Conditions: none
 * Return: none
 * Called By:
 * Date: 07-11-2013
 * Modified:
 ********************************************/
void HMolPolAnalysis::ConstructRootTree()
{
  // Create ROOT tree and give it a name
  // the tree name will be HMolPol_Tree in the ROOT file
  fRootTree = new TTree("HMolPol_Tree","HMolPol Simulation Tree");

  // Create a branch with the data structure defined by fRootEvent
  int bufsize = 64000;  //64 kbytes
  int split   = 99;

  // Add units - the units branch
  fUnits = new HMolPolEventUnits();
  fUnitsBranch = fRootTree->Branch("fUnits",
      "HMolPolEventUnits", &fUnits, bufsize, split);

  // Add global event structure - the primary branch
  fPrimary = new HMolPolEventPrimary();
  fPrimaryBranch = fRootTree->Branch("fPrimary",
      "HMolPolEventPrimary", &fPrimary, bufsize, split);

  // Add individual detectors - the Detector branch
  fDetector.resize(fDetectorName.size());
  fDetectorBranch.resize(fDetectorName.size());
  for (size_t i = 0; i < fDetectorName.size(); i++) {
    G4cout << "Adding " << fDetectorName[i] << " to ROOT tree." << G4endl;
    fDetector[i] = new HMolPolEventGenericDetector();
    fDetectorBranch[i] = fRootTree->Branch(TString("f") + TString(fDetectorName[i]),
        "HMolPolEventGenericDetector", &fDetector[i], bufsize, split);
  }

  //TODO get the Run information to work
  // Write run data
  //pUserRunInformation->Write();
}
/********************************************
 * Programmer: Valerie Gray
 * Function: AutoSaveRootTree
 *
 * Purpose:  To Auto Save the root tree as writing too it goes along
 *      Then take all the "small" trees and combine it into one large one.
 *
 * Global:
 * Entry Conditions: none
 * Return: none
 * Called By:
 * Date: 07-11-2013
 * Modified:
 ********************************************/
void HMolPolAnalysis::AutoSaveRootTree()
{
  // save the current ROOT Tree:
  // In case your program crashes before closing the file holding this tree,
  // the file will be automatically recovered when you will connect the file
  // in UPDATE mode.
  // The Tree will be recovered at the status corresponding to the last AutoSave.
  //
  // if option contains "SaveSelf", gDirectory->SaveSelf() is called.
  // This allows another process to analyze the Tree while the Tree is being filled.
  //
  /// \see http://root.cern.ch/root/html/TTree.html#TTree:AutoSave

  // fRootTree -> AutoSave("SaveSelf");
  fRootTree -> AutoSave();
  gDirectory -> Purge(); //Purge old trees

}
