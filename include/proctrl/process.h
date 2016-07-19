//
// Created by edward on 2016/7/16.
//

#ifndef PROCTRL_PROCESS_H
#define PROCTRL_PROCESS_H

#include <sys/wait.h>
#include <linux/if_ether.h>
#include <proctrl/common.h>
using namespace std;

namespace proctrl {
/*
 * parameters needed to run a job
 */
class ProcessParams
{
public:
    ProcessParams(const string& name, const string& user, const string& workDir,
                  const string& scriptFile, const string& args = "")
            :name(name),
            workDir(workDir),
            scriptFile(scriptFile),
            user(user),
            args(args),
            redirectIO(false){}

public:
    string name;

    string workDir;
    string scriptFile;
    string args;
    string user;	/* the effective user */

    bool redirectIO; //whether redirect IO or not

    boost::unordered_map<string, string> env;
};
/*
 * job states
 */
enum ProcessState
{
    Initial,	/* the Process is just created */
    Active,		/* the Process is currently running */
    Stopped,	/* the Process is stopped */
    Finished,	/* the Process has finished, without error */
    Failed,		/* the Process has failed, due to some error */
    Canceled,	/* the Process has been canceled */
};

/*
 * Ps.
 * 1. Can't control the new background process created by script.
 *    Because the control is based on parent-son relation.
 */
class Process {
public:
    Process(const ProcessParams& pp);

    //forbid copying the process
private:
    Process(const Process&);
    Process& operator=(const Process&) ;

public:
    int start();
    bool exited();
    void wait();

public:
    //control the process
    void stop();
    void continuing();
    void kill();

public:
    pid_t getPid();

private:
    void updateState(int options = (WNOHANG | WUNTRACED | WCONTINUED));

private:
    ProcessParams params;
    ProcessState state;
    pid_t pid;	/* pid of the job process */
    int exitCode;
    int redPipe [2];	/* pipes to redirect the job's input/output */
};

}
#endif //PROCESSCONTROLLER_PROCESS_H
