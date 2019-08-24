///@file ExtractDataToCSV.hpp
///@brief Example class for experiment specific setups
///@author K. L. Jordaan
///@date April 12, 2019
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "MultiChannelCoincidenceProcessor.hpp"

#include "Display.h"
#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "CloverProcessor.hpp"
#include "TemplateProcessor.hpp"

static double coincidenceData;

namespace dammIds
{
///This namespace contains histogram IDs that we want to use. The actual histogram ID will be the number defined
/// here plus experiment::OFFSET (6050). D_TSIZE has an ID of 6050 and D_GEENERGY has an ID of 6051.
namespace experiment
{
const int D_CH1 = 0;
const int D_CH2 = 1;
const int D_COINCIDENCE = 2;             // for graphing coincidence data
const int coincidenceRange = pow(2, 14); // the number of bins being recorded

const int periodOfPulsar = 100; // this is the period of the 10KHz
} // namespace experiment
} // namespace dammIds

using namespace std;
using namespace dammIds::experiment;
using std::vector;

///DeclarehistogramXD registers the histograms with ROOT. If you want to define a new histogram add its
/// ID in the experiment namespace above, and then declare it here. The ROOT file contains IDs prefixed with an
/// "h". This is due to a C++ restriction of variable names starting with a number.
void MultiChannelCoincidenceProcessor::DeclarePlots()
{
}

MultiChannelCoincidenceProcessor::MultiChannelCoincidenceProcessor() : EventProcessor(OFFSET, RANGE, "MultiChannelCoincidenceProcessor")
{
    startStopChannels = {};
    timeWindowInMs = 1;
    SetAssociatedTypes();
    SetupRootOutput(startStopChannels.size());
}

/// TODO :: the strings of start and stop lists need to be parsed into a vector here
MultiChannelCoincidenceProcessor::MultiChannelCoincidenceProcessor(
    const std::vector<std::string> &startList_,
    const std::vector<std::string> &stopList_,
    const double timeWindowInMs_) : EventProcessor(OFFSET, RANGE, "MultiChannelCoincidenceProcessor")
{
    startStopChannels = {};
    // handle the parsing of the start and stop lists
    if (!startList_.empty() && !stopList_.empty() && startList_.size() == stopList_.size())
    {
        for (uint i = 0; i < startList_.size(); i++)
        {
            startStopChannels.push_back({startList_[i], stopList_[i]});
        }
    }

    // not running gues cut means to Set the gamma cutoff energy
    timeWindowInMs = timeWindowInMs_;
    SetAssociatedTypes();
    SetupRootOutput(startStopChannels.size());
}

MultiChannelCoincidenceProcessor::~MultiChannelCoincidenceProcessor() = default;

///Associates this Experiment Processor with template and ge detector types
void MultiChannelCoincidenceProcessor::SetAssociatedTypes()
{
    associatedTypes.insert("NaI");
}

///Registers the ROOT tree and branches with RootHandler.
void MultiChannelCoincidenceProcessor::SetupRootOutput(int numberOfBranches)
{
    tree_ = RootHandler::get()->RegisterTree("data", "Tree that stores some of our coincidence data");
    int i = 0;
        std::string key = startStopChannels[i][0] + " " + startStopChannels[i][1];
        RootHandler::get()->RegisterBranch("data", "coincidence of channels ", &coincidenceData, "energy/D");

}

///Main processing of data of interest
bool MultiChannelCoincidenceProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
        return (false);
    // int coincidenceSpectrum [coincidenceRange];

    double timeWindow = timeWindowInMs * 1000; // as the time window is in ns
    int size = 0;
    for (vector<ChanEvent *>::const_iterator it = event.GetEventList().begin(); it != event.GetEventList().end(); ++it)
    {
        double time = (*it)->GetTime();
        // double time = HighResTimingData(*(*it)).GetHighResTimeInNs(); // HighResTimingData(*(*it)).GetHighResTimeInNs(); // to get the time in ms // alot faster but less precise :: (*it)->GetTime();                 // this is returning unix time in ms
        double energyChannel = (*it)->GetEnergy();
        int slot = (*it)->GetChanID().GetLocation();
        int channel = (*it)->GetChannelNumber();

        std::map<std::string, std::vector<eventProc>>::const_iterator pos = allDataMap.find(to_string(channel));

        if (pos == allDataMap.end())
        {
            allDataMap.insert(std::make_pair<std::string, std::vector<eventProc>>(to_string(channel), {{time,
                                                                                                        energyChannel,
                                                                                                        slot,
                                                                                                        channel}}));
        }
        else
        {
            allDataMap[to_string(channel)].push_back({time,
                                                      energyChannel,
                                                      slot,
                                                      channel});
        }
    }
    /// loop through each of the coincidence channels
    for (uint i1 = 0; i1 < startStopChannels.size(); i1++)
    {
        std::vector<eventProc> data1 = allDataMap[startStopChannels[i1][0]];
        std::vector<eventProc> data2 = allDataMap[startStopChannels[i1][1]];
        while (data1.size() > 0 && data2.size() > 0)
        {
            // do coincidence check
            if (abs(data1[0].time - data2[0].time) < timeWindow)
            {
                // to put data into the tree for viewing
                std::string key = startStopChannels[i1][0] + " " + startStopChannels[i1][1];
                coincidenceData = data1[0].energyChannel;
                FILE *fp = fopen("./example.txt", "a");
                // time,energyChannel,slot,chanel <-- csv
                fprintf(fp, "%lf\n", coincidenceData);
                fclose(fp);
                tree_->Fill();
                data1.erase(data1.begin());
                data2.erase(data2.begin());
            }
            else
            {
                // remove the oldest data point;
                if (data1[0].time < data2[0].time)
                {
                    data1.erase(data1.begin());
                }
                else
                {
                    data2.erase(data2.begin());
                }
            }
        }
    }

    EndProcess();
    return true;
}
