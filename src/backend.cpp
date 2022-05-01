#include "backend.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <dirent.h>

char* string_to_char(std::string inp) {
	// Function to turn an std::string to a char*
	char* charOut = new char[inp.length() + 1];
	strcpy(charOut, inp.c_str());
	return charOut;
}

Backend::Backend(QObject *parent)
	: QObject(parent)
{
	buildFind(0); // Instantly check for builds on load. Probably not efficient
}

int Backend::buildCountValue() {
	return buildCount;
}

int Backend::buildSelectedValue() {
	return buildSelected;
}

QString Backend::buildList(int pos) {
	return builds[pos];
}

QString Backend::repoText()
{
	return repo;
}

QString Backend::branchText()
{
	return branch;
}

bool Backend::downloadSizeUnknownValue() {
	return downloadSizeUnknown;
}

void Backend::buildFind(int additive) {
	int count = 0 + additive;
	int i = 0;
	DIR *dir;
	struct dirent *dirEntry;
	builds->clear();
	if ((dir = opendir ("sm64-builds")) != NULL) {
		while ((dirEntry = readdir (dir)) != NULL) {
			if (dirEntry->d_type == DT_DIR && dirEntry->d_name[0] != '.') {
				builds[i] = dirEntry->d_name;
				count++;
				i++;
				printf("Directory found: %s\n", dirEntry->d_name);
			}
		}
		closedir (dir);
	} else {
		perror ("");
	}
	buildCount = count;
	Q_EMIT buildCountModified();
	Q_EMIT buildListModified();
}

void Backend::setBuildSelected(int target) {
	buildSelected = target;
	Q_EMIT buildSelectModified();
}

void Backend::buildListSet(QString &folder) {
	// This does nothing. It just emits buildListModified().
	Q_EMIT buildListModified();
}

void Backend::setRepo(QString &repoInp)
{
	repo = repoInp;
	Q_EMIT repoModified();
}

void Backend::setBranch(QString &branchInp)
{
	branch = branchInp;
	Q_EMIT branchModified();
}

void Backend::setDownloadSizeUnknownStatus(bool &known)
{
	downloadSizeUnknown = known;
	Q_EMIT downloadSizeUnknownStatus();
}

int Backend::clone() {
	if (!opendir("sm64-builds/")) {
		mkdir("sm64-builds",0777);
	}
	//std::string stdFolder = "sm64-builds/";
	std::string command = "git clone --branch " + branch.toStdString() + " " + repo.toStdString() + " --progress > .out";
	/*const char *dir = (stdFolder).c_str();
	mkdir(dir,0777);*/
	std::string cmd = "cd sm64-builds && " + command + " &";
	system(string_to_char(cmd));
	buildFind(0);
	return 0;
}

int Backend::pull(QString folder) {
	std::string cmdAsString = "cd sm64-builds/" + folder.toStdString() + " && git pull &";
	//printf("%s\n", string_to_char(folder.toStdString())); // Test to see if it worked.
	system(string_to_char(cmdAsString));
	return 0;
}

int Backend::build(QString folder) {
	std::string cmd0 = "cp baserom." + region + ".z64 sm64-builds/" + folder.toStdString() + "/baserom.us.z64";
	std::string cmd1 = "cd sm64-builds/" + folder.toStdString() + " && make -j4 &";
	std::string fullCmd = cmd0 + " && " + cmd1;
	system(string_to_char(fullCmd));
	return 0;
}

int Backend::run(QString folder) {
	char* dir = string_to_char("sm64-builds/" + folder.toStdString() + "/build/");
	if (!opendir(dir)) {
		build(folder); // The repository hasn't run make yet.
	}
	std::string cmdAsString = "cd sm64-builds/" + folder.toStdString() + "/build/" + region + "_pc/ && ./sm64." + region + ".f3dex2e &";
	system(string_to_char(cmdAsString));
	return 0;
}

int Backend::rmDir(QString folder) {
	char* cmd = string_to_char("rm -rf sm64-builds/" + folder.toStdString());
	system(cmd);
	buildFind(0);
	return 0;
}