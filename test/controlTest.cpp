//
// Created by antq on 7/18/16.
//

#include "gtest/gtest.h"
#include <pwd.h>
#include <proctrl/process.h>
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

class ControlTest : public ::testing::Test {
protected:
	virtual void TearDown() {
	}

	virtual void SetUp() {
	}
public:
	ControlTest(): Test() {
		scriptName = "test.sh";
		workDir = "/tmp";
		user = getCurrentUserName();
		string scriptFile = workDir+"/"+ scriptName;
		string scriptContent = "#!/bin/bash\nfor i in `seq 1 100`\ndo\necho $i\nsleep 1\ndone";
		ofstream ofScriptFile (scriptFile.c_str());
		ofScriptFile << scriptContent;
		ofScriptFile.close();
		chmod (scriptFile.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
	}
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

bool checkProcess(pid_t pid) {
	if(kill(pid, 0) == -1) {
		if(errno == ESRCH) {
			return false;
		}
	}
	return true;
}

/*
 * Test flow: start->exited?->kill->exited?->start
 */
TEST_F(ControlTest, startKillStart) {
	ProcessParams pp(scriptName, user, workDir, scriptName);
	Process proc(pp);
	for(int i = 0; i < 3; i++) {
		ASSERT_EQ(0, proc.start());

		ASSERT_TRUE(checkProcess(proc.getPid()));
		ASSERT_FALSE(proc.exited());

		proc.kill();
		//wait some time for making sure parent received signal.
		while(!proc.exited()) {
			usleep(100);
		}
		ASSERT_TRUE(proc.exited());
		ASSERT_FALSE(checkProcess(proc.getPid()));
	}
}

/*
 * Test flow: start->exited?->start
 */
TEST_F(ControlTest, startStart) {
	ProcessParams pp(scriptName, user, workDir, scriptName);
	Process proc(pp);

	ASSERT_EQ(0, proc.start());

	ASSERT_TRUE(checkProcess(proc.getPid()));
	ASSERT_FALSE(proc.exited());

	ASSERT_EQ(-1, proc.start());

	proc.kill();
}

/*
 * Test flow: start->stop->continuing
 */
TEST_F(ControlTest, startStopContinuing) {
	ProcessParams pp(scriptName, user, workDir, scriptName);
	Process proc(pp);

	ASSERT_EQ(0, proc.start());
	ASSERT_TRUE(checkProcess(proc.getPid()));
	ASSERT_FALSE(proc.exited());

	proc.stop();

	sleep(5);

	proc.continuing();

	sleep(5);
	proc.kill();
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

