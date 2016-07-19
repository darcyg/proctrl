//
// Created by antq on 7/18/16.
//

#include "gtest/gtest.h"

#include <pwd.h>
#include <proctrl/process.h>
using namespace proctrl;

class ProcessTest : public ::testing::Test {
protected:
	virtual void TearDown() {
	}

	virtual void SetUp() {
	}

};
const char* getCurrentUserName() {
	struct passwd *passwd;
	passwd = getpwuid ( getuid());
	return passwd->pw_name;
}
bool checkProcess(pid_t pid) {
	if(kill(pid, 0) == -1) {
		if(errno == ESRCH) {
			return false;
		}
	}
	return true;
}

TEST(ProcessTest, control) {
	/* prepare the script file */
	string scriptName = "test.sh";
	string workDir = "/tmp";
	string scriptContent = "#!/bin/bash\nsleep 100";
	string user(getCurrentUserName());
	string scriptFile = workDir+"/"+ scriptName;
	ofstream ofScriptFile (scriptFile.c_str());
	ofScriptFile << scriptContent;
	ofScriptFile.close();
	chmod (scriptFile.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);

	ProcessParams pp(scriptName, user, workDir, scriptName);
	Process proc(pp);
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

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

