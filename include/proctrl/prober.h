//
// Created by antq on 7/24/16.
//

#ifndef PROCTRL_PROBER_H
#define PROCTRL_PROBER_H

#include <string>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

namespace proctrl {
    class Prober
    {
    public:
        virtual ~Prober () {}

        /* return the name of the index this prober supports */
        virtual string name (void)=0;

        /* retrieve the resource utilization of a particular
         * process, or the entire system.
         * @param pid: pid of the process to probe, or zero if
         * the system-wide utilization is wanted;
         * @return: the utilization probed, or zero if pid is bad. */
        virtual double probe (pid_t pid=0)=0;

        /* delete the state related to the specified process.
         * if the prober doesn't maintain any state, this
         * function shall be left as it is. */
        virtual void clean (pid_t pid)
        {/* dummy */}
    };
}

#endif //PROCTRL_PROBER_H
