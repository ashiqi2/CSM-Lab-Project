#include<bits/stdc++.h>
#include <windows.h>
#include <unistd.h>
using namespace std;

#define nl '\n';

void printString(string s) {
  int n = s.size();
  cout << s;
  for (int i = n + 1; i <= 20; ++i) cout << " ";
}

double getRandomNumber(double l, double r) {
  mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
  uniform_real_distribution <double> A(l, r);
  return A(rng);
}

class Customer {
public:
  int customerId;
  double arrivalTime;
  double serviceStartTime;
  double departureTime;
  double serviceTime;
  double interArrivalTime;
  double queueDelayTime;
  Customer() {
    customerId = 0;
    arrivalTime = 0.0;
    serviceStartTime = 0.0;
    departureTime = 0.0;
    serviceTime = 0.0;
    interArrivalTime = 0.0;
    queueDelayTime = 0.0;
  }
};

class Event {
public:
  double invokeTime;
  string type;
  int customerId;
  Event(double invokeTime, string type, int customerId) {
    this->invokeTime = invokeTime;
    this->type = type;
    this->customerId = customerId;
  }
};

class Compare {
public:
  bool operator() (Event a, Event b) {
    if (a.invokeTime == b.invokeTime) {
      if (a.type == "departure") return false;
      return true;
    }
    return a.invokeTime > b.invokeTime;
  }
};

class EventRecord {
public:
  string type;
  double startTime;
  int customerId;
  string serverStatus;
  int queueSize;
  EventRecord(string type, double startTime, int customerId, string serverStatus, int queueSize) {
    this->type = type;
    this->startTime = startTime;
    this->customerId = customerId;
    this->serverStatus = serverStatus;
    this->queueSize = queueSize;
  }
};

class Server {
public:
  string status;
};

void updateClock(double& clock, double time) {
  clock = time;
}

void rowLine() {
  for (int i = 1; i <= 100; ++i) cout << "-";
  cout << nl;
}

int main() {
  double clock = 0;
  Server server;
  server.status = "IDLE";
  int numberOfCustomer;
  double interArrivalTimeRangeL;
  double interArrivalTimeRangeR;
  double serviceTimeRangeL;
  double serviceTimeRangeR;
  cout << "Enter the number of customer: ";
  cin >> numberOfCustomer;
  cout << "Enter inter arrival time range: ";
  cin >> interArrivalTimeRangeL >> interArrivalTimeRangeR;
  cout << "Enter service time range: ";
  cin >> serviceTimeRangeL >> serviceTimeRangeR;
  // customer input generate
  vector<Customer> customer(numberOfCustomer + 5);
  for (int i = 1; i <= numberOfCustomer; ++i) {
    customer[i].interArrivalTime = getRandomNumber(interArrivalTimeRangeL, interArrivalTimeRangeR);
    customer[i].serviceTime = getRandomNumber(serviceTimeRangeL, serviceTimeRangeR);
    customer[i].customerId = i;
  }

  priority_queue<Event, vector<Event>, Compare> eventQueue;
  vector<EventRecord> eventRecord;
  queue<int> serviceQueue;
  int currentlyServedCustomerId;
  for (int i = 1; i <= numberOfCustomer; ++i) {
    if (i == 1) {
      customer[i].arrivalTime = customer[i].interArrivalTime;
    } else {
      customer[i].arrivalTime = customer[i - 1].arrivalTime + customer[i].interArrivalTime;
    }
    Event newEvent(customer[i].arrivalTime, "arrival", customer[i].customerId);
    eventQueue.push(newEvent);
  }
  while (!eventQueue.empty()) {
    Event curEvent = eventQueue.top();
    eventQueue.pop();
    updateClock(clock, curEvent.invokeTime);
    if (curEvent.type == "arrival") {
      EventRecord newEventRecord("arrival", clock, curEvent.customerId, server.status, serviceQueue.size());
      eventRecord.push_back(newEventRecord);
      if (server.status == "BUSY") {
        serviceQueue.push(curEvent.customerId);
      } else if (server.status == "IDLE") {
        currentlyServedCustomerId = curEvent.customerId;
        customer[currentlyServedCustomerId].serviceStartTime = clock;
        server.status = "BUSY";
        Event newEvent(clock + customer[currentlyServedCustomerId].serviceTime, "departure", currentlyServedCustomerId);
        eventQueue.push(newEvent);
        newEventRecord = EventRecord("service", clock, currentlyServedCustomerId, server.status, serviceQueue.size());
        eventRecord.push_back(newEventRecord);
      }
    } else if (curEvent.type == "departure") {
      customer[currentlyServedCustomerId].departureTime = clock;
      server.status = "IDLE";
      EventRecord newEventRecord("departure", clock, currentlyServedCustomerId, server.status, serviceQueue.size());
      eventRecord.push_back(newEventRecord);
      if (!serviceQueue.empty()) {
        currentlyServedCustomerId = serviceQueue.front();
        serviceQueue.pop();
        customer[currentlyServedCustomerId].serviceStartTime = clock;
        server.status = "BUSY";
        Event newEvent(clock + customer[currentlyServedCustomerId].serviceTime, "departure", currentlyServedCustomerId);
        eventQueue.push(newEvent);
        newEventRecord = EventRecord("service", clock, currentlyServedCustomerId, server.status, serviceQueue.size());
        eventRecord.push_back(newEventRecord);
      }
    }
  }
  for (int i = 1; i <= (100 - 18) / 2; ++i) cout << " ";
  cout << "Simulation Process\n";
  rowLine();
  printString("Type");
  printString("Start Time");
  printString("Customer");
  printString("Server Status");
  printString("Queue Size");
  cout << nl;
  rowLine();
  for (auto x : eventRecord) {
    sleep(1);
    printString(x.type);
    printString(to_string(x.startTime));
    printString(to_string(x.customerId));
    printString(x.serverStatus);
    printString(to_string(x.queueSize));
    cout << nl;
    rowLine();
  }

  double totalSimulationTime = eventRecord.back().startTime;
  double totalInverArrivalTime = 0.0;
  double totalServiceTime = 0.0;
  double totalQueueDelayTime = 0.0;
  for (int i = 1; i <= numberOfCustomer; ++i) {
    customer[i].queueDelayTime = customer[i].serviceStartTime - customer[i].arrivalTime;
    totalInverArrivalTime += customer[i].interArrivalTime;
    totalServiceTime += customer[i].serviceTime;
    totalQueueDelayTime += customer[i].queueDelayTime;
  }
  double averageInterArrivalTime = totalInverArrivalTime / (double) numberOfCustomer;
  double averageServiceTime = totalServiceTime / (double) numberOfCustomer;
  double averageQueueDelayTime = totalQueueDelayTime / (double) numberOfCustomer;

  double totalQueueLength = 0.0;
  for (int i = 1; i < (3 * numberOfCustomer); ++i) {
    double interEventTime = eventRecord[i].startTime - eventRecord[i - 1].startTime;
    double len = eventRecord[i - 1].queueSize;
    totalQueueLength += interEventTime * len;
  }
  double averageQueueLength = totalQueueLength / totalSimulationTime;
  double serverUtilization = (totalServiceTime * 100) / totalSimulationTime;
  cout << nl;
  cout << "Number of customer: " << numberOfCustomer << nl;
  cout << "Total Simulation Time: " << totalSimulationTime << nl;
  cout << "average Inter Arrival Time: " << averageInterArrivalTime << nl;
  cout << "Average Service Time: " << averageServiceTime << nl;
  cout << "Average waiting Time: " << averageQueueDelayTime << nl;
  cout << "Average Queue Length: " << averageQueueLength << nl;
  cout << "Server Utilization: " << serverUtilization << "%" << nl;
  cout << nl;

  // create CSV file
  ofstream myfile;
  myfile.open ("event.csv");
  myfile << "Simulation Process Information" << nl;
  myfile << "Type,Start Time,Customer,Server Status,Queue Size" << nl;
  for (auto x : eventRecord) {
    myfile << x.type << "," << x.startTime << "," << x.customerId << "," << x.serverStatus << "," << x.queueSize << nl;
  }
  myfile.close();

  myfile.open("customer.csv");
  myfile << "Customer Information" << nl;
  myfile << "Customer,Inter Arrival Time,Arrival Time,Service Time,Service Start Time,Departure Time,Queue Waiting Time" << nl;
  for (int i = 1; i <= numberOfCustomer; ++i) {
    myfile << customer[i].customerId << "," << customer[i].interArrivalTime << "," << customer[i].arrivalTime << "," << customer[i].serviceTime << "," << customer[i].serviceStartTime << "," << customer[i].departureTime << "," << customer[i].queueDelayTime << nl;
  }
  myfile.close();
  myfile.open("statistics.csv");
  myfile << "Simulation Statistics" << nl;
  myfile << "Number of customer," << numberOfCustomer << nl;
  myfile << "Total Simulation Time," << totalSimulationTime << nl;
  myfile << "Average Inter Arrival Time," << averageInterArrivalTime << nl;
  myfile << "Average Service Time," << averageServiceTime << nl;
  myfile << "Average Waiting Time," << averageQueueDelayTime << nl;
  myfile << "Average Queue Length," << averageQueueLength << nl;
  myfile << "Server Utilization," << serverUtilization << "%" << nl;
  return 0;
}