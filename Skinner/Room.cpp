/*
 * Room class implementation -- see header file 'Room.h' for more info
 */

#include "Room.h"

// Get number of rooms connected to (and including) current room
int Room::getN()
{
  return n;
}

// Get integer ID of connected room at index i
int Room::getAdjAt(int i)
{
  return adj[i];
}

bool Room::adjContains(int rm)
{
  bool result = false;
  int i;
  for (i = 0; i < n; i++) {
    if (adj[i] == rm)
      result = true;
  }
  return result;
}

// Get wall color of current room (for context)
bool Room::getContext()
{
  return context;
}

// Check whether current room is locked or not
bool Room::isLocked()
{
  return locked;
}

// Unlock the current room
void Room::unlock()
{
  locked = false;
}

// Lock the current room
void Room::lock()
{
  locked = true;
}

// Get the current probability of moving to the connected room at index i
float Room::getPMoveAt(int i)
{
  return Pmove[i];
}

// Set the new probability newP of moving to the connected room at index i
void Room::setPMoveAt(int i, float newP)
{
  Pmove[i] = newP;
}

// Appetitive stimulus (button task) room constructor
AppRoom::AppRoom(int N, int Adj[], int Correct[])
{
  n = N;
  context = false; // appetitive rooms have false context (i.e., blue walls)
  locked = true; // all rooms are initially locked
  for (int i = 0; i < 3; i++) {
    adj[i] = Adj[i];
    // probabilities of moving to each connected room are initially equal
    if (i < n)
      Pmove[i] = 1.0f / ((float) n);
    else // disregard the remainder of the array
      Pmove[i] = 0.0f;
  }
  // probabilities of selecting each button are initially equal
  for (int i = 0; i < 3; i++) {
    correct[i] = Correct[i];
    for (int j = 0; j < 3; j++)
      Pbutton[i][j] = 0.33f;
  }
}

// Get the correct button at the specified index
int AppRoom::getCorrectAt(int i)
{
  return correct[i];
}

// Get the current probability of pressing the button at indices i, j
float AppRoom::getPButtonAt(int i, int j)
{
  return Pbutton[i][j];
}

// Set the new probability newP of pressing the button at indices i, j
void AppRoom::setPButtonAt(int i, int j, float newP)
{
  Pbutton[i][j] = newP;
}

// Noxious stimulus (shock task) room constructor
NoxRoom::NoxRoom(int N, int Adj[], int Timer)
{
  n = N;
  context = true; // noxious rooms have true context (i.e., red walls)
  locked = true; // all rooms are initially locked
  for (int i = 0; i < 3; i++) {
    adj[i] = Adj[i];
    // probabilities of moving to each connected room are initially equal
    if (i < n)
      Pmove[i] = 1.0f / ((float) n);
    else // disregard the remainder of the array
      Pmove[i] = 0.0f;
  }
  timer = Timer;
  Pjump = 0.1f; // probability of jumping over barrier is initially 10%
}

// Get the current number of time steps until shock occurs
int NoxRoom::getTimer()
{
  return timer;
}

// Decrement the timer value by one
void NoxRoom::tickTimer()
{
  timer--;
}

// Get the current probability of jumping over the barrier
float NoxRoom::getPJump()
{
  return Pjump;
}

// Set the new probability newP of jumping over the barrier
void NoxRoom::setPJump(float newP)
{
  Pjump = newP;
}
