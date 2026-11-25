#include <iostream>
#include <string>

int main()
{
    std::string text = "SEND\nanna\nHello\n";
    std::string word;

    for (char c : text) {
        if (c == '\n') {
            std::cout << word <<"\n";
            word.clear();
        } else {
            word += c;
        }
    }
    return 0;
}