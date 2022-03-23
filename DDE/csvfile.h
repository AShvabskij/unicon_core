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
    int setWorkDirectory(const std::string &path);
    StringList readNextRow(const std::function<bool(StringList)>& isValid);
    bool eof();

private:
    std::ifstream openFile(const std::string &filePath);
    StringList split(std::string inputStr, char delim);

    std::stringstream* m_fileStream = nullptr;
    std::string m_buff = "";
    std::thread* m_loadThread = nullptr;
    std::string m_fileName = "";
    std::string m_workDirectory = "";
};
