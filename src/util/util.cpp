//
// Created by antq on 7/24/16.
//

#include <stdio.h>
#include <sstream>
#include <proctrl/util.h>

namespace proctrl {
    void Util::enumChildren (pid_t pid, vector <pid_t>& children)
    {
        children.push_back (pid);

        char buf [256];
        sprintf (buf, "ps h --ppid %d -o pid", pid);
        FILE * fp = popen (buf, "r");
        if (fp != NULL) {
            stringstream ss;
            while (fgets (buf, 256, fp))
                ss << buf;
            while (ss >> pid)
                enumChildren (pid, children);
            pclose (fp);
        }
    }
}
