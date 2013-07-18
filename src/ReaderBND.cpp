#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "ReaderBND.h"

ReaderBND::ReaderBND( std::string fname )
{
    m_ifs.open( fname.c_str(), std::ifstream::in );

    if( !m_ifs || m_ifs.bad() )
    {
        throw "Could not open file!";
    }
}

ReaderBND::~ReaderBND()
{
    m_ifs.close();
}

bool ReaderBND::readPositions( std::vector< PointT >* const points )
{
    bool rc = false;
    bool hasPosNum = false;
    bool hasPos = false;
    std::string line;
    size_t countPos = 0;
    double factor = 1;
    while( std::getline( m_ifs, line ) && ( !hasPos || !hasPosNum ) )
    {

        if( line.find( "NumberPositions=" ) == 0 )
        {
            rc &= readNumPos( line, &countPos );
            hasPosNum = true;
        }
        else
            if( line.find( "UnitPosition" ) == 0 )
            {
                rc &= readUnitPos( line, &factor );
                hasPosNum = true;
            }
            else
                if( line.find( "Positions" ) == 0 )
                {
                    rc &= readPositions( points, countPos, factor );
                    hasPos = true;
                }

    }
    return rc;
}

bool ReaderBND::readNumPos( const std::string& line, size_t* const countPos )
{
    size_t pos = line.find_first_of( ' ' );
    if( pos == std::string::npos )
    {
        std::cerr << "Could not split string!" << std::endl;
        return false;
    }
    std::string strPos = line.substr( pos + 1 );
    std::istringstream buffer( strPos );
    buffer >> *countPos;
    std::cout << "Number of positions: " << *countPos << std::endl;
    return true;
}

bool ReaderBND::readUnitPos( const std::string& line, double* const factor )
{
    const std::string tmp = "UnitPosition";
    const size_t length = tmp.size();

    if( line.find( "mm" ) > length )
    {
        *factor = 0.001;
    }
    else
        if( line.find( "cm" ) > length )
        {
            *factor = 0.01;
        }
        else
            if( line.find( "dm" ) > length )
            {
                *factor = 0.1;
            }
            else
                if( line.find( "m" ) > length )
                {
                    *factor = 1.0;
                }
                else
                {
                    *factor = 1.0;
                    std::cerr << "Unknown unit in line: " << line << std::endl;
                    return false;
                }

    std::cout << "Factor: " << *factor << std::endl;
    return true;
}

bool ReaderBND::readPositions( std::vector< PointT >* const points, const size_t& countPos, const double factor )
{
    std::string line;
    size_t pos;
    ScalarT x, y, z;
    std::string sValue;
    std::string tail;
    points->reserve( countPos );
    for( size_t i = 0; i < countPos && std::getline( m_ifs, line ); ++i )
    {
        pos = line.find( '\t' );
        sValue = line.substr( 0, pos );
        std::istringstream bufferX( sValue );
        bufferX >> x;

        tail = line.substr( pos + 1 );
        pos = tail.find( '\t' );
        sValue = tail.substr( 0, pos + 1 );
        std::istringstream bufferY( sValue );
        bufferY >> y;

        sValue = tail.substr( pos + 1 );
        std::istringstream bufferZ( sValue );
        bufferZ >> z;

        points->push_back( PointT( x * factor, y * factor, z * factor ) );
    }
    return true;
}
