//define types
typedef struct Bird {
  float yPos;
  float yVel;
  int lastPos;
  int timerId;
};

typedef struct Pipe {
  int xPos;
  int top;
  int bottom;
  boolean active;
  int timerId;
};

typedef struct Game {
  Bird bird;
  int score;
  Pipe pipe;
  boolean sound_playing;
  boolean active;
};
