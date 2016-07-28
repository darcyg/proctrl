//
// Created by antq on 7/24/16.
//

#ifndef PROCTRL_UTIL_H
#define PROCTRL_UTIL_H

#include <string>
#include <vector>

using namespace std;

namespace proctrl {
    class Util
    {
    public:

        /* find recursively all child processes of pid */
        static void enumChildren (pid_t pid, vector <pid_t>& children);
    };
}
#endif //PROCTRL_UTIL_H
