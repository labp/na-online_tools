#include <cstdlib>
#include <string>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include <fiff/fiff.h>
#include <fiff/fiff_constants.h>
#include <fiff/fiff_ch_info.h>
#include <fiff/fiff_coord_trans.h>
#include <fiff/fiff_info.h>
#include <fiff/fiff_stream.h>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

using Eigen::MatrixXi;
using Eigen::Affine;

using namespace FIFFLIB;

typedef Eigen::Vector3d PointT;
typedef Eigen::Matrix3Xd PointsT;
typedef Eigen::Affine3d TransformationT;

void printHelp();
bool parseArguments( int argc, char *argv[], string* const iFile, string* const oFile, string* const tFile );

int main( int argc, char *argv[] )
{
    cout << "EEG SENSOR TRANSFORMATOR" << endl;
    cout << "========================" << endl;

    // Parse arguments //
    // --------------- //
    string iFile, oFile, tFile;

    if( !parseArguments( argc, argv, &iFile, &oFile, &tFile ) )
    {
        return EXIT_FAILURE;
    }

    cout << "input: " << iFile << endl;
    cout << "output: " << oFile << endl;
    cout << "transformation: " << tFile << endl;

    // Read FIFF file //
    // -------------- //
    cout << "---------------------" << endl;
    cout << "Reading FIFF file ..." << endl;

    QFile fiffFile( QString::fromStdString( iFile ) );
    if( !fiffFile.exists() )
    {
        cerr << "FIFF file does not exist!" << endl;
        return EXIT_FAILURE;
    }
    FiffRawData fiffRaw( fiffFile );
    if( fiffRaw.isEmpty() )
    {
        cerr << "Could open FIFF file!" << endl;
        return EXIT_FAILURE;
    }

    MatrixXi picks = fiffRaw.info.pick_types( false, true, false );
    if( picks.size() == 0 )
    {
        cerr << "No EEG available!" << endl;
        return EXIT_FAILURE;
    }

    FiffInfo fiffInfo = fiffRaw.info.pick_info( picks );
    QList< FiffChInfo > chInfos = fiffInfo.chs;
    cout << "Found channels: " << chInfos.size() << endl;

    QList< FiffChInfo >::Iterator it;
    PointsT points( 3, chInfos.size() );
    PointsT::Index col = 0;
    for( it = chInfos.begin(); it != chInfos.end(); ++it )
    {
        points.col( col++ ) = it->eeg_loc.col( 0 );
    }

    // Read transformation file //
    // ------------------------ //
    cout << "-------------------------------" << endl;
    cout << "Reading and apply transformation ..." << endl;

    QFile transFile( QString::fromStdString( tFile ) );
    if( !transFile.exists() )
    {
        cerr << "Transformation file does not exist!" << endl;
        return EXIT_FAILURE;
    }

    if( !transFile.open( QFile::ReadOnly ) )
    {
        cerr << "Could not open transformation file!" << endl;
        return EXIT_FAILURE;
    }
    QTextStream ssTrans( &transFile );

    Eigen::Matrix4d mat;
    short rows = 0;
    while( !ssTrans.atEnd() && rows < 4 )
    {
        QString line = ssTrans.readLine();
        QStringList splits = line.split( " " );
        if( splits.size() != 4 )
        {
            cerr << "Error reading transformation file!" << endl;
            return EXIT_FAILURE;
        }
        mat( rows, 0 ) = splits.at( 0 ).toDouble();
        mat( rows, 1 ) = splits.at( 1 ).toDouble();
        mat( rows, 2 ) = splits.at( 2 ).toDouble();
        mat( rows, 3 ) = splits.at( 3 ).toDouble();
        ++rows;
    }
    transFile.close();
    cout << "Transformation:\n" << mat << endl;
    TransformationT t( mat );

    points = t * points;

    // Create and write FIFF file //
    //----------------------------//
    cout << "-----------------------" << endl;
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
    const QString eegName = "EEG";
    for( PointsT::Index ch = 0; ch < points.cols(); ++ch )
    {
        FiffChInfo chInfo;
        chInfo.ch_name = eegName + ch;
        chInfo.coil_type = FIFFV_COIL_EEG;
        chInfo.kind = FIFFV_EEG_CH;
        chInfo.coil_trans.setIdentity();

        p = points.col( ch );
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

    return EXIT_SUCCESS;
}

void printHelp()
{
    cout << "Arguments:" << endl;
    cout << "   -i <Input FIFF file>" << endl;
    cout << "   -o <Output FIFF file>" << endl;
    cout << "   -t <Transformation file>" << endl;
}

bool parseArguments( int argc, char *argv[], string* const iFile, string* const oFile, string* const tFile )
{
    opterr = 0;
    int c;
    char iFlag = 0, oFlag = 0, tFlag = 0;
    while( ( c = getopt( argc, argv, "i:o:t:h" ) ) != -1 )
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
            case 't':
                tFlag = 1;
                *tFile = string( optarg );
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

    if( !iFlag || !oFlag || !tFlag )
    {
        cerr << "Missing arguments!" << endl;
        printHelp();
        return false;
    }

    return true;
}
