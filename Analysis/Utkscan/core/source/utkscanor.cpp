///@file utkscanor.cpp
///@brief The main file that provides us with functionality with SCANOR from HRIBF libraries.
///@author S. V. Paulauskas
///@date September 23, 2016

#include <iostream>

#include <cstring>

#include "DetectorDriver.hpp"
#include "GetArguments.hpp"
#include "Scanor.hpp"
#include "ScanorInterface.hpp"
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"

using std::cout;
using std::endl;

UtkScanInterface *scanner = NULL;
UtkUnpacker *unpacker = NULL;

///@brief Begins setups the interface between SCANOR and the C++ and the
/// Unpacker. It also handles the processing of command line arguments.
extern "C" void startup_() {
    cout << "utkscanor.cpp : Instancing the UtkScanInterface" << endl;
    scanner = new UtkScanInterface();
    unpacker = new UtkUnpacker();

    // Set the output message prefix.
    cout << "utkscan.cpp : Setting the Program Name" << endl;
    scanner->SetProgramName("utkscanor");

    //Set the unpacker object based off what the ScanInterface object created
    ScanorInterface::get()->SetUnpacker(unpacker);

    // Initialize the scanner and handle command line arguments from SCANOR
    cout << "utkscan.cpp : Performing the setup routine" << endl;
    scanner->Setup(GetNumberArguments(), GetArguments(), unpacker);
}

///@brief Defines the main interface with the SCANOR library, the program essentially starts here.
///@param [in] iexist : unused paramter from SCANOR call
extern "C" void drrsub_(uint32_t &iexist) {
    try {
        drrmake_();
        DetectorDriver::get()->DeclarePlots();
        endrr_();
    } catch (std::exception &e) {
        // Any exceptions will be intercepted here
        cout << "Exception caught at Initialize:" << endl;
        cout << "\t" << e.what() << endl;
    }
}

// Catch the exit call from scanor and clean up c++ objects CRT
extern "C" void cleanup_() {
    // Do some cleanup.
    cout << "utkscanor.cpp::cleanup_ - Cleaning up the objects that we created and writing ROOT file to disk.";
    scanner->Close();
    delete RootHandler::get();
    delete scanner;
}