#ifndef READERBND_H_
#define READERBND_H_

#include <fstream>
#include <vector>
#include <string>

#include <Eigen/Core>

class ReaderBND
{
public:

    typedef Eigen::Vector3d PointT;
    typedef double ScalarT;

    explicit ReaderBND( std::string fname );
    virtual ~ReaderBND();

    bool readPositions( std::vector< PointT >* const points );

private:
    std::ifstream m_ifs;

    bool readNumPos( const std::string& line, size_t* const countPos );
    bool readUnitPos( const std::string& line, double* const factor );
    bool readPositions( std::vector< PointT >* const points , const size_t& countPos, const double factor );
};

#endif  // READERBND_H_
