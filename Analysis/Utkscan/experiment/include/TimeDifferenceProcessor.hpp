///@file ExtractDataToCSV.hpp
///@brief Example class for experiment specific setups
///@author K. L. Jordaan
///@date May 8, 2019
#ifndef __TIMEDIFFERENCEPROCESSOR_HPP_
#define __TIMEDIFFERENCEPROCESSOR_HPP_
#include "EventProcessor.hpp"

/// A processor that provides a working, concrete example for users to follow when creating a customized experiment
/// processor.
class TimeDifferenceProcessor : public EventProcessor {
public:
    ///Default Constructor
    TimeDifferenceProcessor();

    ///Constructor to take an argument for the gamma cutoff. You can define anything as arguments here. You'll just
    /// add them to DetectoDriverXmlParser::ParseProcessors when its newing the class.
    TimeDifferenceProcessor(const int ch1, const int ch2, const double timeWindowInMs);

    ///Default Destructor that doesn't need to do anything. Anything that you new in this processor should be cleaned
    /// up here.
    ~TimeDifferenceProcessor();

    /// Declare the plots used in the analysis
    void DeclarePlots();

    /// Process the events that depend on other unassociated detector types.
    ///@param [in] event : the event to process
    ///@returns true if the processing was successful
    bool Process(RawEvent &event);

    // type of event being concidered
    struct eventProc {
        double time;                 // this is returning unix time in ms
        int energyChannel;
        int slot;
        int channel;
    };
    std::vector <eventProc> classData1;
    std::vector <eventProc> classData2;

private:
    ///Sets the detector types that are associated with this processor
    void SetAssociatedTypes();

    /// Method to register our tree and branches when constructing the class
    void SetupRootOutput();

    // variables set in config
    int ch1;
    int ch2;
    double timeWindowInMs;
    TTree *tree_; //!< Pointer to the tree that we're going to define in the constructor
};

#endif
