//
// Created by antq on 7/25/16.
//

#ifndef PROCTRL_CPUPROBER_H
#define PROCTRL_CPUPROBER_H

#include <map>
#include <proctrl/prober.h>

namespace proctrl {
    /*
     * CPU Usage: Ratio of the cpu time of the process and his children
     *            and all CPU time.
     */
    class CPUState
    {
    public:
        CPUState (double _procTime=0.f, double _upTime=0.f) :
                procTime(_procTime),
                upTime(_upTime)
        {}
        double procTime;
        double upTime;
    };

// make sure the version of linux kernel >= 2.6.11
    class CPUProber : public Prober
    {
    public:
        static CPUProber* getProber() {
            static CPUProber _CPUProber;
            return &_CPUProber;
        }

    private:
        CPUProber(){}
        CPUProber(const CPUProber&){}
        CPUProber& operator=(const CPUProber&){}
    public:
        string name (void) { return "cpu"; }
        void clean (pid_t pid) { states.erase (pid); }
        double probe (pid_t pid=0);

    private:
        bool getSysState (CPUState& state);
        bool getProcState (pid_t pid, CPUState& state);
        map <pid_t, CPUState> states;
    };
}



#endif //PROCTRL_CPUPROBER_H
