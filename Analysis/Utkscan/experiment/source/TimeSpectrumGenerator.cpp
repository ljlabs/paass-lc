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

        const int periodOfPulsar = 100;             // this is the period of the 10KHz 
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
    RootHandler::get()->RegisterBranch("data", "tof", &tof_, "tof/D");
    RootHandler::get()->RegisterBranch("data", "ten", &tEnergy, "ten/D");
}

///Main processing of data of interest
bool TimeSpectrumGenerator::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    // int coincidenceSpectrum [coincidenceRange];

    vector<eventProc> data1;
    vector<eventProc> data2;
    int size = 0;
    for (vector<ChanEvent *>::const_iterator it = event.GetEventList().begin(); it != event.GetEventList().end(); ++it) {
        double time = (*it)->GetTime(); 
        // double time = HighResTimingData(*(*it)).GetHighResTimeInNs(); // HighResTimingData(*(*it)).GetHighResTimeInNs(); // to get the time in ms // alot faster but less precise :: (*it)->GetTime();                 // this is returning unix time in ms
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
        size = size + 1;
    }
    FILE *fp = fopen("./example.txt","a");
    // time,energyChannel,slot,chanel <-- csv
    fprintf(fp, "number of events %d \n", size );
    fclose(fp);
    /*
    while (data1.size() > 0 && data2.size() > 0) {
        histo.Plot(D_COINCIDENCE, data1[0].energyChannel);
        data1.erase(data1.begin());
        data2.erase(data2.begin());
    }
*/
    while (data1.size() > 0 && data2.size() > 0)
    {
        FILE *fp = fopen("./example.txt","a");
        // time,energyChannel,slot,chanel <-- csv
        fprintf(fp, "p1 %lf, p2 %lf, time diff in events %lf \n", data1[0].time, data2[0].time, data1[0].time - data2[0].time);
        fclose(fp);
        // do coincidence check
        if ((data1[0].time - data2[0].time) < pow(2, 14) && (data2[0].time - data1[0].time) + pow(2, 13) >= 0) {
            histo.Plot(D_COINCIDENCE, data2[0].time - data1[0].time + pow(2, 13));
        }
        // remove both data points in coincidence
        data1.erase(data1.begin());
        data2.erase(data2.begin());
    }
    
    EndProcess();
    return true;
}
