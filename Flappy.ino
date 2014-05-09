#include "LedControl.h"
#include "Timer.h"
#include "types.h"
#include "digits.h"
// matrix digits inspired by http://timewitharduino.blogspot.com/2012/01/mixiclock-4-digits-displayed-on-8x8-led.html


#define BUTTON 2

const float gravity = .002857;
const float flap_power = -.064;
const int max_grav_multiple  = 13;



// DIN, CLK, CS, number of screens

//game display
LedControl M = LedControl(8,10,9,1);

//score display
LedControl M2 = LedControl (11, 13, 12, 1);

//pin the speaker is plugged in to
const int speaker = 7;

Timer T;
Game G;



//make sure the jump sound doesn't play too quickly in  succession 
int sound_count;

void setup(){
 M.shutdown(0, false);
 M.setIntensity(0,2);
 M.clearDisplay(0);
 
 M2.shutdown(0, false);
 M2.setIntensity(0,2);
 M2.clearDisplay(0);
 drawScore(0);
 sound_count = 0;

 
 //setup button pin to be an output
 pinMode( BUTTON, OUTPUT);
 initGame();
}

//set variables to initial values
void initGame(){
  M.setRow(0, 3, B01000000);
  G.score = 0;
  G.active = false;
  G.sound_playing = false;
  
  Bird b;
  b.lastPos = 0;
  b.yPos = 3.0;
  b.yVel = 0.0;
  
  G.bird = b;
  G.pipe = makePipe();
  T.every(30, buttonPressed);
  
  
}

void startGame(){
  G.bird.timerId = T.every(10, moveBird );
  T.after(1000, startPipe);
}

//start an inividual pipe's movement across the screen
void startPipe(){
  G.pipe.timerId = T.every(250, movePipe );
  drawScore(0);
}

Pipe makePipe(){
  randomSeed(analogRead(0));
  Pipe p;
  p.xPos = 8;
  p.bottom = (int) random(1, 6);
  p.top = 6 - p.bottom;
  p.timerId = -1;
  return p;
  
}

void drawScore(int score){
 int tens = (score / 10) % 10;
 int ones = score % 10;
 int nones = ones;
 
 //display one digit
 if (tens == 0){
     for (int i = 1; i < 6; i ++)
     {
        M2.setRow(0, i, digit[ones][i-1] << 2);
     }
 }
 //display two digits
 else{
   for (int i = 1; i < 6; i ++)
   {
    M2.setRow(0, i, digit[tens][i-1] << 5 | digit[ones][i-1] << 1);
   } 
 }
}

void movePipe(){
  //when the bird clears the pipe
  if (G.pipe.xPos == 0){
     playSound(1);
     G.score ++;
     drawScore(G.score);
   }
   
  if (G.pipe.xPos > 0){
    G.pipe.xPos -= 1;
  }
  else if (G.pipe.xPos < 0){
    G.pipe = makePipe();
  }
  else{
    G.pipe.xPos -= 1;
  }

  drawPipe();
}


void moveBird(){

  //adjust bird velocity for gravity and jump
  G.bird.yVel = (G.bird.yVel > gravity * max_grav_multiple) ? G.bird.yVel : G.bird.yVel + gravity;  
  G.bird.lastPos = (int) G.bird.yPos; 
  G.bird.yPos += G.bird.yVel;
  
  
  drawBird();
  
  //check for collision
  int gridLoc = 7 - ((int) G.bird.yPos);
  if ( (int) G.bird.yPos > 7 || (int) G.bird.yPos < 0){
     gameOver();
  }
  else if( G.pipe.xPos == 1 || G.pipe.xPos == 2 ){
     if ( gridLoc <= G.pipe.bottom -1 || gridLoc >= 8 - G.pipe.top){
       gameOver();
     }
  }
}

void drawBird(){
  if (G.bird.lastPos != (int) G.bird.yPos){
    M.setLed(0, G.bird.lastPos, 1, false);
  }
  M.setLed(0, (int) G.bird.yPos, 1, true);
  
}


void buttonPressed(){
  boolean clicked = digitalRead(BUTTON);  
  if (clicked) {
      if (!G.active){
        G.active = true;
        startGame();
      }
      //set positive upward velocity
      G.bird.yVel = flap_power;

      //only play sound once every 5 iterations at most
      if (sound_count > 5){
        playSound(0);
        sound_count = 0;
      }
  }
  sound_count ++;
}


void playSound(int sound_id){
  //jump sound
  if (sound_id == 0 && !G.sound_playing)
  {
    int low = 73;
    int mid = 147;
    int high = 294;
    jump1();
    T.after(50, jump2);
  }
  //coin sound
  else if (sound_id == 1){
    G.sound_playing = true;
    int B = 1976; // define note sound
    int E = 2637;
    coin1();
    T.after(100, coin2);
  }
  //death sound
  else if (sound_id == 2){
    tone(speaker, 87, 300);
    delay(400);
    tone(speaker, 55, 400);
  }
  else{
    noTone(speaker);
  }  
}

void coin1(){tone(speaker, 1976, 75);}//B
void coin2(){tone(speaker, 2637, 300); G.sound_playing = false;}//E

//chains sounds using timers to ensure other code continues executing
void jump1(){tone(speaker, 73, 62.5);} // low
void jump2(){tone(speaker, 147, 250); T.after(30, jump3);} // mid
void jump3(){tone(speaker, 294, 62.5);} // high

void death1(){tone(speaker, 87, 300);}
void death2(){tone(speaker, 55, 400);}

  
void drawPipe(){
  
    byte b = B11111111;
    b = b << ( G.pipe.bottom + G.pipe.top );
    b = b >> (G.pipe.top);
    b = ~b;
    
    int toErase =  (G.pipe.xPos == 7) ? 0 : G.pipe.xPos + 1;
    M.setColumn(0,G.pipe.xPos, b);
    M.setColumn(0,G.pipe.xPos-1, b);
    M.setColumn(0,toErase, B00000000);
}

void gameOver(){
  G.active = false;
  T = Timer();
  playSound(2);

  //death animation
  for (int i = 0; i < 16; i++){
    if (i < 8){
      M.setRow(0, i, B11111111);
    }
    else if ( i == 11){
       M.setRow(0, i%8, B01000000);
    }
    else{
      M.setRow(0, i%8, B00000000);
    }
    delay(80);
  }
  //prepare the next game
  initGame();
}

//continously update timer
void loop() {
  T.update();
}
  
