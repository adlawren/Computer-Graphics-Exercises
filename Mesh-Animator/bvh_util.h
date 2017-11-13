// Notice: The majority of this code was written by Dale Schuurmans, and was
// copied and pasted with his permission

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

#ifndef BVH_UTIL_H
#define BVH_UTIL_H

using namespace std;

string seekLine(ifstream &, string);
string checkNextLine(ifstream &, string);

#endif
