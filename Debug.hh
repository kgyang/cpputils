#ifndef _DEBUG_HH_
#define _DEBUG_HH_

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>

namespace CPPUTILS
{
class Debug
{
    public:
        Debug();
        virtual ~Debug() {}
        int runCmd(std::ostringstream &oss, std::vector<std::string> &argv);

    private:
        typedef int(Debug::*Func)(std::ostringstream&, std::vector<std::string>&);
        struct CmdItem
        {
            Func func;
            const char* usage;
            CmdItem():func(NULL), usage(NULL) {}
            CmdItem(const Func& func, const char* usage):func(func), usage(usage) {}
        };
        int help(std::ostringstream &oss, std::vector<std::string> &argv);
        int log(std::ostringstream &oss, std::vector<std::string> &argv);

        std::map<std::string, CmdItem> cmds_;
};

}

#endif	// _DEBUG_HH_
