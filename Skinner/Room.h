/*
 * Room class -- Contains the persistent environmental state 
 *               variables associated with each room, as well
 *               as methods to retrieve them.  Also contains
 *               appetitive and noxious room sub-classes,
 *               which inherit from the main room class and
 *               add task-specific state variables & methods.
 */

 #ifndef ROOM_H
 #define ROOM_H

class Room
{
protected:
  int n; // number of rooms connected to (and including) current room
  int adj[3]; // list of room ID's connected to (and including) current room
  bool context; // context of the current room (i.e., wall color)
  bool locked; // whether the room is currently locked or unlocked
  float Pmove[3]; // probability of moving to each connected room (including this one)

public:
  int getN(); // get number of rooms connected to (and including) current room
  int getAdjAt(int); // get ID of connected room at specified index
  bool adjContains(int); // check whether given room number is in the list of connected rooms
  bool getContext(); // get context of current room (i.e., wall color)
  bool isLocked(); // check whether current room is locked or not
  void unlock(); // unlock the current room
  void lock(); // lock the current room
  float getPMoveAt(int); // get current probability of moving to room at specified index
  void setPMoveAt(int, float); // set new probability of moving to room at specified index
  virtual ~Room() {};
  virtual int getCorrectAt(int) { return -1; };
  virtual float getPButtonAt(int, int) { return -1.0f; };
  virtual void setPButtonAt(int, int, float) {};
  virtual int getTimer() { return -1; };
  virtual void tickTimer() {};
  virtual float getPJump() { return -1.0f; };
  virtual void setPJump(float) {};
};

class AppRoom : public Room
{
protected:
  int correct[3]; // correct sequence of button presses to complete the task
  float Pbutton[3][3]; // probability of pressing a given button at a given order index

public:
  AppRoom(int, int[3], int[3]); // appetitive room constructor
  ~AppRoom() {}; // appetitive room destructor
  int getCorrectAt(int); // get correct button at specified order index
  float getPButtonAt(int, int); // get current probability of pressing button at specified indices
  void setPButtonAt(int, int, float); // set new probability of pressing button at specified indices
};

class NoxRoom : public Room
{
protected:
  int timer; // number of time steps until shock occurs
  float Pjump; // probability of jumping over barrier

public:
  NoxRoom(int, int[3], int); // noxious room constructor
  ~NoxRoom() {}; // noxious room destructor
  int getTimer(); // get current number of time steps until shock occurs
  void tickTimer(); // decrement timer value by one
  float getPJump(); // get current probability of jumping over barrier
  void setPJump(float); // set new probability of jumping over barrier
};

#endif
