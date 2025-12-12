#pragma once

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

class Util
{
public:
    // xxx yyy\r\nzzz
    static string GetOneline(string &buffer, const string &sep)
    {
        auto pos = buffer.find(sep);
        if (pos == string::npos)
            return "";
        string sub = buffer.substr(0, pos);
        return sub;
    }

    static bool readFile(const string &path, string &body)
    {
        ifstream ofs(path,ios_base::binary);
        if (!ofs.is_open())
            return false;
        ofs.read((char *)body.c_str(), body.size());
        ofs.close();
        return true;
    }

};