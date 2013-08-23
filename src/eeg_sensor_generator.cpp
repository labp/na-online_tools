#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <sstream>

#include <Eigen/Core>

#include <QtCore/QFile>
#include <QtCore/QString>

#include <fiff/fiff.h>
#include <fiff/fiff_constants.h>
#include <fiff/fiff_ch_info.h>
#include <fiff/fiff_coord_trans.h>
#include <fiff/fiff_info.h>
#include <fiff/fiff_stream.h>
#include <mne/mne_surface.h>

using std::cerr;
using std::cout;
using std::endl;
using std::numeric_limits;
using std::string;
using std::stringstream;
using std::vector;

using namespace FIFFLIB;
using namespace MNELIB;

typedef Eigen::Vector3d PointT;

void printHelp();
bool parseArguments( int argc, char *argv[], string* const iFile, string* const oFile, size_t* const skip,
                float* const cutBottom );
void printForwardCmd( const string& oFile );
size_t removeBottomPoints( vector< PointT >* const points, const MNESurface& surface, float cutFactor );

int main( int argc, char *argv[] )
{
    cout << "EEG SENSOR GENERATOR" << endl;
    cout << "====================" << endl;

    // Parse arguments //
    //-----------------//
    string iFile, oFile;
    size_t skip = 1;
    float cutBottom = 0.33;

    if( !parseArguments( argc, argv, &iFile, &oFile, &skip, &cutBottom ) )
    {
        return EXIT_FAILURE;
    }

    cout << "input: " << iFile << endl;
    cout << "output: " << oFile << endl;
    cout << "skip: " << skip << endl;
    cout << "cutBottom: " << cutBottom << endl;

    // Read BEM file //
    //---------------//
    cout << "--------------------" << endl;
    cout << "Reading BEM file ..." << endl;

    QFile file( QString::fromStdString( iFile ) );
    QList< MNESurface::SPtr > surfaces;
    if( !MNESurface::read( file, surfaces ) )
    {
        cerr << "Could not load BEM layers!" << endl;
        return EXIT_FAILURE;
    }

    MNESurface::ConstSPtr outerSkin;
    QList< MNESurface::SPtr >::ConstIterator cit;
    for( cit = surfaces.begin(); cit != surfaces.end(); ++cit )
    {
        if( ( *cit )->id == FIFFV_BEM_SURF_ID_HEAD )
        {
            cout << "Found outer skin layer." << endl;
            outerSkin = ( *cit );
            break;
        }
    }

    if( outerSkin.isNull() )
    {
        cerr << "Could not find BEM layer: outer skin" << endl;
        return EXIT_FAILURE;
    }
    cout << "Read " << outerSkin->np << " positions." << endl;

    // Remove points of bottom sphere //
    //--------------------------------//
    cout << "--------------------" << endl;
    cout << "Removing points of bottom sphere ..." << endl;
    vector< PointT > points;
    removeBottomPoints( &points, *outerSkin, cutBottom );
    cout << points.size() << " positions left." << endl;

    // Create and write FIFF file //
    //----------------------------//
    cout << "--------------------" << endl;
    cout << "Opening output FIFF ..." << endl;

    QFile fileOut( QString::fromStdString( oFile ) );
    FiffStream::SPtr fiffOutStream = FiffStream::start_file( fileOut );

    FiffInfo fiffInfoOut;

    FiffCoordTrans coordTrans;
    coordTrans.trans.setIdentity();
    coordTrans.invtrans.setIdentity();
    coordTrans.from = FIFFV_COORD_HEAD;
    coordTrans.to = FIFFV_COORD_DEVICE;

    fiffInfoOut.ctf_head_t = coordTrans;
    fiffInfoOut.dev_head_t = coordTrans;
    fiffInfoOut.dev_ctf_t = coordTrans;
    fiffInfoOut.dig_trans = coordTrans;

    cout << "Creating channel info ..." << endl;

    size_t count = 0;

    PointT p;
    for( size_t ch = 0; ch < points.size(); ch += skip )
    {
        FiffChInfo chInfo;
        chInfo.coil_type = FIFFV_COIL_EEG;
        chInfo.kind = FIFFV_EEG_CH;
        chInfo.coil_trans.setIdentity();

        p = points[ch];
        chInfo.eeg_loc.col( 0 ) = p;
        chInfo.eeg_loc.col( 1 ) = p;
        chInfo.loc.setZero();
        chInfo.loc( 0, 0 ) = p.x();
        chInfo.loc( 1, 0 ) = p.y();
        chInfo.loc( 2, 0 ) = p.z();

        fiffInfoOut.chs.append( chInfo );
        ++count;
    }

    fiffInfoOut.nchan = count;
    cout << "Took " << count << " positions." << endl;

    cout << "Writing FIFF ..." << endl;

    fiffOutStream->write_info_base( fiffInfoOut );
    fiffOutStream->end_file();

    cout << "====================" << endl;
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

size_t removeBottomPoints( vector< PointT >* const points, const MNESurface& surface, float cutFactor )
{
    size_t removed = 0;
    const MNESurface::PointsT& sPoints = surface.rr;

    // Find min and max of z-values //
    //------------------------------//
    MNESurface::PointsT::Scalar min = numeric_limits< MNESurface::PointsT::Scalar >::max();
    MNESurface::PointsT::Scalar max = numeric_limits< MNESurface::PointsT::Scalar >::min();
    for( MNESurface::PointsT::Index c = 0; c < sPoints.cols(); ++c )
    {
        const MNESurface::PointsT::Scalar z = sPoints.col( c ).z();
        if( z < min )
        {
            min = z;
        }
        if( z > max )
        {
            max = z;
        }
    }

    // Remove bottom points //
    //----------------------//
    const MNESurface::PointsT::Scalar z_threashold = min + ( max - min ) * cutFactor;
    for( MNESurface::PointsT::Index c = 0; c < sPoints.cols(); ++c )
    {
        const PointT pIn = sPoints.col( c ).cast< double >();
        if( pIn.z() > z_threashold )
        {
            points->push_back( pIn );
        }
        else
        {
            ++removed;
        }
    }

    return removed;
}
