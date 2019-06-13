#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"
#include "util.h"

using namespace std;

class ProcessParser{
private:
    std::ifstream stream;
    public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getNumberOfCores();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid);
};

// TODO: Define all of the above functions below:
std::string ProcessParser::getVmSize(string pid) {
  std::string line;
  //Declaring search attribute for file
  std::string name = "VmData";
  float result;
  // Opening stream for specific file
  ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
  while(std::getline(stream, line)){
    // Searching line by line
    if (line.compare(0, name.size(),name) == 0) {
      // slicing string line on ws for values using sstream
      istringstream buf(line);
      istream_iterator<string> beg(buf), end;
      vector<string> values(beg, end);
      //conversion kB -> GB
      result = (stof(values[1])/float(1024*1024));
      break;
    }
  }
  return to_string(result);
}


std::string ProcessParser::getCpuPercent(string pid) {
  std::string line;
  std::string value; // is this used?
  ifstream stream = Util::getStream((Path::basePath() + pid + "/" + Path::statPath()));
  std::getline(stream, line);
  float utime = stof(ProcessParser::getProcUpTime(pid));
  std::string str = line;
  istringstream buf(str);
  istream_iterator<string> beg(buf), end;
  vector<string> values(beg, end);
  float stime = stof(values[14]);
  float cutime = stof(values[15]);
  float cstime = stof(values[16]);
  float startime = stof(values[21]);
  float uptime = ProcessParser::getSysUpTime();
  float freq = sysconf(_SC_CLK_TCK);
  float total_time = utime + stime + cutime + cstime;
  float seconds = uptime - (startime / freq);
  float result = 100 * ((total_time / freq) / seconds);

  return std::to_string(result);
}

long int ProcessParser::getSysUpTime(){
  std::string line;
  ifstream stream = Util::getStream((Path::basePath() + Path::upTimePath()));
  std::getline(stream, line);
  istringstream buf(line);
  istream_iterator<string> beg(buf), end;
  vector<string> values(beg, end);
  return stoi(values[0]);  
}


std::string ProcessParser::getProcUpTime(string pid){
  std::string line;
  ifstream stream = Util::getStream((Path::basePath() + pid + "/" +  Path::statPath()));
  std::getline(stream, line);
  istringstream buf(line);
  istream_iterator<string> beg(buf), end;
  vector<string> values(beg, end);
  return to_string(float(stof(values[13]) / sysconf(_SC_CLK_TCK)));
}

string ProcessParser::getProcUser(string pid) {
  std::string uid;
  std::string line;
  std::string name = "Uid:";
  std::string userName = "";
  // getting user id.
  ifstream stream = Util::getStream((Path::basePath() + pid +  "/" + Path::statusPath()));
  while(std::getline(stream, line)){
    if (line.compare(0, name.size(),name) == 0) {
      istringstream buf(line);
      istream_iterator<string> beg(buf), end;
      vector<string> values(beg, end);
      uid = values[1];
      break;
    }
  }
  // getting user name.
  stream = Util::getStream("/etc/passwd");
  while(std::getline(stream, line)){
    std::replace(line.begin(), line.end(), ':', ' ');
    istringstream buf(line);
    istream_iterator<string> beg(buf), end;
    vector<string> users(beg, end);
    if(users[2] == uid) {
      userName = users[0];
    } 
    break;
  }

  return userName;
}

