#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <string>

namespace util {

std::string dirname(const std::string &path);
std::wstring dirname(const std::wstring &path);
std::string pwd();
void cd(const std::string &path);
void cd(const std::wstring &path);
std::string cp932_to_fs(const std::string &text);

}

#endif // UTILITY_H_INCLUDED
