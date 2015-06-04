// Libraries:
#include <iostream>
#include <vector>
#include <cmath>

// Headers:
#include "Agent.h"
#include "Room.h"

using namespace std;

int Max_Eps; // The maximum number of episodes
double Gamma, Alpha; // The discount factor and learning rate
double Qmat[9][10]; // The Q matrix
Agent* Bob; // It's Agent Bob!
vector<Room*> Rooms; // Vector containing the various rooms in the maze

// Free all memory allocated for Rooms vector
void destroyRooms()
{
  int rm;
  for (rm = 0; rm < ((int) Rooms.size()); rm++) {
    delete Rooms[rm];
  }
}

// Select a connected room to move to (including current room)
//  * Based on Pmove values for the given room
//  * This assumes that the current room is unlocked!
int selectMove(int rm)
{
  int n = Rooms[rm]->getN();
  int i;
  float Psum = 0.0f;
  float Pmove[n];
  for (i = 0; i < n; i++) {
    float Pmove_i = Rooms[rm]->getPMoveAt(i);
    Pmove[i] = Pmove_i + Psum;
    Psum += Pmove_i;
  }
  float r = ((float) rand()) / ((float) RAND_MAX);
  for (i = 0; i < n; i++) {
    if (r <= Pmove[i]) { return Rooms[rm]->getAdjAt(i); }
  }
  return Rooms[rm]->getAdjAt(n - 1);
}

// Select the action to take in an appetitive room (button task)
//  * Returns the button (0, 1, or 2) that Bob decides to press
//  * Based on Pbutton values for the given room & order index
//  * This assumes that the current room is both locked & appetitive!
int selectButton(int rm, int i)
{
  int j;
  float Psum = 0.0f;
  float Pbutton[3];
  AppRoom* room = (dynamic_cast <AppRoom*> (Rooms[rm]));
  for (j = 0; j < 3; j++) {
    float Pbutton_ij = room->getPButtonAt(i, j);
    Pbutton[j] = Pbutton_ij + Psum;
    Psum += Pbutton_ij;
  }
  float r = ((float) rand()) / ((float) RAND_MAX);
  for (j = 0; j < 3; j++) {
    if (r <= Pbutton[j]) { return j; }
  }
  return 2;
}

// Select the action to take in a noxious room (shock task)
//  * Returns true if Bob jumps, or false if Bob doesn't jump
//  * Based on Pjump value for the given room
//  * This assumes that the current room is both locked & noxious!
bool selectJump(int rm)
{
  float Pjump = (dynamic_cast <NoxRoom*> (Rooms[rm]))->getPJump();
  float r = ((float) rand()) / ((float) RAND_MAX);
  return (r <= Pjump);
}

// Get the minimum (i.e., optimal) Q value associated with the given room
double minQ(int rm)
{
  vector<double> Q;
  Room* room = Rooms[rm];
  int n = room->getN();
  int i;
  for (i = 0; i < n; i++) {
    int id = room->getAdjAt(i);
    Q.push_back(Qmat[rm][id]);
  }
  return *min_element(Q.begin(), Q.end());
}

// Alter the Pmove values for the given room, based upon the Q values
// from previous episodes
void associateMove(int rm)
{
  Room* room = Rooms[rm];
  int n = room->getN();
  if (n > 1) {
    vector<double> Q;
    int i;
    for (i = 0; i < n; i++) {
      int idx = room->getAdjAt(i);
      Q.push_back(Qmat[rm][idx]);
    }
    double max = *max_element(Q.begin(), Q.end());
    double sum = 0.0f;
    for (i = 0; i < n; i++) {
      sum += max - Q[i];
    }
    for (i = 0; i < n; i++) {
      if (Q[i] == max)
        room->setPMoveAt(i, 0.1f);
      else {
        float newP = (max - Q[i]) * (0.9f / sum);
        room->setPMoveAt(i, newP);
      }
    }
  }
}

// Alter the Pbutton values for the given button-order bo, depending
// on whether the chosen button-index bi was correct or not
//  * This assumes that the current room is both locked & appetitive!
void associateButton(int rm, int bo, int bi)
{
  AppRoom* room = (dynamic_cast <AppRoom*> (Rooms[rm]));
  bool correct = (bi == room->getCorrectAt(bo));
  int i;
  float Pbutton;

  if (correct) { // If the correct button was pressed...
    for (i = 0; i < 3; i++) {
      Pbutton = room->getPButtonAt(bo, i);
      if (i == bi) { // Increment the Pbutton value for this order & index
        if (Pbutton < 0.9f)
          room->setPButtonAt(bo, i, Pbutton + 0.1f);
        else
          room->setPButtonAt(bo, i, 1.0f); // Maximum of 1
      }
      else { // Decrement the Pbutton value for this order, other indices
        if (Pbutton > 0.05f)
          room->setPButtonAt(bo, i, Pbutton - 0.05f);
        else
          room->setPButtonAt(bo, i, 0.0f); // Minimum of 0
      }
    }
  }
  else { // If the incorrect button was pressed...
    for (i = 0; i < 3; i++) {
      Pbutton = room->getPButtonAt(bo, i);
      if (i == bi) { // Decrement the Pbutton value for this order & index
        if (Pbutton > 0.1f)
          room->setPButtonAt(bo, i, Pbutton - 0.1f);
        else
          room->setPButtonAt(bo, i, 0.0f); // Minimum of 0
      }
      else { // Increment the Pbutton value for this order, other indices
        if (Pbutton < 0.95f)
          room->setPButtonAt(bo, i, Pbutton + 0.05f);
        else
          room->setPButtonAt(bo, i, 1.0f); // Maximum of 1
      }
    }
  }
}

// Increment Pjump values in all other noxious rooms by 0.1 (up to 0.9 max)
//  * Represents the association of noxious stimulus with environmental context
//  * Should only be called when Agent Bob receives a shock!
void associateJump()
{
  int n = Rooms.size();
  int rm;
  for (rm = 0; rm < n; rm++) {
    if (Rooms[rm]->getContext()) {
      NoxRoom* room = (dynamic_cast <NoxRoom*> (Rooms[rm]));
      float Pjump = room->getPJump();
      if (Pjump < 0.9f) {
        room->setPJump(Pjump + 0.2f);
      }
    }
  }
}

// Main Function
int main(int argc, char* argv[])
{
  int i, j, ep; // Just some iterators

  // Set the maximum number of episodes, discount factor, and learning rate
  if (argc < 2) {
    Max_Eps = 10;
    printf("Setting max no. of episodes to default (10)\n");
    Gamma = 0.5f;
    printf("Setting discount factor to default (0.5)\n");
    Alpha = 0.2f;
    printf("Setting learning rate to default (0.2)\n\n");
  }
  else if (argc < 3) {
    Max_Eps = atoi(argv[1]);
    printf("Setting max no. of episodes to %d\n", Max_Eps);
    Gamma = 0.5f;
    printf("Setting discount factor to default (0.5)\n");
    Alpha = 0.2f;
    printf("Setting learning rate to default (0.2)\n\n");
  }
  else if (argc < 4) {
    Max_Eps = atoi(argv[1]);
    printf("Setting max episodes to %d\n", Max_Eps);
    Gamma = atof(argv[2]);
    printf("Setting discount factor to %f\n", Gamma);
    Alpha = 0.2f;
    printf("Setting learning rate to default (0.2)\n\n");
  }
  else {
    Max_Eps = atoi(argv[1]);
    printf("Setting max episodes to %d\n", Max_Eps);
    Gamma = atof(argv[2]);
    printf("Setting discount factor to %f\n", Gamma);
    Alpha = atof(argv[3]);
    printf("Setting learning rate to %f\n\n", Alpha);
  }

  // Initialize the Q matrix
  for (i = 0; i < 9; i++) {
    for (j = 0; j < 10; j++)
      Qmat[i][j] = 0.0f;
  }

  // Construct the rooms in the maze, and add them to the Rooms vector
  Rooms.push_back(new AppRoom(1, (int[]){1, -1, -1}, (int[]){0, 1, 1}));
  Rooms.push_back(new NoxRoom(3, (int[]){0, 2, 4}, 3));
  Rooms.push_back(new AppRoom(2, (int[]){1, 5, -1}, (int[]){2, 0, 1}));
  Rooms.push_back(new NoxRoom(2, (int[]){4, 6, -1}, 4));
  Rooms.push_back(new AppRoom(2, (int[]){1, 3, -1}, (int[]){2, 1, 1}));
  Rooms.push_back(new AppRoom(2, (int[]){2, 8, -1}, (int[]){0, 1, 2}));
  Rooms.push_back(new AppRoom(2, (int[]){3, 7, -1}, (int[]){0, 0, 2}));
  Rooms.push_back(new NoxRoom(3, (int[]){6, 8, 9}, 2));
  Rooms.push_back(new AppRoom(2, (int[]){5, 7, -1}, (int[]){2, 2, 2}));

  for (ep = 0; ep < Max_Eps; ep++) { // Limit of Max_Eps episodes
    int rm; // Initialize room iterator
    int R = 0; // The R value, or the difference between oldSum and the
               // sum after moving to a specific room
    int oldSum = 0; // The sum of hunger & pain values before moving to
                    // a new room (used to calculate R value)

    // Make sure all rooms are locked at the start of each episode
    for (rm = 0; rm < ((int) Rooms.size()); rm++)
      Rooms[rm]->lock();

    Bob = new Agent(); // Create Agent Bob.  Hi, Bob :)

    rm = 0; // Set room iterator to 0
    int last = -1; // Initialize last-room iterator to -1
    int bo = 0; // Initialize button-order iterator to 0

    do { // Do the following while not at exit room (9)
      if (Rooms[rm]->isLocked()) { // The room is locked
        if (Rooms[rm]->getContext()) { // The room is locked & noxious
          NoxRoom* room = (dynamic_cast <NoxRoom*> (Rooms[rm]));
          if (room->getTimer() == 0) { // Shock time!
            room->setPJump(0.9f); // Bob really wants to jump now!
            associateJump(); // Associate this context with bad stuff happening
            if (selectJump(rm)) { // Agent Bob jumped over the barrier!
              room->unlock(); // Task complete! Unlock the current room
            }
            Bob->timeStep(1, 20); // Agent Bob gets shocked! Ouch!
          }
          else { // Not shock time yet...
            room->tickTimer(); // One second closer to shockage...
            if (selectJump(rm)) { // Agent Bob jumped over the barrier!
              room->unlock(); // Task complete! Unlock the current room
            }
            Bob->timeStep(1, -1); // Agent Bob is hungry & licking his wounds
          }
        }
        else { // The room is locked & appetitive
          AppRoom* room = (dynamic_cast <AppRoom*> (Rooms[rm]));
          int bi = selectButton(rm, bo); // Bob presses a button...
          associateButton(rm, bo, bi); // Alter Pbutton values based on chosen button
          if (bi == room->getCorrectAt(bo)) { // The right button was pressed!
            if (bo == 2) { // That was the last button!
              bo = 0; // Reset the button-order iterator
              room->unlock(); // Task complete! Unlock the current room
            }
            else // That wasn't the last button...
              bo++; // Increment the button-order iterator
            Bob->timeStep(-5, -1); // Agent Bob gets a food pellet! Yum!
          }
          else { // The wrong button was pressed!
            bo = 0; // Reset the button-order iterator
            Bob->timeStep(1, -1); // Agent Bob is hungry & licking his wounds
          }
        }
      }
      else { // The room is unlocked
        if (last == -1)
          oldSum = Bob->getH() + Bob->getP();
        else {
          int newSum = Bob->getH() + Bob->getP();
          R = newSum - oldSum;
          oldSum = newSum;
          Qmat[last][rm] += Alpha * (R + (Gamma * minQ(rm)) - Qmat[last][rm]);
          associateMove(last); // Adjust Pmove values based on Q values
        }

        int next = selectMove(rm); // Select the next room to move to
        if (next == 9)
          Bob->timeStep(-10, -1); // Agent Bob reached the exit!
        else
          Bob->timeStep(1, -1); // Agent Bob is hungry & licking his wounds
        last = rm; // Change the last room to the current room
        rm = next; // Change the current room to the next room
      }
    } while (rm != 9); // End do-while loop when goal state is reached

    R = Bob->getH() + Bob->getP() - oldSum;
    Qmat[7][9] += Alpha * (R - Qmat[7][9]);
    associateMove(7);

    if (ep == (Max_Eps - 1)) {
      printf("Completed episode %d in %d time steps, with %d hunger and %d pain.\n",
             Max_Eps, Bob->getT(), Bob->getH(), Bob->getP());
    }

    delete Bob; // Destroy Agent Bob.  Bye, Bob :(
  } // End for loop when Max_Eps episodes have been completed

  printf("\nQ Matrix:\n");
  printf("  0        1        2        3        4        5        6        7        8        9\n");
  for (i = 0; i < 9; i++) {
    printf("%d ", i);
    for (j = 0; j < 10; j++) {
      double Q = round(Qmat[i][j] * 100) / 100;
      printf("%f ", Q);
    }
    printf("\n");
  }

  // printf("\nProbabilities:\n");
  // for (i = 0; i < ((int) Rooms.size()); i++) {
  //   Room* room = Rooms[i];
  //   printf("From room %d to:\n", i);
  //   for (j = 0; j < room->getN(); j++) {
  //     int pct = (int) (room->getPMoveAt(j) * 100);
  //     printf("  room %d (%d%%)\n", room->getAdjAt(j), pct);
  //   }
  // }

  destroyRooms(); // free memory allocated to Rooms vector

  return 0;
}
