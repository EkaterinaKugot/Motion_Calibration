// 4, 7 for direction
// 5, 6 for speed

#define DIR_RIGHT 4//false - forward, true - back
#define DIR_LEFT 7//false - forward, true - back
#define SPEED_RIGHT 5
#define SPEED_LEFT 6

#define FORWARD 'F'
#define BACKWARD 'B'
#define LEFT 'L'
#define RIGHT 'R'
#define CHANGE_STATE 'A'
#define RETURN_STATE 'X'
#define MOVE 'P'

#define CHANGE 'T'
#define SECURE 'C'

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);// RX, TX

enum State{
  EMPTY,
  DIRECTION,
  SPEED,
  TURN,
  MOVEMENT,
};

void move(bool lforward, bool rforward, int lvelocity, int rvelocity){
  digitalWrite(DIR_RIGHT, rforward);
  digitalWrite(DIR_LEFT, lforward);
  analogWrite(SPEED_RIGHT, rvelocity);
  analogWrite(SPEED_LEFT, lvelocity);
}

void move_forward(int velocity){
  move(true, true, velocity, velocity);
}

void move_back(int velocity){
  move(false, false, velocity, velocity);
}

void rotate_left(int velocity){
  move(false, true, velocity, velocity);
}

void rotate_right(int velocity){
  move(true, false, velocity, velocity);
}

void turn_right(int velocity){
  move(true, true, velocity, velocity/2);
}
void turn_left(int velocity){
  move(true, true, velocity/2, velocity);
}

void stop(){
  move(true, true, 0, 0);
}

State state = EMPTY;
State prev_state = EMPTY;

char* stateToString(State st) {
  switch (st) {
    case EMPTY: return "EMPTY";
    case DIRECTION: return "DIRECTION";
    case SPEED: return "SPEED";
    case TURN: return "TURN";
    case MOVEMENT: return "MOVEMENT";
    default: return "UNKNOWN";
  }
}

typedef void (*CommandFunction)(int);

CommandFunction move_commands[4] = {
  move_forward,     
  move_back, 
  rotate_right,    
  rotate_left
};

char command;
int time;
char prev_comm;

int const size = 4;
char tmp_command_chars[size] = {'F', 'B', 'R', 'L'};
char command_chars[size] = {'q', 'q', 'q', 'q'};

bool is_change = false;
int fixed_speed = 200;

void setup() {
  for(int i = 0; i<=7; i++)
  {
      pinMode(i, OUTPUT);
  }
  
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }
  mySerial.begin(9600);
}

void loop() {
  switch(state){
    case EMPTY:
      break;
    case MOVEMENT:
      executeCommand(command);
      break;
    case DIRECTION:
      direction_calibration(command);
      break;
  }

  if(mySerial.available()){
    command = mySerial.read();
    Serial.println(command);
    select_state();
  }
}

void direction_calibration(char command){
  switch(command){
    case(CHANGE): // меняем движение
      if (!is_change){
        shiftLeft(tmp_command_chars, size);
        is_change = true;
      }
      break;
    case(SECURE): //закрепляем
      for (int i = 0; i < 4; ++i) {
        if (prev_comm == tmp_command_chars[i]) {
          command_chars[i] = prev_comm;
          printArray(command_chars, size);
          return;
        }
      }
      break;
    default:
      is_change = false;
      for (int i = 0; i < 4; ++i) {
        if (command == tmp_command_chars[i]) {
          move_commands[i](fixed_speed);
          prev_comm = command;
          return;
        }
      }
      stop();
      break;
  }
}

void select_state(){
  if (command == CHANGE_STATE){
    switch(state){
      case EMPTY:
        state = DIRECTION;
        break;
      case DIRECTION:
        state = SPEED;
        break;
      case SPEED:
        state = TURN;
        break;
      case TURN:
        state = DIRECTION;
        break;
    }
  }else if (command == MOVE){
    prev_state = state;
    state = MOVEMENT;
  } else if (command == RETURN_STATE){
    state = prev_state;
  }
  // Serial.println(stateToString(state));
}

void shiftLeft(char arr[], int size) {
  char firstElement = arr[0];
  for (int i = 0; i < size - 1; ++i) {
    arr[i] = arr[i + 1];
  }
  arr[size - 1] = firstElement;
  printArray(arr, size);
}

void printArray(char arr[], int size) {
  for (int i = 0; i < size; ++i) {
    Serial.print(arr[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void executeCommand(char command){
  for (int i = 0; i < 4; ++i) {
    if (command == command_chars[i]) {
      move_commands[i](fixed_speed);
      return;
    }
  }
  stop();
}

