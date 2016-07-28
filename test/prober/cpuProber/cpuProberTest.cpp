//
// Created by antq on 7/24/16.
//
#include "gtest/gtest.h"
#include <pwd.h>
#include <proctrl/process.h>
#include <proctrl/prober.h>
#include <proctrl/cpuProber.h>
#include "cpulimit.h"
using namespace proctrl;

string randStrGenerator() {
    string numAlpha = "01234567abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int length = (int)numAlpha.length();
    srand(time(NULL));
    string result = "";
    for(int i = 0; i < 6; i++) {
        result += numAlpha[rand() % length];
    }
    return result;
}

class CPUProberTest : public ::testing::Test {
protected:

    virtual void SetUp() {
        scriptName = randStrGenerator() + ".sh";
        workDir = ".";
        user = getCurrentUserName();
        string scriptFile = workDir+"/"+ scriptName;
        string scriptContent = "#!/bin/bash\ndd if=/dev/zero of=/dev/null";
        ofstream ofScriptFile (scriptFile.c_str());
        ofScriptFile << scriptContent;
        ofScriptFile.close();
        chmod (scriptFile.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
    }

    virtual void TearDown() {
        string scriptFile = workDir+"/"+ scriptName;
        remove(scriptFile.c_str());
    }
public:
    const char* getCurrentUserName() {
        struct passwd *passwd;
        passwd = getpwuid ( getuid());
        return passwd->pw_name;
    }
public:
    string scriptName;
    string workDir;
    string user;

};
static double limit = 20.0;
static void* _limitCPU(void* arg) {
    pid_t pid = *((pid_t *)arg);
    limit_process(pid, limit / 100.0, 1);
}

/* Get the number of CPUs */
static int get_ncpu() {
    int ncpu;
#ifdef _SC_NPROCESSORS_ONLN
    ncpu = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined __APPLE__
    int mib[2] = {CTL_HW, HW_NCPU};
	size_t len = sizeof(ncpu);
	sysctl(mib, 2, &ncpu, &len, NULL, 0);
#elif defined _GNU_SOURCE
	ncpu = get_nprocs();
#else
	ncpu = -1;
#endif
    return ncpu;
}

TEST_F(CPUProberTest, cpu) {
    ProcessParams pp(scriptName, user, workDir, scriptName);
    Process proc(pp);

    ASSERT_EQ(0, proc.start());

    ASSERT_FALSE(proc.exited());

    //limit the cpu usage
    int ncpu = get_ncpu();
    pid_t pid = proc.getPid();
    pthread_t limitId;
    ASSERT_TRUE(pthread_create (&limitId, NULL, ::_limitCPU, &pid) == 0);

    //wait a minute for limiting the cpu usage.
    sleep(1);

    //error range is +-5
    double offset = 5;
    Prober* prober = CPUProber::getProber();
    cout.precision(3);
    for(int i = 0; i < 10; i++) {
        double usage = prober->probe(pid) * ncpu;
        cout<<fixed<<usage<<endl;
        if(i > 2) {
            ASSERT_LE(usage, limit+offset);
            ASSERT_GE(usage, limit-offset);
        }
        sleep(1);
    }

    proc.kill();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

