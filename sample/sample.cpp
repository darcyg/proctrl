//
// Created by antq on 7/19/16.
//

#include <proctrl/process.h>
#include <pwd.h>
#include <sys/stat.h>
using  namespace proctrl;

const char* getCurrentUserName() {
	struct passwd *passwd;
	passwd = getpwuid ( getuid());
	return passwd->pw_name;
}
int main() {
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
	if(0 == proc.start()) {
		cout<<proc.getPid()<<" started"<<endl;
	} else {
		return -1;
	}

	cout<<"before kill: "<<proc.exited()<<endl;

	proc.kill();

	sleep(1);
	cout<<"after kill: "<<proc.exited()<<endl;

	return 0;
}
