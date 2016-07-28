//
// Created by antq on 7/24/16.
//

#include <math.h>
#include <string.h>
#include <map>
#include <vector>
#include <proctrl/cpuProber.h>
#include <proctrl/util.h>

namespace proctrl {


/* implementation of CPUProber */

    double CPUProber::probe (pid_t pid)
    {
        CPUState& oldState = states [pid];
        CPUState nowState;

        /* get current state */

        if (pid == 0) {
            if (!getSysState (nowState))
                return 0.f;
        } else {
            /* read all children's busy time */
            vector <pid_t> cpid;
            Util::enumChildren (pid, cpid);
            for (unsigned i=0 ; i<cpid.size () ; i++)
                if (!getProcState (cpid [i], nowState))
                    return 0.f;

            /* read system time */
            CPUState state;
            if (!getSysState (state))
                return 0.f;
            nowState.upTime = state.upTime;
        }

        /* calculate the usage */

        double usage = (nowState.procTime-oldState.procTime)/(nowState.upTime-oldState.upTime)*100;
        oldState = nowState;	/* save the new state */
        usage = fmin (usage, 100);
        usage = fmax (usage, 0  );
        return usage;
    }

    bool CPUProber::getSysState (CPUState& state)
    {
        FILE * fp = fopen ("/proc/stat", "r");
        if (fp == NULL) return false;

        char buf [256];
        unsigned long user, nice, sys, idle, iowait, irq, softirq, stealstolen;
        while (fscanf(fp, "%4s %lu %lu %lu %lu %lu %lu %lu %lu %*[^/n]", buf,
                      &user, &nice, &sys, &idle,
                      &iowait, &irq, &softirq, &stealstolen) != EOF) {
            if (strcmp(buf, "cpu") == 0)
                break;
        }

        fclose (fp);

        state.upTime = user+nice+sys+idle+iowait+irq+softirq+stealstolen;
        return true;
    }

    bool CPUProber::getProcState (pid_t pid, CPUState& state)
    {
        char buf [256];
        sprintf (buf, "/proc/%d/stat", pid);
        FILE * fp = fopen (buf, "r");
        if (fp == NULL)
            return false;

        unsigned long utime, stime, cutime, cstime;
        int nread = fscanf (fp, "%*d %*s %*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d "
                                    "%lu %lu %lu %lu",
                            &utime, &stime, &cutime, &cstime);
        fclose (fp);
        if (nread != 4)
            return false;
        state.procTime += utime+stime+cutime+cstime;
        return true;
    }
}
