#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <functional>

typedef std::vector<std::string> StringList;

class CsvFile
{
public:
    CsvFile();
    ~CsvFile();

    int open(const std::string &fileName);
    StringList readNextRow(const std::function<bool(StringList)>& isValid);
    bool eof();

private:
    std::ifstream openFile(const std::string &fileName);
    StringList split(std::string inputStr, char delim);

    std::stringstream* m_fileStream = nullptr;
    std::string m_buff = "";
    std::thread* m_loadThread = nullptr;
    std::string m_fileName = "";
};
