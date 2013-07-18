#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

#include <QtCore/QFile>
#include <QtCore/QString>

#include <fiff/fiff_constants.h>
#include <fiff/fiff.h>
#include <fiff/fiff_info.h>
#include <fiff/fiff_ch_info.h>
#include <fiff/fiff_stream.h>
#include <fiff/fiff_coord_trans.h>

#include "ReaderBND.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::istringstream;

void printHelp();
bool parseArguments( int argc, char *argv[], string* const iFile, string* const oFile, size_t* const skip );
void printForwardCmd( const string& oFile );

int main( int argc, char *argv[] )
{
    cout << "EEG SENSOR GENERATOR" << endl;
    cout << "--------------------" << endl;

    // parsing arguments

    string iFile, oFile;
    size_t skip = 1;

    if( !parseArguments( argc, argv, &iFile, &oFile, &skip ) )
    {
        return EXIT_FAILURE;
    }

    cout << endl;
    cout << "Reading BEM file from:\n   " << iFile << endl;
    ReaderBND reader( iFile );
    std::vector< ReaderBND::PointT > points;
    reader.readPositions( &points );
    cout << "Read " << points.size() << " positions." << endl;

    cout << endl;
    cout << "Open output FIFF ..." << endl;

    QFile fileOut( QString::fromStdString( oFile ) );
    FIFFLIB::FiffStream::SPtr fiffOutStream = FIFFLIB::FiffStream::start_file( fileOut );

    FIFFLIB::FiffInfo fiffInfoOut;

    FIFFLIB::FiffCoordTrans coordTrans;
    coordTrans.trans.setIdentity();
    coordTrans.invtrans.setIdentity();
    coordTrans.from = FIFFV_COORD_HEAD;
    coordTrans.to = FIFFV_COORD_DEVICE;

    fiffInfoOut.ctf_head_t = coordTrans;
    fiffInfoOut.dev_head_t = coordTrans;
    fiffInfoOut.dev_ctf_t = coordTrans;
    fiffInfoOut.dig_trans = coordTrans;

    cout << "Create channel info ..." << endl;

    size_t count = 0;
    cout << "Skip: " << skip << endl;
    ReaderBND::PointT p;
    for( size_t ch = 0; ch < points.size(); ch += skip )
    {
        FIFFLIB::FiffChInfo chInfo;
        chInfo.coil_type = FIFFV_COIL_EEG;
        chInfo.kind = FIFFV_EEG_CH;
        chInfo.coil_trans.setIdentity();

        p = points[ch];
        chInfo.eeg_loc.col( 0 ) = p;
        chInfo.eeg_loc.col( 1 ) = p;
        chInfo.loc.setZero();
        chInfo.loc( 0, 0 ) = p( 0 );
        chInfo.loc( 1, 0 ) = p( 1 );
        chInfo.loc( 2, 0 ) = p( 2 );

        fiffInfoOut.chs.append( chInfo );
        ++count;
    }

    fiffInfoOut.nchan = count;
    cout << "Took " << count << " positions." << endl;

    cout << "Write FIFF to:\n   " << oFile << endl;

    fiffOutStream->write_info_base( fiffInfoOut );
    fiffOutStream->end_file();

    cout << endl;
    printForwardCmd( oFile );

    return EXIT_SUCCESS;
}

void printHelp()
{
    cout << "Arguments:" << endl;
    cout << "   -i <Input BEM file>" << endl;
    cout << "   -o <Output FIFF file>" << endl;
    cout << "   -s <skip> (optional: take every ... point from input)" << endl;
}

bool parseArguments( int argc, char *argv[], string* const iFile, string* const oFile, size_t* const skip )
{
    opterr = 0;
    int c;
    char iFlag = 0, oFlag = 0;
    while( ( c = getopt( argc, argv, "i:o:s:h" ) ) != -1 )
    {
        switch( c )
        {
            case 'i':
                iFlag = 1;
                *iFile = string( optarg );
                break;
            case 'o':
                oFlag = 1;
                *oFile = string( optarg );
                break;
            case 's':
            {
                istringstream ss( optarg );
                ss >> *skip;
            }
                if( *skip < 1 )
                {
                    cerr << "Wrong argument for skip!" << endl;
                    return false;
                }
                break;
            case 'h':
                printHelp();
                return false;
            default:
                if( isprint( optopt ) )
                    cerr << "Unknown option " << optopt << endl;
                else
                    cerr << "Unknown option character " << optopt << endl;
                printHelp();
                return false;
        }
    }

    if( !iFlag || !oFlag )
    {
        cerr << "Missing arguments!" << endl;
        printHelp();
        return false;
    }

    return true;
}

void printForwardCmd( const string& oFile )
{
    cout << "Now you can create a high resolution leadfield with:" << endl;
    cout << "----------------------------------------------------\n" << endl;
    cout << "mne_forward_solution --eeg --fixed \\\n";
    cout << "--src <source file> \\\n";
    cout << "--mri <COR file> \\\n";
    cout << "--meas " << oFile << " \\\n";
    cout << "--bem <BEM layer file> \\\n";
    cout << "--fwd <output file for forward solution>" << endl;
    cout << "\n----------------------------------------------------" << endl;
}
