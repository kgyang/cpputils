#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <string>
#include <ostream>

#include "Debug.hh"


int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    struct termios term;
    bool scriptmode = (tcgetattr(0, &term) < 0);

    CPPUTILS::Debug debug;

    while (1)
    {
        std::string prompt = scriptmode ? "" : "-> ";

        char *input = readline(prompt.c_str());
        if (!input) break;

        if (*input && !scriptmode) add_history(input);
        std::istringstream line(input);
        free(input);

        std::vector<std::string> args;
        std::string arg;
        while (line >> arg) args.push_back(arg);   

        if (args.empty()) continue;

        if (args[0] == "exit")
        {
            break;
        }

        std::ostringstream oss;
        oss.str("");
       
        debug.runCmd(oss, args);
        std::cout << oss.str();
        
    }

    return 0;
}
