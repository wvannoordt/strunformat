#ifndef STR_UNFORMAT_H
#define STR_UNFORMAT_H

#include <string>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
namespace cmf
{
    /// @brief Gets file extension from a file name
    /// @param filename The name of the file
    /// @author WVN
    static inline std::string GetFileExtension(std::string filename)
    {
        size_t dot = filename.find_last_of(".");
        if (dot == std::string::npos) return "";
        return filename.substr(dot+1, filename.length()-dot);
    }

    /// @brief Checks if a file exists
    /// @param filename The name of the file
    /// @author WVN
    static inline bool FileExists(std::string filename)
    {
        std::ifstream f(filename.c_str());
        return f.good();
    }

    /// @brief Returns a string formatted with units with order-of-magnitude prefixes
    /// @param quantity The number of whatevers
    /// @param unit The unit you have (e.g. m for meters)
    /// @param pitch The multiplier between each level (e.g. 1000 factor of difference between m and km)
    /// @author WVN
    static inline std::string UnitString(size_t quantity, std::string unit, size_t pitch)
    {
        int numOrders = 0;
        std::string prefixes[7];
        prefixes[0] = "";
        prefixes[1] = "K";
        prefixes[2] = "M";
        prefixes[3] = "G";
        prefixes[4] = "T";
        prefixes[5] = "P";
        prefixes[6] = "P";
        double dq = (double)quantity;
        while (dq > pitch)
        {
            numOrders++;
            dq/=pitch;
            if (numOrders==6) break;
        }
        return std::to_string(dq) + " " + prefixes[numOrders] + unit;
    }

    /// @brief Returns a string formatted with units with order-of-magnitude prefixes
    /// @param quantity The number of whatevers
    /// @param unit The unit you have (e.g. m for meters)
    /// @author WVN
    static inline std::string UnitString(size_t quantity, std::string unit)
    {
        return UnitString(quantity, unit, 1000);
    }

    /// @brief Returns a number string formatted with commas
    /// @param num The number to make into a string
    /// @author WVN
    static inline std::string NiceCommaString(size_t num)
    {
        std::string numstr = std::to_string(num);
        std::string output = "";
        for (int i = numstr.length()-1; i >= 0; i--)
        {
            output = numstr[i] + output;
            if ((i!=0)&&(i!=numstr.length()-1)&&((numstr.length()-i)%3)==0) output = "," + output;
        }
        return output;
    }

    /// @brief Returns a number string padded with zeros
    /// @param num The number to make into a string
    /// @author WVN
    static inline std::string ZFill(int val, int znum)
    {
        std::string output = std::to_string(val);
        while (output.length()<znum) output = "0" + output;
        return output;
    }

    /// @brief Returns a string with the hex address of a pointer
    /// @param ptr The pointer to convert
    /// @author WVN
    static inline std::string PtrToStr(const void* ptr)
    {
        std::ostringstream address;
        address << ptr;
        std::string name = address.str();
        return name;
    }

    /// @brief Helper functin for strformat()
    /// @param subStrings The vector of substrings to format
    /// @param templateStr The template string
    /// @author WVN
    static inline void GetFormatSubstrings(std::vector<std::string>& subStrings, std::string templateStr)
    {
        std::string delimiter = "{}";
        std::string templateStrCopy = templateStr;
        size_t pos = 0;
        std::string token;
        while ((pos = templateStrCopy.find(delimiter)) != std::string::npos)
        {
            token = templateStrCopy.substr(0, pos);
            subStrings.push_back(token);
            templateStrCopy.erase(0, pos + delimiter.length());
        }
        subStrings.push_back(templateStrCopy);
    }
    
    /// @brief Helper functin for strformat()
    /// @param subStrings The vector of substrings to format
    /// @param templateStr The template string
    /// @author WVN
    static inline std::vector<std::string> StringSplit(std::string templateStr, std::string delimiter)
    {
        std::vector<std::string> subStrings;
        std::string templateStrCopy = templateStr;
        size_t pos = 0;
        std::string token;
        while ((pos = templateStrCopy.find(delimiter)) != std::string::npos)
        {
            token = templateStrCopy.substr(0, pos);
            subStrings.push_back(token);
            templateStrCopy.erase(0, pos + delimiter.length());
        }
        subStrings.push_back(templateStrCopy);
        return subStrings;
    }

    /// @brief Helper function for strformat
    /// @author WVN
    template <typename T> static inline void strformat_recursive (std::string& templateStr, std::ostringstream& strstream, std::vector<std::string>& subStrings, int& lev, T t)
    {
        strstream << subStrings[lev] << t;
        lev++;
    }

    /// @brief Helper function for strformat
    /// @author WVN
    template <typename T, typename... Ts> static inline void strformat_recursive (std::string& templateStr, std::ostringstream& strstream, std::vector<std::string>& subStrings, int& lev, T t, Ts... ts)
    {
        strstream << subStrings[lev] << t;
        lev++;
        strformat_recursive(templateStr, strstream, subStrings, lev, ts...);
    }

    /// @brief Formats a string arbitrarily
    /// @param templateStr The template string. Each argument that follows will replace an instance of "{}"
    /// @param ts The arguments to format, must have stream operator defined
    /// @author WVN
    template <typename... Ts> static inline std::string strformat (std::string templateStr, Ts... ts)
    {
        std::ostringstream strstream;
        std::vector<std::string> subStrings;
        GetFormatSubstrings(subStrings, templateStr);
        if ((sizeof...(Ts))!=(subStrings.size()-1))
        {
            //[INSERT ERROR HERE]
        }
        int lev = 0;
        strformat_recursive(templateStr, strstream, subStrings, lev, ts...);
        strstream << subStrings[lev];
        return strstream.str();
    }
    
    /// @brief Helper function for strformat
    /// @author WVN
    template <typename T> static inline void strunformat_recursive (std::string data, std::string& templateStr, std::vector<std::string>& subStrings, int& lev, T& t)
    {
        std::string& startToken = subStrings[lev];
        std::string& endToken = subStrings[lev+1];
        size_t spos = data.find(startToken);
        if (spos==std::string::npos) return;
        size_t epos = data.find(endToken, spos+startToken.length());
        if (endToken=="") epos = data.length();
        if (epos==std::string::npos) return;
        size_t subStrStart = spos+startToken.length();
        size_t subStrLength = epos-spos-startToken.length();
        t = data.substr(subStrStart, subStrLength);
        lev++;
    }
    
    /// @brief Helper function for unstrformat
    /// @author WVN
    template <typename T, typename... Ts> static inline void strunformat_recursive (std::string data, std::string& templateStr, std::vector<std::string>& subStrings, int& lev, T& t, Ts&... ts)
    {
        std::string& startToken = subStrings[lev];
        std::string& endToken = subStrings[lev+1];
        size_t spos = data.find(startToken);
        if (spos==std::string::npos) return;
        size_t epos = data.find(endToken, spos+startToken.length());
        if (endToken=="") epos = data.length();
        if (epos==std::string::npos) return;
        size_t subStrStart = spos+startToken.length();
        size_t subStrLength = epos-spos-startToken.length();
        t = data.substr(subStrStart, subStrLength);
        std::string dataSubStr = data.substr(epos, data.length()-epos);
        lev++;
        strunformat_recursive(dataSubStr, templateStr, subStrings, lev, ts...);
    }
    
    /// @brief Un-formats a string arbitrarily (parses based on pattern). This function is not guaranteed to work for complex parsing
    /// @param templateStr The template string.
    /// @param ts The arguments to populate, must be strings
    /// @author WVN
    template <typename... Ts> static inline void strunformat (std::string data, std::string templateStr, Ts&... ts)
    {
        std::vector<std::string> subStrings;
        GetFormatSubstrings(subStrings, templateStr);
        if ((sizeof...(Ts))!=(subStrings.size()-1))
        {
            //[INSERT ERROR HERE]
        }
        int lev = 0;
        strunformat_recursive(data, templateStr, subStrings, lev, ts...);
    }
}

#endif
