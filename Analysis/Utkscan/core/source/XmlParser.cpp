///@file XmlParser.cpp
///@brief
///@author S. V. Paulauskas
///@date February 09, 2017
#include <iostream>
#include <stdexcept>
#include <string>

#include "HelperFunctions.hpp"
#include "TrapFilterParameters.hpp"
#include "XmlParser.hpp"

using namespace std;

///We parse the Cfd node here. This node contains the information necessary
/// to the proper function of the various Cfd timing codes. The only
/// currently recognized node here is the Parameters node. If the Cfd node
/// exists then the Paramter node must also exist.
void XmlParser::ParseCfdNode(const pugi::xml_node &node, Globals *globals) {
    if (!node.child("Parameters").empty()) {
        std::map<std::string, std::pair<double, double> > pars;
        for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
            if (std::string(it->name()).compare("Parameters") == 0)
                for (pugi::xml_node_iterator parit = it->begin();
                     parit != it->end(); ++parit)
                    pars.insert(std::make_pair(
                            parit->attribute("name").as_string(),
                            std::make_pair(parit->child("Fraction").attribute("value").as_double(0.),
                                           parit->child("Delay").attribute("value").as_double(0.))));
        globals->SetCfdParameters(pars);
    } else
        throw invalid_argument(CriticalNodeMessage(node.child("Parameters").name()));
    set<string> knownNodes = {"Parameters"};
    WarnOfUnknownChildren(node, knownNodes);
}

///This method parses the Description node. The description node contains
/// information related to the experiment. This node can contain any text
/// that the user thinks could be useful to know when the program starts up.
string XmlParser::ParseDescriptionNode(const pugi::xml_node &node) {
    return node.text().get();
}

///This method parses the fitting node. There are only two free parameters at
/// the moment. The main part of this node is the fitting parameters. These
/// parameters are critical to the function of the software. I the fitting node
/// is present then the Parameters node must also be.
void XmlParser::ParseFittingNode(const pugi::xml_node &node,
                                 Globals *globals) {
    if (!node.child("SigmaBaselineThresh").empty())
        globals->SetSigmaBaselineThreshold(
                node.child("SigmaBaselineThresh").attribute("value").as_double());
    else
        globals->SetSigmaBaselineThreshold(3.0);

    sstream_ << "Sigma Baseline Threshold : " << globals->GetSigmaBaselineThresh();
    messenger_.detail(sstream_.str());
    sstream_.str("");

    if (!node.child("SiPmSigmaBaselineThresh").empty())
        globals->SetSiPmBaselineThreshold(
                node.child("SiPmSigmaBaselineThresh").attribute("value").as_double());
    else
        globals->SetSiPmBaselineThreshold(25.0);

    sstream_ << "SiPM Sigma Baseline Threshold : "
             << globals->GetSiPmSigmaBaselineThresh();
    messenger_.detail(sstream_.str());
    sstream_.str("");

    if (!node.child("Parameters").empty()) {
        std::map<std::string, std::pair<double, double> > pars;
        for (pugi::xml_node_iterator parit = node.child("Parameters").begin();
             parit != node.child("Parameters").end(); ++parit)
            pars.insert(std::make_pair(parit->attribute("name").as_string(),
                                       std::make_pair(
                                               parit->child("Beta").attribute("value").as_double(0.),
                                               parit->child("Gamma").attribute("value").as_double(0.))));
    } else
        throw invalid_argument(CriticalNodeMessage(node.child("Parameters").name()));

    set<string> knownNodes = {"Parameters", "SiPmSigmaBaselineThresh",
                              "SigmaBaselineThresh"};
    WarnOfUnknownChildren(node, knownNodes);
}

///This method parses the Global node. This node contains some of the basic
/// information about the analysis. All of the nodes with the exception of
/// the HasRaw node are critical nodes. They must always be present for the
/// analysis to work properly.
void XmlParser::ParseGlobalNode(const pugi::xml_node &node, Globals *globals) {
    if (!node.child("Revision").empty()) {
        string revision =
                node.child("Revision").attribute("version").as_string();

        if (revision == "A" || revision == "D") {
            globals->SetAdcClockInSeconds(10e-9);
            globals->SetClockInSeconds(10e-9);
            globals->SetFilterClockInSeconds(10e-9);
        } else if (revision == "F") {
            globals->SetAdcClockInSeconds(4e-9);
            globals->SetClockInSeconds(8e-9);
            globals->SetFilterClockInSeconds(8e-9);
        } else {
            throw invalid_argument(
                    "XmlParser::ParseGlobal - The revision \"" +
                    revision + "\", is not known to us. Known "
                            "revisions are A, D, F");
        }
        messenger_.detail("Revision : " + revision);
    } else
        throw invalid_argument(CriticalNodeMessage("Revision"));

    if (!node.child("EventWidth").empty()) {
        double eventLengthInSeconds =
                Conversions::ConvertSecondsWithPrefix(
                        node.child("EventWidth").attribute("value").as_double(
                                0),
                        node.child("EventWidth").attribute("unit").as_string(
                                "None"));
        globals->SetEventLengthInSeconds(eventLengthInSeconds);
        globals->SetEventLengthInTicks(
                (unsigned int) (eventLengthInSeconds /
                                globals->GetClockInSeconds()));
        sstream_ << "Event width: " << eventLengthInSeconds * 1e6 << " us"
                 << ", i.e. " << eventLengthInSeconds / globals->GetClockInSeconds()
                 << " pixie16 clock ticks.";
        messenger_.detail(sstream_.str());
        sstream_.str("");
    } else
        throw invalid_argument(CriticalNodeMessage("EventWidth"));

    if (!node.child("HasRaw").empty())
        globals->SetHasRawHistogramsDefined(
                node.child("HasRaw").attribute("value").as_bool(true));
    else
        globals->SetHasRawHistogramsDefined(true);

    if (!node.child("OutputPath").empty())
        globals->SetHasRawHistogramsDefined(
                node.child("OutputPath").attribute("value").as_bool(true));
    else
        throw invalid_argument(CriticalNodeMessage("OutputPath"));

    set<string> knownNodes = {"Revision", "EventWidth", "HasRaw", "OutputPath"};
    WarnOfUnknownChildren(node, knownNodes);
}

///This method parses the Reject node. The rejection regions are regions of
/// the data files that the user would like to ignore. These rejection
/// regions must be entered with units of seconds.
vector<pair<unsigned int, unsigned int> > XmlParser::ParseRejectNode(
        const pugi::xml_node &node) {
    vector<pair<unsigned int, unsigned int> > regions;
    for (pugi::xml_node time = node.child("Time"); time;
         time = time.next_sibling("Time")) {
        int start = time.attribute("start").as_int(0);
        int end = time.attribute("end").as_int(0);

        std::stringstream ss;
        if (start == 0 || end == 0 || start > end) {
            ss << "Globals: incomplete or wrong rejection region "
               << "declaration: " << start << ", " << end;
            throw invalid_argument(ss.str());
        }

        ss << "Rejection region: " << start << " to " << end << " s";
        messenger_.detail(ss.str());
        regions.push_back(std::make_pair(start, end));
    }
    sstream_ << "Total Number of Rejection Regions : " << regions.size();
    messenger_.detail(sstream_.str());
    sstream_.str("");
    return regions;
}

///This method parses the Root node. For utkscan the root node is always
/// named Configuration. We throw invalid_arguments if we do not find a few
/// of the critial nodes.
void XmlParser::ParseRootNode(const pugi::xml_node &node) {
    set<string> knownChildren = {"Author", "Description", "Global",
                                 "DetectorDriver", "Map", "Vandle",
                                 "TreeCorrelator", "TimeCalibration",
                                 "Trace", "Fitting", "Cfd", "Reject",
                                 "Notebook"};
    if(node.child("Map").empty())
        throw invalid_argument(CriticalNodeMessage(node.child("Map").name()));
    if(node.child("Global").empty())
        throw invalid_argument(CriticalNodeMessage(node.child("Global").name()));
    WarnOfUnknownChildren(node, knownChildren);
}

///This node parses the Trace node. This node contains all of the information
/// necessary for the users to do trace analysis. The only critical node here
/// is the WaveformRange node. If the Trace node exists then this node must
/// also exist.
void XmlParser::ParseTraceNode(const pugi::xml_node &node, Globals *globals) {
    if (!node.child("DiscriminationStart").empty())
        globals->SetDiscriminationStart(
                node.child("DiscriminationStart").attribute("value").as_uint());
    else
        globals->SetDiscriminationStart(3);

    sstream_ << "Discrimination Start : " << globals->GetDiscriminationStart();
    messenger_.detail(sstream_.str());
    sstream_.str("");

    if(!node.child("TraceDelay").empty()) {
        globals->SetTraceDelay(
                node.child("TraceDelay").attribute("value").as_uint());
    } else
        throw CriticalNodeMessage(node.child("TraceDelay").name());

    sstream_ << "Trace Delay : " << globals->GetTraceDelayInNs() << " ns";
    messenger_.detail(sstream_.str());
    sstream_.str("");

    if (!node.child("QdcCompression").empty())
        globals->SetQdcCompression(
                node.child("QdcCompression").attribute("value").as_double());
    else
        globals->SetQdcCompression(1.0);

    sstream_ << "QDC Compression : " << globals->GetQdcCompression();
    messenger_.detail(sstream_.str());
    sstream_.str("");

    if (!node.child("WaveformRange").empty()) {
        std::map<std::string, std::pair<unsigned int, unsigned int> > waveRngs;
        for (pugi::xml_node_iterator waveit = node.child("WaveformRange").begin();
             waveit != node.child("WaveformRange").end(); ++waveit) {
            waveRngs.insert(
                    std::make_pair(
                            waveit->attribute("name").as_string(),
                            std::make_pair(
                                    waveit->child("Low").attribute(
                                            "value").as_int(5),
                                    waveit->child("High").attribute(
                                            "value").as_int(10))));
        }
        globals->SetWaveformRanges(waveRngs);
    } else
        throw invalid_argument(CriticalNodeMessage("WaveformRange"));

    if(!node.child("TrapFilters")) {
        std::map<std::string, std::pair<TrapFilterParameters,
                TrapFilterParameters> > tmp;
        for (pugi::xml_node_iterator trapit = node.child("TrapFilters").begin();
             trapit != node.child("TrapFilters").end(); ++trapit) {
            pugi::xml_node trig = trapit->child("Trigger");
            TrapFilterParameters tfilt(
                    trig.attribute("l").as_double(125),
                    trig.attribute("g").as_double(125),
                    trig.attribute("t").as_double(10));
            pugi::xml_node en = trapit->child("Energy");
            TrapFilterParameters efilt(en.attribute("l").as_double(300),
                                       en.attribute("g").as_double(300),
                                       en.attribute("t").as_double(50));
            tmp.insert(std::make_pair(
                    trapit->attribute("name").as_string(),
                    std::make_pair(tfilt, efilt)));
        }
        globals->SetTrapFilterParameters(tmp);
    }

    set<string> knownNodes = {"DiscriminationStart", "TraceDelay",
                              "QdcCompression", "WaveformRange", "TrapFilters"};
    WarnOfUnknownChildren(node, knownNodes);
}

///Parses the Vandle node. This node contains some information that is
/// specific to the analysis and function of VANDLE detectors.
void XmlParser::ParseVandleNode(const pugi::xml_node &node, Globals *globals) {
    if (!node.child("SpeedOfLightBig").empty())
        globals->SetVandleBigSpeedOfLight(
                node.child("SpeedOfLightBig").attribute("value").as_double());
    else
        globals->SetVandleBigSpeedOfLight(15.22998);

    sstream_ << "Speed of Light in Big Bars : "
             << globals->GetVandleBigSpeedOfLightInCmPerNs() << " cm/ns";
    messenger_.detail(sstream_.str());
    sstream_.str("");

    if (!node.child("SpeedOfLightMedium").empty())
        globals->SetVandleMediumSpeedOfLight(
                node.child("SpeedOfLightMedium").attribute("value").as_double());
    else
        globals->SetVandleMediumSpeedOfLight(15.5);

    sstream_ << "Speed of Light in Medium Bars : "
             << globals->GetVandleMediumSpeedOfLightInCmPerNs() << " cm/ns";
    messenger_.detail(sstream_.str());
    sstream_.str("");

    if (!node.child("SpeedOfLightSmall").empty())
        globals->SetVandleSmallSpeedOfLight(
                node.child("SpeedOfLightSmall").attribute("value").as_double());
    else
        globals->SetVandleSmallSpeedOfLight(12.65822);

    sstream_ << "Speed of Light in Small Bars : "
             << globals->GetVandleSmallSpeedOfLightInCmPerNs() << " cm/ns";
    messenger_.detail(sstream_.str());
    sstream_.str("");

    set<string> knownNodes = {"SpeedOfLightBig", "SpeedOfLightMedium",
                              "SpeedOfLightSmall"};
    WarnOfUnknownChildren(node, knownNodes);
}

///This method simply warns the user of extraneous information that was
/// contained in the node.
void XmlParser::WarnOfUnknownChildren(const pugi::xml_node &node,
                                      const set<string> &knownChildren) {
    for (pugi::xml_node_iterator it = node.begin(); it != node.end(); ++it)
        if (knownChildren.find(it->name()) == knownChildren.end()) {
            sstream_ << "Unknown parameter in " << it->path()
                     << ".  This information is ignored by the program.";
            messenger_.detail(sstream_.str());
            sstream_.str("");
        }
}

///We simply return an error message that will be used to give the user
/// information about what went wrong.
string XmlParser::CriticalNodeMessage(const std::string &name) {
    return "XmlParser::ParseGlobal - We couldn't find the"
            " \"" + name + "\" node. This node is critical to operation.";
}