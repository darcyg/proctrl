//
// Created by edward on 2016/7/17.
//



#include <strings.h>
#include <string.h>
#include <signal.h>	/* signal */
#include <sys/stat.h>
#include <pwd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#include <sstream>
#include <boost/algorithm/string.hpp>

#include <proctrl/process.h>

using namespace std;

namespace proctrl {

    Process::Process(const ProcessParams& pp):
        params(pp),
        state(Initial),
        pid(0),
        exitCode(0){
        redPipe[0] = redPipe[1] = redPipe[2] = -1;
    }

    Process::~Process() {
        if(!this->exited()) {
            this->kill();
        }

        close(redPipe[0]);
        close(redPipe[1]);
        close(redPipe[2]);

    }

    int Process::start() {
        //judge whether the process has been started and not exited
        if(state != Initial && !Process::exited()) {
            return -1;
        }
        /* get the uid of the run-as user */
        uid_t uid = 0;
        struct passwd * pwe = getpwnam (params.user.c_str());
        if (pwe != NULL) {
            uid = pwe->pw_uid;
        } else {
            return -2;
        }

        string scriptFile = params.workDir + "/" + params.scriptFile;

        //check the script file exist and executable with the run-as user
        struct stat st;
        if(stat(scriptFile.c_str(), &st) == 0) {
            if(!(st.st_mode & S_IXUSR) || st.st_uid != uid) {
                return -3;
            }
        } else {
            return -4;
        }

        /* prepare the redirection channels */
        int ipipe [2] = {-1,-1};
        int opipe [2] = {-1,-1};
        int epipe [2] = {-1,-1};
        if (params.redirectIO) {
            if (pipe (ipipe) == -1) {
                return 0;	/* do not fail the job, just try later */
            }
            if (pipe (opipe) == -1) {
                close (ipipe [0]);
                close (ipipe [1]);
            }
            if (pipe (epipe) == -1) {
                close (opipe [0]);
                close (opipe [1]);
                close (ipipe [0]);
                close (ipipe [1]);
            }
        }

        //-------------- fork it! -----------------
        pid_t pid;
        if ((pid = fork ()) == -1) {
            close (ipipe [0]);
            close (ipipe [1]);
            close (opipe [0]);
            close (opipe [1]);
            close (epipe [0]);
            close (epipe [1]);
        }

        if (pid == 0) {

            /* drop the root privilege */
            setuid (uid);

            /* create a new session.
             * when canceling a job, we need the signals to be
             * delivered to all children of the job */
            setsid ();

            /* redirect stdin/stdout */
            if (params.redirectIO) {
                if (dup2 (ipipe [0],0)<0 ||
                    dup2 (opipe [1],1)<0 ||
                    dup2 (epipe [1],2)<0) {
                    exit(errno);
                }
                close (ipipe [0]);
                close (ipipe [1]);
                close (opipe [0]);
                close (opipe [1]);
                close (epipe [0]);
                close (epipe [1]);
            }

            /* prepare the arguments/environments */

            vector <string> args;
            boost::split (args, params.args, boost::is_any_of(" "));
            unsigned long argc = args.size()+2;
            char ** argv = (char **) malloc (sizeof (char*)*argc);
            if (argv == NULL) exit (ENOMEM);

            argv [0] = strdup(scriptFile.c_str());
            argv [argc-1] = NULL;
            for (int i=0;i<argc-2;i++)
                argv [i+1] = strdup (args[i].c_str());

            for(auto p : params.env) {
                setenv(p.first.c_str(), p.second.c_str(), 1);
            }

            /* execute it! ------> */
            execv (scriptFile.c_str(), argv);
            exit (errno);
        }

        /* redirect stdin/stdout */
        if (params.redirectIO) {
            close (ipipe [0]);
            close (opipe [1]);
            close (epipe [1]);
            redPipe [0] = ipipe [1];
            redPipe [1] = opipe [0];
            redPipe [2] = epipe [0];
            //make the input fd non-block
            fcntl(redPipe [0], F_SETFL, fcntl(redPipe [0], F_GETFL) | O_NONBLOCK);
        }
        this->pid = pid;
        this->state = Active;
        this->exitCode = 0;
        //wait a moment to wait child process.
        sleep(1);
        return 0;
    }
    bool Process::exited() {
        if(this->state == Active || this->state == Stopped) {
            updateState();
        }
        return (this->state == Finished || this->state == Canceled
                || this->state == Failed);
    }
    void Process::wait() {
        updateState(0);
    }

    void Process::updateState(int options) {
        if(this->state == Finished || this->state == Canceled) {
            return;
        }
        int status;
        //get latest signal of the pid.
        pid_t retPid = waitpid(this->pid, &status, options);
        if(retPid < 0) {
            this->exitCode = 0;
            this->state = Finished;
        } else if(retPid == this->pid) {
            if(WIFEXITED(status)) {
                this->exitCode = WEXITSTATUS(status);
                this->state = Finished;
            } else if(WIFSIGNALED(status)) {
                this->state = Canceled;
            } else if(WIFSTOPPED(status)) {
                int sig = WSTOPSIG(status);
                if(sig == SIGSTOP) {
                    this->state = Stopped;
                }
            } else if(WIFCONTINUED(status)) {
                this->state = Active;
            }
        }
    }


    //control the process
    /*
     * Stop all process in the process group
     */
    void Process::stop() {
        updateState();
        if(this->state == Active) {
            /* a negative pid forces the signal to be delivered
             * to all processes in the process group
             */
            ::kill (-pid, SIGSTOP);
        }
    }
    /*
     * Continue all process in the process group
     */
    void Process::continuing() {
        updateState();
        if(this->state == Stopped) {
            /* a negative pid forces the signal to be delivered
             * to all processes in the process group
             */
            ::kill (-pid, SIGCONT);
        }
    }
    void Process::kill() {
        updateState();
        if(this->state == Active || this->state == Stopped) {
            ::kill (-pid, SIGKILL);
            /* close pipes */
            close (redPipe [0]);
            close (redPipe [1]);
            redPipe[0] = redPipe[1] = redPipe[2] = -1;
        }
    }

    pid_t Process::getPid() {
        return this->pid;
    }

    ssize_t Process::input(byte* data, size_t length) {
        /* Ps: if the job has frozen, the write operation may block forever.
         *      we should use a non-blocking write here. */
        return write (redPipe [0], data, length);
    }
    ssize_t Process::readOutput(int& fd, byte* data, size_t length) {
        fd_set fs;
        FD_ZERO (&fs);
        if (fd >= 0)
            FD_SET (fd, &fs);
        else
            return 0;

        struct timeval to = {1, 0};
        int rv = select (fd+1, &fs, NULL, NULL, &to);
        if (rv < 0 || rv == 0)
            return 0;

        ssize_t result = read (fd, data, length);

        return result;
    }
    ssize_t Process::getOutput(byte* data, size_t length) {
        return readOutput(redPipe[1], data, length);
    }
    ssize_t Process::getErrOutput(byte* data, size_t length) {
        return readOutput(redPipe[2], data, length);
    }

}