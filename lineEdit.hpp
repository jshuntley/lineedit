/************************************************************************
 * @Author : Josh Huntley
 * @Class : CptS 223; Fall 2025
 * @Assignment : Linear Structures Programming Assignment
 * @Date : Sep 24, 2025
 * @Description : Line editor header file. Contains the LineEdit class and class members.
 *************************************************************************/

#ifndef LINEEDIT_HPP
#define LINEEDIT_HPP

#pragma once

#include <filesystem>
#include <string>
#include <vector>

class LineEdit
{
public:
    LineEdit() = default;
    explicit LineEdit(std::filesystem::path filePath);

    void loop();

private:
    std::filesystem::path filePath_{};
    std::vector<std::string> lines_{};
    size_t currentLine_{1};
    bool dirty_{false};

    void loadFile();

    std::string readLine();

    bool saveFile(const std::filesystem::path &filePath);

    bool insert(const std::string &text);

    bool append(const std::string &text);

    bool jump(long long a);

    bool remove(long long a = 0, long long b = 0);

    void printLine(size_t lineNumber) const;
    bool printSelected(long long a = 0, long long b = 0);

    void printPrompt(void);
};

#endif
