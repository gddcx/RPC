#ifndef _HTTP_H_
#define _HTTP_H_

#include <vector>
#include <string>
/*
    ------------------------------
    | Body Len | 
    ------------------------------
*/

class Http {
public:
    void ParseHeader(const std::vector<char>& data);
    int GetHeaderLen();
    int SkipParseHttpHeader(int parseOffset);
    static std::string GetHeader(short bodyLen);
private:
    static const int _headerLen = 2;
public:
    short bodyLen = 0;
};

#endif
