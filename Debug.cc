#include <stdlib.h>
#include <map>
#include "Log.hh"
#include "Debug.hh"
#include "Trace.hh"

namespace CPPUTILS
{

Debug::Debug()
{
    cmds_["help"] = cmds_["?"] = cmds_["h"] = CmdItem(&Debug::help,
        "help [cmd]");

    cmds_["log"] = CmdItem(&Debug::log,
        "show [sys|app]\n"
        "clr <sys|app>\n"
        "max <sys|app> <val>");

    cmds_["trace"] = CmdItem(&Debug::trace,
        "show\n"
        "set <none|error|warming|debug|detail>");
}

int Debug::runCmd(std::ostringstream &oss, std::vector<std::string> &argv)
{
    for (std::vector<std::string>::iterator iter = argv.begin();
          iter != argv.end(); ++iter)
    {
        std::string& s = *iter;
        for (std::string::iterator p = s.begin(); p != s.end(); ++p)
        {
            *p = std::tolower(*p, std::locale());
        }
    }

    if (argv.empty()) return -1;
    std::string cmd = argv[0];
    argv.erase(argv.begin());
    
    std::map<std::string, CmdItem>::iterator iter = cmds_.find(cmd);
    
    if (iter == cmds_.end())
    {
        return -1;
    }
    else
    {
        Func func = iter->second.func;
        if ((this->*func)(oss, argv) != 0)
        {
            // cmd parameter parse failure
            oss << iter->second.usage << std::endl;
        }
    }

    return 0;
}

int Debug::help(std::ostringstream &oss, std::vector<std::string> &argv)
{
  if (argv.empty())
  {
    for (std::map<std::string, CmdItem>::iterator iter = cmds_.begin();
         iter != cmds_.end(); ++iter)
    {
      oss << iter->first << std::endl;
    }
  }
  else
  {
    std::map<std::string, CmdItem>::iterator iter = cmds_.find(argv[0]);
    if (iter != cmds_.end())
    {
        oss << iter->second.usage << std::endl;
    }
    else
    {
        oss << "unknown command" << std::endl;
    }
  }
  return 0;
}

int Debug::log(std::ostringstream &oss, std::vector<std::string> &argv)
{
    if (argv.size() < 1) return -1;

    std::string cmd = argv[0];
    argv.erase(argv.begin());

    if (cmd == "show")
    {
        if (argv.size() == 0)
        {
            oss << "SYS LOG: "
                << Log::sysLog.getLogNumber()
                << "(" << Log::sysLog.getMaxLogNumber() << ")"
                << std::endl;
            oss << "APP LOG: "
                << Log::appLog.getLogNumber()
                << "(" << Log::appLog.getMaxLogNumber() << ")"
                << std::endl;
        }
        else if (argv.size() == 1)
        {
            std::string sub = argv[0];
            if (sub != "sys" && sub != "app") return -1;
            Log& log = (sub == "sys") ? Log::sysLog : Log::appLog;
            oss << log.read(32768) << std::endl;
        }
        else
        {
            return -1;
        }
    }
    else if (cmd == "clr")
    {
        if (argv.size() != 1) return -1;
        std::string sub = argv[0];
        if (sub != "sys" && sub != "app") return -1;
        Log& log = (sub == "sys") ? Log::sysLog : Log::appLog;
        log.clear();
    }
    else if (cmd == "max")
    {
        if (argv.size() != 2) return -1;
        std::string sub = argv[0];
        if (sub != "sys" && sub != "app") return -1;
        Log& log = (sub == "sys") ? Log::sysLog : Log::appLog;
        log.setMaxLogNumber(atoi(argv[1].c_str()));
    }
    else
    {
        return -1;
    }
    return 0;
}

int Debug::trace(std::ostringstream &oss, std::vector<std::string> &argv)
{
    std::map<TraceLevel, std::string> levels;
    levels[TRACE_LEVEL_NONE] = "none";
    levels[TRACE_LEVEL_ERROR] = "error";
    levels[TRACE_LEVEL_WARNING] = "warning";
    levels[TRACE_LEVEL_DEBUG] = "debug";
    levels[TRACE_LEVEL_DETAIL] = "detail";

    if (argv.size() < 1) return -1;

    std::string cmd = argv[0];
    argv.erase(argv.begin());

    if (cmd == "show")
    {
        oss << "Trace level: " << levels[trace_getLevel()]<< std::endl;
    }
    else if (cmd == "set")
    {
        if (argv.size() != 1) return -1;
        for (std::map<TraceLevel, std::string>::iterator iter = levels.begin();
             iter != levels.end(); ++iter)
        {
            if (iter->second == argv[0])
            {
                trace_setLevel(iter->first);
                return 0;
            }
        }
        return -1;
    }
    else
    {
        return -1;
    }
    return 0;
}

}
