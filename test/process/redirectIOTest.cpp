//
// Created by antq on 7/21/16.
//

#include "gtest/gtest.h"
#include <pwd.h>
#include <fcntl.h>
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

class RedirectIOTest : public ::testing::Test {
protected:
	virtual void TearDown() {
	}

	virtual void SetUp() {
	}
public:
	RedirectIOTest(): Test() {
		workDir = "/tmp";
		user = getCurrentUserName();
	}
	void generateScript(string scriptName, string scriptContent) {
		string scriptFile = workDir+"/"+ scriptName;
		ofstream ofScriptFile (scriptFile.c_str());
		ofScriptFile << scriptContent;
		ofScriptFile.close();
		chmod (scriptFile.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);
	}
protected:
	const char* getCurrentUserName() {
		struct passwd *passwd;
		passwd = getpwuid ( getuid());
		return passwd->pw_name;
	}

	int getPipeSize() {
		int ipipe[2];
		if(pipe(ipipe) < 0) {
			return 0;
		}
		int result = fcntl(ipipe[1], F_GETPIPE_SZ);
		close(ipipe[0]);
		close(ipipe[1]);
		return result;
	}
protected:
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
 * Test flow: input, get stdout and stderr
 */
TEST_F(RedirectIOTest, basic) {
	string scriptName = randStrGenerator();
	string scriptContent = "#!/bin/bash\n"
			"read end\n"
			"(>&2 echo `echo $end | awk '{print $1+1}'`)\n"
			"for i in `seq 1 ${end}`\n"
			"do\n"
			"echo $i\n"
			"done";
	this->generateScript(scriptName, scriptContent);

	ProcessParams pp(scriptName, this->user, this->workDir, scriptName);
	pp.redirectIO = true;
	Process proc(pp);

	int number = 5;
	ASSERT_EQ(0, proc.start());
	string inputStr = to_string(number) + "\n";
	proc.input((byte*)inputStr.c_str(), inputStr.length());

	//check stdout
	char buf[256];
	string expectStr = "";
	string result;
	for(int i = 1; i <= number; i++) {
		expectStr += to_string(i) + "\n";
		memset(buf, 0, sizeof(buf));
		ssize_t ret = proc.getOutput((byte*)buf, sizeof(buf));
		if(ret > 0) {
			result += string(buf);
		} else {
			sleep(1);
		}
	}
	ASSERT_TRUE(expectStr.compare(result) == 0);
	//check stderr
	memset(buf, 0, sizeof(buf));
	ssize_t ret = proc.getErrOutput((byte*)buf, sizeof(buf));
	ASSERT_GT(ret, 0);
	ASSERT_EQ(number+1, atoi(string(buf).c_str()));

	//kill the process
	proc.kill();
	//wait some time for making sure parent received signal.
	while(!proc.exited()) {
		usleep(100);
	}
	ASSERT_TRUE(proc.exited());
	ASSERT_FALSE(checkProcess(proc.getPid()));
	//clear files
	ASSERT_EQ(0, remove(string(this->workDir + "/" + scriptName).c_str()));
}

/*
 * Test flow: input to child process without reader
 */
TEST_F(RedirectIOTest, inputToNoReader) {
	string scriptName = randStrGenerator();
	string scriptContent = "#!/bin/bash\n"
			"sleep 1";
	this->generateScript(scriptName, scriptContent);

	ProcessParams pp(scriptName, this->user, this->workDir, scriptName);
	pp.redirectIO = true;
	Process proc(pp);
	ASSERT_EQ(0, proc.start());

	int pipeSize = getPipeSize();
	char data[pipeSize+1];
	memset(data, 0, sizeof(data));
	ssize_t ret = proc.input((byte*)data, sizeof(data));

	ASSERT_EQ(pipeSize, ret);
	ret = proc.input((byte*)data, sizeof(data));
	ASSERT_EQ(-1, ret);
	ASSERT_EQ(EAGAIN, errno);
	//kill the process
	proc.kill();
	//wait some time for making sure parent received signal.
	while(!proc.exited()) {
		usleep(100);
	}
	ASSERT_TRUE(proc.exited());
	ASSERT_FALSE(checkProcess(proc.getPid()));
	//clear files
	ASSERT_EQ(0, remove(string(this->workDir + "/" + scriptName).c_str()));
}

/*
 * Test flow: input to a stopped child process
 */
TEST_F(RedirectIOTest, inputToStoppedProc) {
	string scriptName = randStrGenerator();
	string scriptContent = "#!/bin/bash\n"
			"read num\n"
			"echo ${num}";
	this->generateScript(scriptName, scriptContent);

	ProcessParams pp(scriptName, this->user, this->workDir, scriptName);
	pp.redirectIO = true;
	Process proc(pp);
	ASSERT_EQ(0, proc.start());

	//stop the process
	proc.stop();

	string expectStr = "10\n";

	char data[4] = "10\n";
	ssize_t ret = proc.input((byte*)data, sizeof(data));
	ASSERT_GT(ret, 0);

	int tryTimes = 2;
	char buf[256];
	string result = "";
	for(int i = 0; i < tryTimes; i++) {
		memset(buf, 0, sizeof(buf));
		ret = proc.getOutput((byte*)buf, sizeof(buf));
		if(ret > 0) {
			result += string(buf);
		} else {
			sleep(1);
		}
	}
	ASSERT_TRUE(result.empty());

	//continue the process
	proc.continuing();
	for(int i = 0; i < tryTimes; i++) {
		memset(buf, 0, sizeof(buf));
		ret = proc.getOutput((byte*)buf, sizeof(buf));
		if(ret > 0) {
			result += string(buf);
		} else {
			sleep(1);
		}
	}
	ASSERT_TRUE(expectStr.compare(result) == 0);

	//kill the process
	proc.kill();
	//wait some time for making sure parent received signal.
	while(!proc.exited()) {
		usleep(100);
	}
	ASSERT_TRUE(proc.exited());
	ASSERT_FALSE(checkProcess(proc.getPid()));
	//clear files
	ASSERT_EQ(0, remove(string(this->workDir + "/" + scriptName).c_str()));
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

