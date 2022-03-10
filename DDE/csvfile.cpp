#include "csvfile.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

const std::string FILE_ERROR = "Data file is not found!\n";
const std::string FILE_PARSE_ERROR = "Error while parsing the file!\n";
const int DATA_READ_ERROR = -1;
const int OPEN_FILE_ERROR = -2;

CsvFile::CsvFile()
{
}

CsvFile::~CsvFile()
{
    delete m_fileStream;
    delete m_loadThread;
}

int CsvFile::open(const string &fileName)
{
    if (m_fileName != fileName || m_buff.empty()) {
        ifstream file = openFile(fileName);
        if (!file.is_open()) {
            cout << FILE_ERROR;
            return OPEN_FILE_ERROR;
        }

        m_buff.clear();
        file.seekg(0, std::ios::end);
        m_buff.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        m_buff.assign((std::istreambuf_iterator<char>(file)),
                   std::istreambuf_iterator<char>());

        file.close();
        m_fileName = fileName;
    }

    if (m_buff.empty()) {
        cout << FILE_PARSE_ERROR;
        return DATA_READ_ERROR;
    }

    delete m_fileStream;
    m_fileStream = new std::stringstream(m_buff);

    return 0;
}

std::ifstream CsvFile::openFile(const std::string& fileName)
{
    std::ifstream file(".\\data\\" + fileName);
    if (!file.is_open()) {
        file.open(fileName);
    }

    int res = file.is_open() ? 0 : -1;
    if (res != 0) {
        cout << OPEN_FILE_ERROR;
    }

    return file;
}

StringList CsvFile::readNextRow(const std::function<bool(StringList)> &isValid)
{
    std::vector<string> res;
/*
    if (m_loadThread) {
        m_loadThread->join();
        delete m_loadThread;
        m_loadThread = nullptr;
    }
*/
    if (!m_fileStream) {
        return res;
    }

    while (!m_fileStream->eof()) {
        std::string row;
        std::getline(*m_fileStream, row);
        StringList res = split(row, ';');
        if (isValid(res)) {
            return res;
        }
    }

    return StringList();
}

StringList CsvFile::split(std::string inputStr, char delim)
{
    std::vector<std::string> res;
    std::string item;
    std::stringstream ss(inputStr);

    while(std::getline(ss, item, delim)) {
        res.push_back(item);
    }

    return res;
}

bool CsvFile::eof()
{
    return m_fileStream->eof();
}
