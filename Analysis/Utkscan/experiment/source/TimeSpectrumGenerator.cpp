///@file ExtractDataToCSV.hpp
///@brief Example class for experiment specific setups
///@author K. L. Jordaan
///@date April 12, 2019
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "TimeSpectrumGenerator.hpp"

#include "Display.h"
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "CloverProcessor.hpp"
#include "TemplateProcessor.hpp"

static double tStart;
static double tStop;
static double tDiff;

namespace dammIds {
    ///This namespace contains histogram IDs that we want to use. The actual histogram ID will be the number defined
    /// here plus experiment::OFFSET (6050). D_TSIZE has an ID of 6050 and D_GEENERGY has an ID of 6051.
    namespace experiment {
        const int D_CH1 = 0;
        const int D_CH2 = 1;
        const int D_COINCIDENCE = 2;                // for graphing coincidence data
        const int coincidenceRange = pow(2, 14);    // the number of bins being recorded

        const int periodOfPulsar = 100;             // this is the period of the 10KHz 
        const int DD_START_VS_STOP = 3;
    }   
}

using namespace std;
using namespace dammIds::experiment;
using std::vector;

///DeclarehistogramXD registers the histograms with ROOT. If you want to define a new histogram add its
/// ID in the experiment namespace above, and then declare it here. The ROOT file contains IDs prefixed with an
/// "h". This is due to a C++ restriction of variable names starting with a number.
void TimeSpectrumGenerator::DeclarePlots() {
    histo.DeclareHistogram1D(
        D_CH1,
        coincidenceRange,
        "Data from ch 1 - Raw"
    );
    histo.DeclareHistogram1D(
        D_CH2,
        coincidenceRange,
        "Data from ch 2 - Raw"
    );
    histo.DeclareHistogram1D(
        D_COINCIDENCE,
        coincidenceRange,
        "Time Spectrum"
    );
    histo.DeclareHistogram2D(DD_START_VS_STOP, SA, SA, "Template En vs. Ge En");
}

TimeSpectrumGenerator::TimeSpectrumGenerator() : EventProcessor(OFFSET, RANGE, "TimeSpectrumGenerator") {
    ch1 = 0;
    ch2 = 1;
    SetAssociatedTypes();
    SetupRootOutput();
}

TimeSpectrumGenerator::TimeSpectrumGenerator(const int ch1_, const int ch2_) : EventProcessor(OFFSET, RANGE, "TimeSpectrumGenerator") {
    // not running gues cut means to Set the gamma cutoff energy
    ch1 = ch1_;
    ch2 = ch2_;
    SetAssociatedTypes();
    SetupRootOutput();
    vector <eventProc> classData1;
    vector <eventProc> classData2;
}

TimeSpectrumGenerator::~TimeSpectrumGenerator() = default;

///Associates this Experiment Processor with template and ge detector types
void TimeSpectrumGenerator::SetAssociatedTypes() {
    associatedTypes.insert("NaI");
}

///Registers the ROOT tree and branches with RootHandler.
void TimeSpectrumGenerator::SetupRootOutput() {
    tree_ = RootHandler::get()->RegisterTree("data", "Tree that stores some of our data");
    RootHandler::get()->RegisterBranch("data", "start", &tStart, "tStart/D");
    RootHandler::get()->RegisterBranch("data", "stop", &tStop, "tStop/D");
    RootHandler::get()->RegisterBranch("data", "difference", &tDiff, "tDiff/D");
    RootHandler::get()->RegisterBranch("data", "difference", &tDiff, "tDiff/D");
}

///Main processing of data of interest
bool TimeSpectrumGenerator::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    // int coincidenceSpectrum [coincidenceRange];

    vector<double> data1;
    vector<double> data2;
    int size = 0;
    int start = -1;
    int stop = -1;
    for (vector<ChanEvent *>::const_iterator it = event.GetEventList().begin(); it != event.GetEventList().end(); ++it) {
        double time = (*it)->GetTime() * 8; // 8 NS / SYSTEM Clock 
        // double time = HighResTimingData(*(*it)).GetHighResTimeInNs(); // HighResTimingData(*(*it)).GetHighResTimeInNs(); // to get the time in ms // alot faster but less precise :: (*it)->GetTime();                 // this is returning unix time in ms
        double energyChannel = (*it)->GetEnergy();
        int slot = (*it)->GetChanID().GetLocation();
        int channel = (*it)->GetChannelNumber();
        
        if (channel == ch1) {
            data1.push_back(
                time);
            histo.Plot(D_CH1, int(energyChannel));
        } 
        if (channel == ch2) {
            data2.push_back(
                time);
            histo.Plot(D_CH2, int(energyChannel));
        } 
        while (data1.size() > 0 && data2.size() > 0)
        { 
            tStart = data1.back();
            tStop = data2.back()-300;

            histo.Plot(DD_START_VS_STOP, tStart, tStop);
            tDiff = tStop - tStart;
            tree_->Fill();

            FILE *fp = fopen("./example.txt","a");
            // time,energyChannel,slot,chanel <-- csv
            fprintf(fp, "%lf\n", tDiff);
            fclose(fp);

            histo.Plot(D_COINCIDENCE, 300.0+tDiff);
            // remove both data points in coincidence
            data1.clear();
            data2.clear();
        }
    }
    
    
    EndProcess();
    return true;
}
