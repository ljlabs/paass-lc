///@file ExtractDataToCSV.hpp
///@brief Example class for experiment specific setups
///@author K. L. Jordaan
///@date April 12, 2019
#include <iostream>
#include <fstream>

#include "ExtractDataToCSV.hpp"

#include "Display.h"
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "CloverProcessor.hpp"
#include "TemplateProcessor.hpp"

static double tof_;
static double tEnergy;

namespace dammIds {
    ///This namespace contains histogram IDs that we want to use. The actual histogram ID will be the number defined
    /// here plus experiment::OFFSET (6050). D_TSIZE has an ID of 6050 and D_GEENERGY has an ID of 6051.
    namespace experiment {
        const int D_TSIZE = 0; //!< Size of the event
        const int D_GEENERGY = 1; //!< Gamma energy
        const int DD_TENVSGEN = 2; //!< Energy vs Gamma Energy
    }
}

using namespace std;
using namespace dammIds::experiment;

///DeclarehistogramXD registers the histograms with ROOT. If you want to define a new histogram add its
/// ID in the experiment namespace above, and then declare it here. The ROOT file contains IDs prefixed with an
/// "h". This is due to a C++ restriction of variable names starting with a number.
void ExtractDataToCSV::DeclarePlots() {
    histo.DeclareHistogram1D(D_TSIZE, S3, "Num Template Evts");
    histo.DeclareHistogram1D(D_GEENERGY, SA, "Gamma Energy with Cut");
    histo.DeclareHistogram2D(DD_TENVSGEN, SA, SA, "Template En vs. Ge En");
}

ExtractDataToCSV::ExtractDataToCSV() : EventProcessor(OFFSET, RANGE, "ExtractDataToCSV") {
    gCutoff_ = 0.; ///Set the gamma cutoff energy to a default of 0.

    SetAssociatedTypes();
    SetupRootOutput();
}

ExtractDataToCSV::ExtractDataToCSV(const double &gcut) : EventProcessor(OFFSET, RANGE, "ExtractDataToCSV") {
    // not running gues cut means to Set the gamma cutoff energy
    gCutoff_ = gcut;
    SetAssociatedTypes();
    SetupRootOutput();
}

ExtractDataToCSV::~ExtractDataToCSV() = default;

///Associates this Experiment Processor with template and ge detector types
void ExtractDataToCSV::SetAssociatedTypes() {
    associatedTypes.insert("template");
    associatedTypes.insert("clover");
    associatedTypes.insert("NaI");
}

///Registers the ROOT tree and branches with RootHandler.
void ExtractDataToCSV::SetupRootOutput() {
    tree_ = RootHandler::get()->RegisterTree("data", "Tree that stores some of our data");
    RootHandler::get()->RegisterBranch("data", "tof", &tof_, "tof/D");
    RootHandler::get()->RegisterBranch("data", "ten", &tEnergy, "ten/D");
    // FILE *fp = fopen("/home/kyle/Documents/workspace/honsPhy/paass-lc/install/bin/kyle/example.txt","w");
    // fprintf(fp,"&tof_ : %f, &tEnergy : %f", &tof_, &tEnergy);
    // cout << "Write text to be written on file." << endl;
    // fclose(fp);
}

///Main processing of data of interest
bool ExtractDataToCSV::Process(RawEvent &event) {
    try {
        for (vector<ChanEvent *>::const_iterator it = event.GetEventList().begin(); it != event.GetEventList().end(); ++it) {
    
            double time = (*it)->GetTime();
            double energyChannel = (*it)->GetEnergy();
            int slot = (*it)->GetChanID().GetLocation();
            int channel = (*it)->GetChannelNumber();
            
            // cout << endl;
            FILE *fp = fopen("./example.txt","a");
            // time,energyChannel,slot,chanel <-- csv
            fprintf(fp, "%f,%f,%i,%i \n", time, energyChannel, slot, channel);
            fclose(fp);
        }
    } catch (PaassWarning &w) {
        cout << Display::WarningStr("Warning caught at DetectorDriver::ProcessEvent") << endl;
        cout << "\t" << Display::WarningStr(w.what()) << endl;
    } catch (PaassException &e) {
        /// Any exception in activation of basic places, PreProcess and Process
        /// will be intercepted here
        cout << endl << Display::ErrorStr("Exception caught at DetectorDriver::ProcessEvent") << endl;
        throw;
    }
    EndProcess();
    return true;
}
