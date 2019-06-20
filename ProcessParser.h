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

vector<string> ProcessParser::getPidList()
{
    DIR* dir;
    // Basically, we are scanning /proc dir for all directories with numbers as their names
    // If we get valid check we store dir names in vector as list of machine pids
    vector<string> container;
    if(!(dir = opendir("/proc")))
        throw std::runtime_error(std::strerror(errno));

    while (dirent* dirp = readdir(dir)) {
        // is this a directory?
        if(dirp->d_type != DT_DIR)
            continue;
        // Is every character of the name a digit?
        if (all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){ return std::isdigit(c); })) {
            container.push_back(dirp->d_name);
        }
    }
    //Validating process of directory closing
    if(closedir(dir))
        throw std::runtime_error(std::strerror(errno));
    return container;
}

string ProcessParser::getCmd(string pid)
{
    string line;
    ifstream stream = Util::getStream((Path::basePath() + pid + Path::cmdPath()));
    std::getline(stream, line);
    return line;
}

int ProcessParser::getNumberOfCores()
{
    // Get the number of host cpu cores
    string line;
    string name = "cpu cores";
    ifstream stream = Util::getStream((Path::basePath() + "cpuinfo"));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return stoi(values[3]);
        }
    }
    return 0;
}


vector<string> ProcessParser::getSysCpuPercent(string coreNumber) {
  string line;
  string name = "cpu" + coreNumber;
  ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
  while (std::getline(stream, line)) {
    if (line.compare(0, name.size(),name) == 0) {
      istringstream buf(line);
      istream_iterator<string> beg(buf), end;
      vector<string> values(beg, end);
      return values;
    }

  }
  return (vector<string>());

}
float getSysActiveCpuTime(vector<string> values)
{
    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]));
}

float getSysIdleCpuTime(vector<string>values)
{
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2)
{
/*
Because CPU stats can be calculated only if you take measures in two different time,
this function has two parameters: two vectors of relevant values.
We use a formula to calculate overall activity of processor.
*/
    float activeTime = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
    // cout << "Active Time1: " << getSysActiveCpuTime(values1)<< "\n";
    // cout << "Active Time2: " << getSysActiveCpuTime(values2)<< "\n";
    float idleTime = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
    // cout << "Idle Time1: " << getSysIdleCpuTime(values1)<< "\n";
    // cout << "Idle Time2: " << getSysIdleCpuTime(values2)<< "\n";
    
    float totalTime = activeTime + idleTime;
    float result = 100.0*(activeTime / totalTime);
    return to_string(result);
}
    
float ProcessParser::getSysRamPercent(){
  string name1 = "MemAvailable:";
  string name2 = "MemFree:";
  string name3 = "Buffers:";
  string line;
  float totalMem = 0;
  float freeMem = 0;
  float buffers = 0;
  ifstream stream = Util::getStream((Path::basePath() + Path::memInfoPath()));
  while (std::getline(stream, line)) {
    if (line.compare(0, name1.size(),name1) == 0) {
      istringstream buf(line);
      istream_iterator<string> beg(buf), end;
      vector<string> values(beg, end);
      totalMem = stof(values[1]);
      // cout << "TotalMem: " << totalMem << "\n";
    }
    if (line.compare(0, name2.size(),name2) == 0) {
      istringstream buf(line);
      istream_iterator<string> beg(buf), end;
      vector<string> values(beg, end);
      freeMem = stof(values[1]);
      // cout << "FreeMem: " << freeMem << "\n";
    }
    if (line.compare(0, name3.size(),name3) == 0) {
      istringstream buf(line);
      istream_iterator<string> beg(buf), end;
      vector<string> values(beg, end);
      buffers = stof(values[1]);
      // cout << "Buffers: " << buffers << "\n";
    }
 }

 return float(100*(1-(freeMem/(totalMem - buffers))));
}

string ProcessParser::getSysKernelVersion() {
    string line;
    string name = "Linux version ";
    ifstream stream = Util::getStream((Path::basePath() + Path::versionPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return values[2];
        }
    }
    return "";

}
string ProcessParser::getOSName() {
  string line;
  string name = "PRETTY_NAME=";
  ifstream stream = Util::getStream("/etc/os-release");
  while (std::getline(stream, line)) {
    if (line.compare(0, name.size(),name) == 0) {
      std::size_t found = line.find("=");
      found++;
      string result = line.substr(found);
      result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
      return result;  
    }
  }

  return "";
}
    

int ProcessParser::getTotalThreads() {
  vector<string> pidList = ProcessParser::getPidList();
  string line;
  string name = "Threads:";
  int total = 0;
  for (string pid : pidList) {
    ifstream stream = Util::getStream(Path::basePath() + pid + "/" + Path::statusPath());
    while (std::getline(stream, line)) {
      if (line.compare(0, name.size(),name) == 0) {
        istringstream buf(line);
        istream_iterator<string> beg(buf), end;
        vector<string> values(beg, end);
        total += stoi(values[1]);
      }
    }
  }
  
  return total;
}


int ProcessParser::getTotalNumberOfProcesses() {
    string line;
    string name = "processes";
    ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return stoi(values[1]);
        }
    }

    return 0;
}

int ProcessParser::getNumberOfRunningProcesses() {
    string line;
    string name = "procs_running";
    ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return stoi(values[1]);
        }
    }

    return 0;

}
