/************************************************************************
 * @Author : Josh Huntley
 * @Class : CptS 223; Fall 2025
 * @Assignment : Linear Structures Programming Assignment
 * @Date : Sep 24, 2025
 * @Description : Line editor main cpp file. Contains the core functionality of the program.
 *************************************************************************/

#include "lineEdit.hpp"

#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

/**
 * Namespace for helper functions
 */
namespace
{

    std::string art = R"( 
 _     _              _____    _ _ _   
| |   (_)            |  ___|  | (_) |  
| |    _ _ __   ___  | |__  __| |_| |_ 
| |   | | '_ \ / _ \ |  __|/ _` | | __|
| |___| | | | |  __/ | |__| (_| | | |_ 
\_____/_|_| |_|\___| \____/\__,_|_|\__|)";

    std::string kSep = "----------------------------------------------------------------------------------------------------\n";

    /**
     * @brief Print the help / command menu
     */
    void menu(void)
    {
        std::cout << kSep << "Commands:\n"
                             "  W [<filename>]  Write to file.\n"
                             "  J <line>        Jump to line.\n"
                             "  I [<text>]      Insert at current (before). Blank line if no text.\n"
                             "  A [<text>]      Insert after current. Blank line if no text.\n"
                             "  L [a] [b]       List lines. No args=all, one line, or inclusive range.\n"
                             "  D [a] [b]       Delete line. No args=current, one line, or inclusive range.\n"
                             "  C               Clear screen.\n"
                             "  Q               Quit; prompt to save if modified.\n"
                             "  H               Help.\n"
                  << kSep;
    }

    bool prompt = false;

    /**
     * @brief Struct for parsed line
     */
    struct Parsed
    {
        char cmd = '\0';
        std::string rest;
        bool ok = false;
    };

    /**
     * @brief Parse line input to split commands from params or input
     *
     * @param line The line of input to be parsed
     * @return p - a struct with vars: char 'cmd', string 'rest', bool 'ok'
     * @sa Parsed
     */
    Parsed parse_cmd(const std::string &line)
    {
        Parsed p;

        // skip leading whitespace
        size_t i = line.find_first_not_of(" \t\r\n");
        if (i == std::string::npos)
            return p; // empty input

        char c = line[i];

        if (c < 'A' || c > 'Z')
        {
            return p; // reject if not uppercase
        }

        p.cmd = c;

        size_t j = i + 1;

        if (j == line.size())
        {
            // nothing after the command → valid, no args
            p.ok = true;
            return p;
        }

        if (line[j] != ' ')
        {
            // if it's not a space, then it's not a single-letter command
            std::cout << "[invalid command]\n";
            menu();
            return p;
        }

        // consume exactly one space
        ++j;
        if (j < line.size())
        {
            p.rest = line.substr(j); // capture all remaining text
        }

        p.ok = true;
        return p;
    }

    /**
     * @brief Get a file path
     *
     * @return path to the file
     */
    std::filesystem::path getPath(void)
    {
        for (;;)
        {
            std::cout << "Enter a file path: ";
            std::string s;

            if (!std::getline(std::cin, s))
                return {};

            // trim
            auto first = s.find_first_not_of(" \t\r\n");

            if (first == std::string::npos)
                continue;

            auto last = s.find_last_not_of(" \t\r\n");

            std::ifstream file(s);
            if (file)
            {
                std::string ans;

                std::cout << "Overwrite \"" << s << "\"? [y, N] ";

                std::getline(std::cin, ans);

                if (ans.empty() || (ans[0] == 'N') || (ans[0] == 'n'))
                    getPath();
            }

            file.close();

            return std::filesystem::path(s.substr(first, last - first + 1));
        }
    }
} // namespace

/**
 * @brief Class constructor for starting with a file/path
 *
 * @param filePath The file / filepath to read
 */
LineEdit::LineEdit(std::filesystem::path filePath) : filePath_(std::move(filePath)) { loadFile(); }

/**
 * @brief Loads a file from disk into memory for editing
 *
 * @param filePath The file / filepath to read
 * @post The chosen file is loaded into memory, then closes the file
 */
void LineEdit::loadFile(void)
{
    lines_.clear();

    try
    {
        std::ifstream inFile(filePath_);

        std::string line;

        while (std::getline(inFile, line))
        {
            lines_.push_back(line);
            currentLine_++;
        }

        inFile.close();
    }
    catch (const std::exception &e)
    {
        // if file doesn't exsist, move on
    }
}

/**
 * @brief Save the file to disk. If run without filename, will prompt. If run with filename, saves to that one
 *
 * @param filePath The filename / path to save to
 * @return boolean to indicate success or failure
 * @post File will be saved to the chosen file path, then closed. Marks file clean after saving
 */
bool LineEdit::saveFile(const std::filesystem::path &filePath)
{
    std::ofstream outFile(filePath);

    if (!outFile)
    {
        std::cout << "Unable to save to " + filePath.string();
        return false;
    }

    for (const auto &line : lines_)
        outFile << line << '\n';

    dirty_ = false;

    outFile.close();

    std::cout << "Saved " << lines_.size() << " lines to " << filePath << '\n';

    return true;
}

/**
 * @brief Insert entry at current line
 *
 * @param line The line of input to be inserted
 * @return Boolean indicating success or failure
 * @post The entered content will be inserted at the current line index. File status changes to dirty
 */
bool LineEdit::insert(const std::string &line)
{
    if (lines_.empty())
        lines_.push_back(line);
    else
        lines_.insert(lines_.begin() + (currentLine_ - 1), line);

    printSelected(currentLine_);

    currentLine_++;

    dirty_ = true;

    return true;
}

/**
 * @brief Append entry at next line index
 *
 * @param line The line of input to be appended
 * @return Boolean indicating success or failure
 * @post The entered content will be inserted at the next line index. File status changes to dirty
 */
bool LineEdit::append(const std::string &line)
{
    if (lines_.empty() || currentLine_ > lines_.size())
    {
        lines_.push_back("");
        ++currentLine_;
        lines_.insert(lines_.begin() + (currentLine_ - 1), line);
    }
    else
        lines_.insert(lines_.begin() + currentLine_, line);

    currentLine_ == lines_.size() ? printSelected(currentLine_) : printSelected(currentLine_ + 1);

    dirty_ = true;

    return true;
}

/**
 * @brief Jump to specific line indexQ
 * @param a The index to jump to
 * @return boolean to indicate success or failure
 * @post The user will be moved to the chosen line index
 */
bool LineEdit::jump(long long a)
{
    const size_t N = lines_.size();

    if (N == 0)
        return false;

    if (a == 0)
        return false;

    if (a <= (long long)N && a >= -1)
    {
        currentLine_ = (a == -1) ? static_cast<int>(N) : a;
        return true;
    }

    return false;
}

/**
 * @brief Remove current line, selected line, or range of lines
 *
 * @param a Optional value to select a single line to delete / the first index in the range to delete
 * @param b Optional value used to select last line in range to delete
 * @return Boolean indicating success or failure
 * @post Selected line(s) will be deleted. File status changes to dirty
 */
bool LineEdit::remove(long long a, long long b)
{
    const size_t N = lines_.size();

    if (N == 0)
    {
        std::cout << kSep << "(empty)\n"
                  << kSep;

        dirty_ = true;
        return true;
    }

    if (a == 0 && b == 0)
    {
        lines_.erase(lines_.begin() + (currentLine_ - 1));

        if (currentLine_ > lines_.size())
            currentLine_ = lines_.size() + 1;

        dirty_ = true;
        return true;
    }

    if (b == 0)
    {
        if (a == -1)
        {
            lines_.erase(lines_.end());

            if (currentLine_ > lines_.size())
                currentLine_ = lines_.size() + 1;

            dirty_ = true;
            return true;
        }

        if (a >= 1 && a <= (long long)N)
        {
            lines_.erase(lines_.begin() + (a - 1));

            if (currentLine_ > lines_.size())
                currentLine_ = lines_.size() + 1;

            dirty_ = true;
            return true;
        }

        return false;
    }

    size_t A = a;

    size_t B = (b == -1) ? N : b;
    if (B < A)
        return false;

    // clamp end to N
    if (A > N)
        return false;

    if (B > N)
        B = N;

    lines_.erase(lines_.begin() + (A - 1), lines_.begin() + B);

    // if we erased all the lines
    if (lines_.empty())
        currentLine_ = 1;

    // if we erased the index we were on > 1
    if (currentLine_ > lines_.size())
        currentLine_ = lines_.size() + 1;

    dirty_ = true;
    return true;
}

/**
 * @brief Main function to print lines
 *
 * @param a The line index to print
 * @post Selected line will be printed to console
 */
void LineEdit::printLine(size_t a) const
{
    if (a < lines_.size())
    {
        size_t indent = a < 9 ? 3 : 2;
        a == (currentLine_ - 1) ? std::cout << std::string((indent - 1), ' ') << '*' << a + 1 << "| " << lines_.at(a) << '\n' : std::cout << std::string(indent, ' ') << a + 1 << "| " << lines_.at(a) << '\n';
    }
}

/**
 * @brief Print selected lines based on user input or all lines w/o parameters
 *
 * @param a Optional value to select a single line to print / the first index in the range to print
 * @param b Optional value used to select last line in range to print
 * @return Boolean indicating success or failure
 * @post Selected line(s) from a to b will be printed
 * @sa printLine()
 */
bool LineEdit::printSelected(long long a, long long b)
{
    const size_t N = lines_.size();
    if (N == 0)
    {
        std::cout << kSep << "(empty)\n"
                  << kSep;
        return true;
    }

    if (a == 0 && b == 0)
    {
        std::cout << kSep;
        for (size_t i = 0; i < N; i++)
        {
            printLine(i);
        }
        std::cout << kSep;

        return true;
    }

    // One-arg mode: a>0 prints that line; a==-1 prints last
    if (b == 0)
    {
        if (a == -1)
        {
            std::cout << kSep;
            printLine(static_cast<int>(N - 1));
            std::cout << kSep;

            return true;
        }

        if (a >= 1 && a <= (long long)N)
        {
            std::cout << kSep;
            printLine(a - 1);
            std::cout << kSep;

            return true;
        }

        return false;
    }

    // // Two-arg mode: a is start (must be >=1), b is end (may be -1)
    // if (a == 0)
    //     return false; // -1 as start is invalid
    size_t A = a;

    size_t B = (b == -1) ? N : b;
    if (B < A)
        return false;

    // clamp end to N
    if (A > N)
        return false;

    if (B > N)
        B = N;

    std::cout << kSep;
    for (size_t i = A - 1; i <= B - 1; ++i)
    {
        printLine(i);
    }
    std::cout << kSep;

    return true;
}

/**
 * @brief Show a prompt on the console. Indicates current line index and clean/dirty file status
 */
void LineEdit::printPrompt(void)
{
    dirty_ ? std::cout << currentLine_ << '!' << "> " : std::cout << currentLine_ << "> ";

    prompt = true;
}

/**
 * @brief Main program loop
 */
void LineEdit::loop(void)
{
    std::cout << "\033[2J\033[1;1H"; // clear screen and reset cursor

    std::string path = filePath_.empty() ? "(new file)" : filePath_.string();
    std::cout << art << "\n"
              << "Created by Josh Huntley\n\n"
              << "Editing: "
              << path << "\n------------\n";

    for (std::string line;;)
    {
        printPrompt();

        if (!std::getline(std::cin, line))
            break;

        auto p = parse_cmd(line);

        if (!p.ok)
        {
            std::cout << "[invalid command]\n";
            menu();
            continue;
        }

        switch (p.cmd)
        {
        case 'I':
        { // Insert at current
            // p.rest is the line to insert; may be ""
            insert(p.rest);

            break;
        }
        case 'A':
        { // Insert after current
            if (!append(p.rest))
            {
                std::cout << "[invalid line]\n";
                menu();
            }

            break;
        }
        case 'W':
        { // Write file
            if (!p.rest.empty())
                filePath_ = p.rest;

            if (filePath_.empty())
                filePath_ = getPath();

            if (!filePath_.empty())
            {
                saveFile(filePath_);
            }

            break;
        }
        case 'J':
        { // Jump to line
            if (p.rest.empty())
            {
                std::cout << "[enter line number]\n";
                menu();
                break;
            }

            long long n = 0;
            std::istringstream iss(p.rest);
            iss >> n;

            if (!jump(n))
            {
                std::cout << "[invalid line]\n";
                menu();
            }

            break;
        }
        case 'L':
        { // Print one or range of lines
            long long a{0}, b{0};
            std::istringstream iss(p.rest);

            iss >> a >> b;

            if (!printSelected(a, b))
            {
                std::cout << "[invalid line]\n";
                menu();
            }

            break;
        }
        case 'D':
        { // Delete one or range of lines
            long long a{0}, b{0};
            std::istringstream iss(p.rest);

            iss >> a >> b;

            if (!remove(a, b))
            {
                std::cout << "[invalid line]\n";
                menu();
            }

            break;
        }
        case 'C':
        {
            std::cout << "\033[2J\033[1;1H";

            break;
        }
        case 'Q':
        {
            if (dirty_)
            {
                std::cout << "File modified. Save changes? [y/N] Cancel [c] ";
                std::string ans;
                std::getline(std::cin, ans);

                if (!ans.empty() && (ans[0] == 'y' || ans[0] == 'Y'))
                {
                    if (filePath_.empty())
                        filePath_ = getPath();

                    if (!filePath_.empty())
                    {
                        saveFile(filePath_);
                    }
                    else
                    {
                        std::cout << "Canceled\n";
                        continue;
                    }
                }
                else if (!ans.empty() && (ans[0] != 'n' || ans[0] != 'N'))
                {
                    std::cout << "Cancelled.\n";
                    continue;
                }
            }
            return; // exit loop
        }
        case 'H':
        default:
            // show help
            menu();
            break;
        }
    }
}