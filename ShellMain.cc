#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <string>
#include <ostream>

#include "Debug.hh"

#include "UiSecAclDbRecordCacheApiStub.hh"

struct Foo
{
  int a;
  int b;

  Foo() { a = 0; b = 0; }
  Foo(int a, int b) { this->a = a; this->b = b; }

  bool operator < (Foo const &o) const
  {
    return (a < o.a);
  }

};

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    struct termios term;
    bool scriptmode = (tcgetattr(0, &term) < 0);

    CPPUTILS::Debug debug;

    Db::RecCacheApi_t<Foo> dbApi;

    Foo f1(1,10);
    Foo f2(2,20);
    Foo f3(3,30);
    Foo f4(0,0);
    dbApi.add(f1);
    dbApi.add(f2);
    dbApi.add(f3);
    dbApi.getNext(f4);
    printf("next is %d\n", f4.a);
    dbApi.getNext(f4);
    printf("next is %d\n", f4.a);
    dbApi.getNext(f4);
    printf("next is %d\n", f4.a);
    dbApi.getNext(f4);
    printf("next is %d\n", f4.a);
    dbApi.modify(Foo(1,11));
    Foo f5(1,0);
    dbApi.get(f5);
    printf("f5 val is %d\n", f5.b);
    dbApi.remove(f5);
    dbApi.getNext(f5);
    printf("f5 next is %d\n", f5.a);
    std::list<Foo> all;
    dbApi.getAll(all);
    for (std::list<Foo>::iterator it = all.begin(); it != all.end(); ++it)
    {
        printf("all %d\n", it->a);
    }
    for (Db::RecCacheApi_t<Foo>::recCacheIter_t it = dbApi.begin(); it != dbApi.end(); ++it)
    {
        printf("iter %d\n", it->a);
    }
    return 0;

    using_history();

    while (1)
    {
        std::string prompt = "-> ";

        char *input = readline(scriptmode ? NULL : prompt.c_str());
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
