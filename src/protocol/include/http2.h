#ifndef _HTTP_2_H_
#define _HTTP_2_H_

#include <vector>

class Http2 {
public:
    static void Http2Parse(std::vector<char>& data);
    static void Http2Construct();
public:
    static int headerLen;
};

#endif