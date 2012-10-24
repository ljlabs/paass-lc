/** \file MapFile.hpp
 *
 * An independent way to read in the map file with an updated format
 *   from the InitMap() days
 */

#ifndef __MAPFILE_HPP_
#define __MAPFILE_HPP_

#include <set>
#include <string>
#include <vector>
#include <map>
#include "TreeCorrelator.hpp"

/** 
 * A nice simple class which reads in the map file when declared
 *   and updates the external modchan vector accordingly
 */
class MapFile
{
private:
    void TokenizeString(const std::string &in, std::vector<std::string> &out) const;
    void ProcessTokenList(const std::vector<std::string> &list) const;
    bool HasWildcard(const std::string &str) const;
    void TokenToVector(std::string token, std::vector<int> &list, int number) const;
		      
    bool isRead;
public:
    static const std::string defaultFile; ///< default map file

    MapFile(void);
    MapFile(const std::string &filename);


    operator bool() {return isRead;} ///< is the map file valid

    /** Delete all correlator Places. It's called upon command 'end' in 
     * scanor.*/
    ~MapFile() {
        for (map<string, Place*>::iterator it = TreeCorrelator::get().places.begin(); it != TreeCorrelator::get().places.end(); ++it) {
            //cout << "MapFile, removing place " << it->first << endl;
            delete it-> second;
        }
    }
};

#endif // __MAPFILE_HPP_
