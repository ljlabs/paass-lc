///@file ExtractDataToCSV.hpp
///@brief Example class for experiment specific setups
///@author K. L. Jordaan
///@date April 12, 2019
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "NaICoincidenceProcessor.hpp"

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
        const int D_CH1 = 0;
        const int D_CH2 = 1;
        const int D_COINCIDENCE = 2;                // for graphing coincidence data
        const int coincidenceRange = pow(2, 14);    // the number of bins being recorded
        const int ms = 250 * pow(10, 3);            // number of clock tics in 1 millisecond
    }   
}

using namespace std;
using namespace dammIds::experiment;

///DeclarehistogramXD registers the histograms with ROOT. If you want to define a new histogram add its
/// ID in the experiment namespace above, and then declare it here. The ROOT file contains IDs prefixed with an
/// "h". This is due to a C++ restriction of variable names starting with a number.
void NaICoincidenceProcessor::DeclarePlots() {
    histo.DeclareHistogram1D(
        D_CH1,
        coincidenceRange,
        "Data from ch 1 used for coincidence"
    );
    histo.DeclareHistogram1D(
        D_CH2,
        coincidenceRange,
        "Data from ch 2 used for coincidence"
    );
    histo.DeclareHistogram1D(
        D_COINCIDENCE,
        coincidenceRange,
        "Processed coincidence Energy Spectrum"
    );
}

NaICoincidenceProcessor::NaICoincidenceProcessor() : EventProcessor(OFFSET, RANGE, "NaICoincidenceProcessor") {
    ch1 = 1;
    ch2 = 1;
    timeWindowInMs = 1;
    SetAssociatedTypes();
    SetupRootOutput();
}

NaICoincidenceProcessor::NaICoincidenceProcessor(const int ch1_, const int ch2_, const double timeWindowInMs_) : EventProcessor(OFFSET, RANGE, "NaICoincidenceProcessor") {
    // not running gues cut means to Set the gamma cutoff energy
    ch1 = ch1_;
    ch2 = ch2_;
    timeWindowInMs = timeWindowInMs_;
    SetAssociatedTypes();
    SetupRootOutput();
}

NaICoincidenceProcessor::~NaICoincidenceProcessor() = default;

///Associates this Experiment Processor with template and ge detector types
void NaICoincidenceProcessor::SetAssociatedTypes() {
    associatedTypes.insert("NaI");
}

///Registers the ROOT tree and branches with RootHandler.
void NaICoincidenceProcessor::SetupRootOutput() {
    tree_ = RootHandler::get()->RegisterTree("data", "Tree that stores some of our data");
    RootHandler::get()->RegisterBranch("data", "tof", &tof_, "tof/D");
    RootHandler::get()->RegisterBranch("data", "ten", &tEnergy, "ten/D");
}

///Main processing of data of interest
bool NaICoincidenceProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    int coincidenceSpectrum [coincidenceRange];
    double timeWindow = timeWindowInMs * ms;
    vector <eventProc> data1;
    vector <eventProc> data2;
    for (vector<ChanEvent *>::const_iterator it = event.GetEventList().begin(); it != event.GetEventList().end(); ++it) {
        double time = (*it)->GetTime();                 // this is returning unix time in ms
        double energyChannel = (*it)->GetEnergy();
        int slot = (*it)->GetChanID().GetLocation();
        int channel = (*it)->GetChannelNumber();
        if (channel == ch1) {
            data1.push_back({
                time,
                int(energyChannel),
                slot,
                channel
            });
            histo.Plot(D_CH1, int(energyChannel));
        }
        if (channel == ch2) {
            data2.push_back({
                time,
                int(energyChannel),
                slot,
                channel
            });
            histo.Plot(D_CH2, int(energyChannel));
        }
    }
    while (data1.size() > 0 && data2.size() > 0)
    {
        FILE *fp = fopen("./example.txt","a");
        // time,energyChannel,slot,chanel <-- csv
        fprintf(fp, abs(data1[0].time - data2[0].time) < timeWindow ? "true" : "false");
        fclose(fp);
        // do coincidence check
        if (abs(data1[0].time - data2[0].time) < timeWindow) {
            // ignore garbage data < -- this is a place where optimization may occur
            if (data1[0].energyChannel < coincidenceRange || data1[0].energyChannel >= 0) {
                histo.Plot(D_COINCIDENCE, data1[0].energyChannel);
            }
            // remove both data points in coincidence
            data1.erase(data1.begin());
            data2.erase(data2.begin());
        } else {
            // remove the oldest data point;
            if (data1[0].time < data2[0].time) {
                data1.erase(data1.begin());
            } else {
                data2.erase(data2.begin());
            }
        }
    }
    EndProcess();
    return true;
}
