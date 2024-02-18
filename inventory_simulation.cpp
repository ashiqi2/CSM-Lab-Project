#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>

using namespace std;

void initialize();
void timing();
void order_arrival();
void evaluate();
void report();
void update_time_avg_stats();
float expon(float mean);
int random_integer(float* prob_distrib);
float uniform(float a, float b);
void demand();

int amount, bigs, initial_inv_level, inv_level, next_event_type, num_events, num_months, num_values_demand, smalls;
float area_holding, area_shortage, holding_cost, incremental_cost, maxlag, mean_interdemand, minlag, setup_cost, shortage_cost, sim_time, time_last_event, time_next_event[5], total_ordering_cost;

ifstream infile;
ofstream outfile;

float prob_distrib_demand[26];

int main() {

  int i, num_policies;

  // Open input and output files
  infile.open("inv.in");
  outfile.open("inv.out");

  // Specify the number of events for the timing function
  num_events = 4;

  // Read input parameters
  infile >> initial_inv_level >> num_months >> num_policies >> num_values_demand >> mean_interdemand >> setup_cost >> incremental_cost >> holding_cost >> shortage_cost >> minlag >> maxlag;

  for (i = 1; i <= num_values_demand; ++i) {
    infile >> prob_distrib_demand[i];
  }

  // Write report heading and input parameters
  outfile << "\t\t\t\t---------------------------------------------------------------------------\n";
  outfile << "\t\t\t\t|                    Single-product inventory system                       |\n";
  outfile << "\t\t\t\t---------------------------------------------------------------------------\n\n";
  outfile << "\t\t\t\tInitial inventory level:" << setw(24) << initial_inv_level << " items\n\n";
  outfile << "\t\t\t\tNumber of demand sizes:" << setw(25) << num_values_demand << "\n\n";
  outfile << "\t\t\t\tDistribution function of demand sizes:  ";
  for (i = 1; i <= num_values_demand; ++i) {
    outfile << setw(8) << prob_distrib_demand[i];
  }
  outfile << "\n\n\t\t\t\tMean interdemand time:" << setw(26) << mean_interdemand << "\n\n";
  outfile << "\t\t\t\tDelivery lag range:" << setw(29) << minlag << "  to" << setw(3) << maxlag << " months\n\n";
  outfile << "\t\t\t\tLength of the simulation:" << setw(22) << num_months << " months\n\n";
  outfile << "\t\t\t\tsetup_cost         = " << setw(3) << setup_cost << "\n";
  outfile << "\t\t\t\tincremental_cost   = " << setw(3) << incremental_cost << "\n";
  outfile << "\t\t\t\tholding_cost       = " << setw(3) << holding_cost << "\n";
  outfile << "\t\t\t\tshortage_cost      = " << setw(3) << shortage_cost << "\n\n";
  outfile << "\t\t\t\tNumber of policies = " << setw(2) << num_policies << "\n\n";
  outfile << "\t\t\t\t---------------------------------------------------------------------------\n";
  outfile << "\t\t\t\t|                            Final Result                                  |\n";
  outfile << "\t\t\t\t---------------------------------------------------------------------------\n\n";
  outfile << "\t\t\t\t                 Average        Average        Average        Average\n";
  outfile << "\t\t\t\t  Policy       total cost    ordering cost  holding cost   shortage cost";

  // Run the simulation varying the inventory policy
  for (i = 1; i <= num_policies; ++i) {
    // Read the inventory policy, and initialize the simulation
    infile >> smalls >> bigs;
    initialize();

    // Run the simulation until it terminates after an end-simulation event (type 3) occurs
    do {
        // Determine the next event
      timing();

      // Update time-average statistical accumulators
      update_time_avg_stats();

      // Invoke the appropriate event function
      switch (next_event_type) {
        case 1:
          order_arrival();
          break;
        case 2:
          demand();
          break;
        case 4:
          evaluate();
          break;
        case 3:
          report();
          break;
      }
    } while (next_event_type != 3);
  }

  // End the simulations
  infile.close();
  outfile.close();
  return 0;
}

void initialize() {
  // Initialize the simulation clock
  sim_time = 0.0;

  // Initialize the state variables
  inv_level = initial_inv_level;
  time_last_event = 0.0;

  // Initialize the statistical counters
  total_ordering_cost = 0.0;
  area_holding = 0.0;
  area_shortage = 0.0;

  // Initialize the event list
  time_next_event[1] = 1.0e+30;
  time_next_event[2] = sim_time + expon(mean_interdemand);
  time_next_event[3] = num_months;
  time_next_event[4] = 0.0;
}

void timing() {
  int i;
  float min_time_next_event = 1.0e+29;

  next_event_type = 0;

  // Determine the event type of the next event to occur
  for (i = 1; i <= num_events; ++i) {
    if (time_next_event[i] < min_time_next_event) {
      min_time_next_event = time_next_event[i];
      next_event_type = i;
    }
  }

  // Check if the event list is empty
  if (next_event_type == 0) {
    // The event list is empty, so stop the simulation
    outfile << "\nEvent list empty at time " << sim_time;
    // exit(1); // Uncomment if you want to exit the program when the event list is empty
  }

  // Advance the simulation clock
  sim_time = min_time_next_event;
}

void order_arrival() {
  // Increment the inventory level by the amount ordered
  inv_level += amount;

  // No order is now outstanding, so eliminate the order-arrival event from consideration
  time_next_event[1] = 1.0e+30;
}

void demand() {
  // Decrement the inventory level by a generated demand size
  inv_level -= random_integer(prob_distrib_demand);

  // Schedule the time of the next demand
  time_next_event[2] = sim_time + expon(mean_interdemand);
}

void evaluate() {
  // Check whether the inventory level is less than smalls
  if (inv_level < smalls) {
    // Inventory level is less than smalls, so place an order for the appropriate amount
    amount = bigs - inv_level;
    total_ordering_cost += setup_cost + incremental_cost * amount;

    // Schedule the arrival of the order
    time_next_event[1] = sim_time + uniform(minlag, maxlag);
  }

  // Schedule the next inventory evaluation regardless of the place-order decision
  time_next_event[4] = sim_time + 1.0;
}

void report() {
  // Compute and write estimates of desired measures of performance
  float avg_holding_cost, avg_ordering_cost, avg_shortage_cost;

  avg_ordering_cost = total_ordering_cost / num_months;
  avg_holding_cost = holding_cost * area_holding / num_months;
  avg_shortage_cost = shortage_cost * area_shortage / num_months;
  outfile << "\n\n\t\t\t\t(" << setw(3) << smalls << "," << setw(3) << bigs << ")" << setw(15) << avg_ordering_cost + avg_holding_cost + avg_shortage_cost << setw(15) << avg_ordering_cost << setw(15) << avg_holding_cost << setw(15) << avg_shortage_cost;
}

void update_time_avg_stats() {
  float time_since_last_event;

  // Compute time since last event and update last-event-time marker
  time_since_last_event = sim_time - time_last_event;
  time_last_event = sim_time;

  // Determine the status of the inventory level during the previous interval
  // Update area_shortage if it was negative, update area_holding if it was positive
  if (inv_level < 0) {
    area_shortage -= inv_level * time_since_last_event;
  }
  else if (inv_level > 0) {
    area_holding += inv_level * time_since_last_event;
  }
}

float expon(float mean) {
  // Return an exponential random variate with mean "mean"
  return -mean * log((float) rand() / RAND_MAX);
}

int random_integer(float* prob_distrib) {
  int i;
  float u;

  // Generate a U(0,1) random variate
  u = (float) rand() / RAND_MAX;

  // Return a random integer in accordance with the (cumulative) distribution function prob_distrib
  for (i = 1; u >= prob_distrib[i]; ++i);
  return i;
}

float uniform(float a, float b) {
  // Return a U(a,b) random variate
  return a + ((float) rand() / RAND_MAX) * (b - a);
}
