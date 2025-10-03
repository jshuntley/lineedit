/************************************************************************
 * @Author : Josh Huntley
 * @Class : CptS 223; Fall 2025
 * @Assignment : Linear Structures Programming Assignment
 * @Date : Sep 24, 2025
 * @Description : Implement a line editor
 * @Compile : "make" or g++ main.cpp lineEdit.cpp -o lineedit
 * @Run : ./lineedit <file>
 *************************************************************************/

#include "lineEdit.hpp"

int main(int argc, char *argv[])
{
    LineEdit *lineEdit;

    // see if user entered a filename. if not, prompt for file to edit
    if (argc <= 1 || argv[1] == nullptr || std::string(argv[1]).empty())
    {
        lineEdit = new LineEdit();
    }
    else
    {
        lineEdit = new LineEdit(argv[1]);
    }

    lineEdit->loop();

    return 0;
}
