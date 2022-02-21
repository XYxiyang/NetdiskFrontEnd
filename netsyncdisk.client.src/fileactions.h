#ifndef FILEACTIONS_H
#define FILEACTIONS_H

#endif // FILEACTIONS_H
#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<winsock2.h>
#include<Windows.h>
#include<cstdlib>
#include<thread>
#include<time.h>
#include<string>
#include<fstream>
#include<iomanip>
#include<sstream>
#include<cstring>
#include<direct.h>
#include <ImageHlp.h>
#define CHECKMD5 1
#define SHOWTREE 0
using namespace std;
const string file1 = "C:\\mycloud\\";
const string file2 = "\\usrconfig.conf";

const string path1 = "C:\\mycloud\\";
const string path2 = "\\";

void startMonitor(string file);
string openFile(string filename,int length,int opt);
void receiveFiles();
string makesureConfigexist();
void createFoldersbyFile(string filename,string dir);
boolean addDir(string filename,string dirname,string clientusrname);
boolean bondDir(string filename,string dir1,string dir2);
boolean deleteDir(string filename, string dirname,string clientusrname);
boolean unbondDir(string filename, string dirname);
void writeLog(string file,string operation,string status,string content);
void mysleep(int period);
