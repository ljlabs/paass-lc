///@file ExtractDataToCSV.hpp
///@brief Example class for experiment specific setups
///@author K. L. Jordaan
///@date May 8, 2019
#ifndef __MULTICHANNELCOINCIDENCEPROCESSOR_HPP_
#define __MULTICHANNELCOINCIDENCEPROCESSOR_HPP_
#include "EventProcessor.hpp"
#include "HighResTimingData.hpp"

/// A processor that provides a platform to to find coincidence events between multiple processors
class MultiChannelCoincidenceProcessor : public EventProcessor
{
public:
    ///Default Constructor
    MultiChannelCoincidenceProcessor();

    /// Constructor to take
    ///     a list of left detectors which will be used to generate the spectro gram
    ///     a list of right detectors to find a cocoincidence
    ///     a time window to gate the events
    MultiChannelCoincidenceProcessor(
        const std::vector<std::string> &startList,
        const std::vector<std::string> &stopList,
        const double timeWindowInMs
    );

    ///Default Destructor that doesn't need to do anything. Anything that you new in this processor should be cleaned
    /// up here.
    ~MultiChannelCoincidenceProcessor();

    /// Declare the plots used in the analysis
    void DeclarePlots();

    /// Process the events that depend on other unassociated detector types.
    ///@param [in] event : the event to process
    ///@returns true if the processing was successful
    bool Process(RawEvent &event);

    // type of event being concidered
    struct eventProc
    {
        double time; // just number of clock tics since the system was turned on
        double energyChannel;
        int slot;
        int channel;
    };
    // variables set in config
    std::vector<std::array<std::string, 2>> startStopChannels;
    std::map<std::string, std::vector<eventProc>> allDataMap;
    TTree *tree_; //!< Pointer to the tree that we're going to define in the constructor

private:
    ///Sets the detector types that are associated with this processor
    void SetAssociatedTypes();

    /// Method to register our tree and branches when constructing the class
    void SetupRootOutput(int numberOfBranches);

    double timeWindowInMs;
};

#endif
