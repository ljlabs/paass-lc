///@file ExtractDataToCSV.hpp
///@brief Example class for experiment specific setups
///@author K. L. Jordaan
///@date April 12, 2019
#ifndef __EXTRAXTDATATOCSV_HPP_
#define __EXTRAXTDATATOCSV_HPP_
#include "EventProcessor.hpp"

/// A processor that provides a working, concrete example for users to follow when creating a customized experiment
/// processor.
class ExtractDataToCSV : public EventProcessor {
public:
    ///Default Constructor
    ExtractDataToCSV();

    ///Constructor to take an argument for the gamma cutoff. You can define anything as arguments here. You'll just
    /// add them to DetectoDriverXmlParser::ParseProcessors when its newing the class.
    ExtractDataToCSV(const double &gcut);

    ///Default Destructor that doesn't need to do anything. Anything that you new in this processor should be cleaned
    /// up here.
    ~ExtractDataToCSV();

    /// Declare the plots used in the analysis
    void DeclarePlots();

    /// Process the events that depend on other unassociated detector types.
    ///@param [in] event : the event to process
    ///@returns true if the processing was successful
    bool Process(RawEvent &event);

private:
    ///Sets the detector types that are associated with this processor
    void SetAssociatedTypes();

    /// Method to register our tree and branches when constructing the class
    void SetupRootOutput();

    double gCutoff_; //!< Variable used to set gamma cutoff energy
    TTree *tree_; //!< Pointer to the tree that we're going to define in the constructor
};

#endif