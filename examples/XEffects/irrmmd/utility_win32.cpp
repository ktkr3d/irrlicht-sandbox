#ifdef _WIN32
#include "utility.h"
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi")
#include <vector>
#include <iostream>

namespace util {
	std::string dirname(const std::string &path)
	{
		std::vector<char> buf;
		buf.reserve(path.size()+1);
		for(size_t i=0; i<path.size(); ++i){
			if(path[i]=='/'){
				buf.push_back('\\');
			}
			else{
				buf.push_back(path[i]);
			}
		}
		buf.push_back(0);
		if(!PathRemoveFileSpecA(&buf[0])){
			return "";
		}
		return std::string(&buf[0]);
	}

	std::wstring dirname(const std::wstring &path)
	{
		std::vector<wchar_t> buf;
		buf.reserve(path.size()+1);
		for(size_t i=0; i<path.size(); ++i){
			if(path[i]==L'/'){
				buf.push_back(L'\\');
			}
			else{
				buf.push_back(path[i]);
			}
		}
		buf.push_back(0);
		if(!PathRemoveFileSpecW(&buf[0])){
			return L"";
		}
		return std::wstring(&buf[0]);
	}

	std::string pwd()
	{
		char path[MAX_PATH];
		if(GetCurrentDirectoryA(MAX_PATH, path)){
			return path;
		}
		return "";
	}

	void cd(const std::string &path)
	{
#ifdef DEBUG
		std::cout << "cd: " << path << std::endl;
#endif
		SetCurrentDirectoryA(path.c_str());
	}

	void cd(const std::wstring &path)
	{
#ifdef DEBUG
		std::wcout << "cd: " << path << std::endl;
#endif
		SetCurrentDirectoryW(path.c_str());
	}

	static std::wstring to_WideChar(UINT uCodePage, const std::string &text)
	{
		int size=MultiByteToWideChar(uCodePage, 0, text.c_str(), -1, NULL, 0);
		std::vector<wchar_t> buf(size);
		MultiByteToWideChar(uCodePage, 0, text.c_str(), -1, &buf[0], static_cast<int>(buf.size()));
		return std::wstring(&buf[0]);
	}

	static std::string to_MultiByte(UINT uCodePage, const std::wstring &text)
	{
		int size=WideCharToMultiByte(uCodePage, 0, text.c_str(), -1, NULL, 0, 0, NULL);
		std::vector<char> buf(size);
		WideCharToMultiByte(uCodePage, 0, text.c_str(), -1, &buf[0], static_cast<int>(buf.size()), 0, NULL);
		return std::string(&buf[0]);
	}

	std::string cp932_to_fs(const std::string &text)
	{
		return to_MultiByte(CP_UTF8, to_WideChar(CP_OEMCP, text).c_str());
	}
}
#endif
