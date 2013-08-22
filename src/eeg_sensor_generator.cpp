#include <cstdlib>
#include <iostream>
#include <limits>
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

using std::cerr;
using std::cout;
using std::endl;
using std::numeric_limits;
using std::string;
using std::stringstream;
using std::vector;

void printHelp();
bool parseArguments( int argc, char *argv[], string* const iFile, string* const oFile, size_t* const skip,
                float* const cutBottom );
void printForwardCmd( const string& oFile );
size_t removeBottomPoints( vector< ReaderBND::PointT >& points, float cutFactor );

int main( int argc, char *argv[] )
{
    cout << "EEG SENSOR GENERATOR" << endl;
    cout << "--------------------" << endl;

    // parsing arguments
    string iFile, oFile;
    size_t skip = 1;
    float cutBottom = 0.33;

    if( !parseArguments( argc, argv, &iFile, &oFile, &skip, &cutBottom ) )
    {
        return EXIT_FAILURE;
    }

    cout << endl;
    cout << "Reading BEM file from:\n   " << iFile << endl;
    ReaderBND reader( iFile );
    vector< ReaderBND::PointT > points;
    reader.readPositions( &points );
    cout << "Read " << points.size() << " positions." << endl;
    cout << "Removing bottom sphere points and downer part of BEM layer." << endl;
    removeBottomPoints( points, cutBottom );
    cout << points.size() << " positions left." << endl;

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
    cout << "   -c <cut bottom factor> (optional: remove points of the bottom sphere, 0 keeps all points - standard: 0.33)"
                    << endl;
}

bool parseArguments( int argc, char *argv[], string* const iFile, string* const oFile, size_t* const skip,
                float* const cutBottom )
{
    opterr = 0;
    int c;
    char iFlag = 0, oFlag = 0;
    while( ( c = getopt( argc, argv, "i:o:s:hc:" ) ) != -1 )
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
                stringstream ss( optarg );
                ss >> *skip;
            }
                if( *skip < 1 )
                {
                    cerr << "Wrong argument for skip!" << endl;
                    return false;
                }
                break;
            case 'c':
            {
                stringstream ss( optarg );
                ss >> *cutBottom;
            }
                if( *cutBottom < 0 || *cutBottom > 1 )
                {
                    cerr << "cutBottom must be between 0.0 and 1.0!" << endl;
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
    cout << "--trans <ASCII file containing a 4x4 identity matrix > \\\n";
    cout << "--meas " << oFile << " \\\n";
    cout << "--bem <BEM layer file> \\\n";
    cout << "--fwd <output file for forward solution>" << endl;
    cout << "\n----------------------------------------------------" << endl;
}

size_t removeBottomPoints( vector< ReaderBND::PointT >& points, float cutFactor )
{
    size_t removed = 0;

    // Find min and max of z-values
    ReaderBND::PointT::Scalar min = numeric_limits< ReaderBND::PointT::Scalar >::max();
    ReaderBND::PointT::Scalar max = numeric_limits< ReaderBND::PointT::Scalar >::min();
    vector< ReaderBND::PointT >::const_iterator cit;
    for( cit = points.begin(); cit != points.end(); ++cit )
    {
        const ReaderBND::PointT::Scalar z = cit->z();
        if( z < min )
        {
            min = z;
        }
        if( z > max )
        {
            max = z;
        }
    }

    // Remove bottom points
    const ReaderBND::PointT::Scalar z_threashold = min + ( max - min ) * cutFactor;
    vector< ReaderBND::PointT >::iterator it;
    for( it = points.begin(); it != points.end(); )
    {
        if( it->z() < z_threashold )
        {
            it = points.erase( it );
            ++removed;
        }
        else
        {
            ++it;
        }
    }

    return removed;
}
