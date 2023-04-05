#include <stdint.h>
#include <stdbool.h>
#include "ImageData.h"


//func prototype
bool south(int,int,char[6][7],char,int);
bool west(int,int,char[6][7],char,int);
bool Nwest(int,int,char[6][7],char,int);
bool Swest(int,int,char[6][7],char,int);


//define globals so I can modify them in interrupt handler
volatile int global = 42;
volatile uint32_t controller_status = 0;
volatile int getout=1;
//chess pieces
volatile char piece='X';
volatile int round=0;
volatile bool result=false;
//store pieces location on the board
char game[6][7];
volatile char *gp=(char *)game;
volatile int pos = 6;
volatile int x_pos = 304;
volatile int y_pos = 64;
volatile char dir;
//transparent color:
volatile uint32_t black=0x0;
volatile int row;
volatile int col;




//print offsets
typedef struct{
    uint32_t DPalette : 2;
    uint32_t DXOffset : 10;
    uint32_t DYOffset : 10;
    uint32_t DZ : 3;
    uint32_t DReserved : 7;
} BGControl, *BGControlRef;

typedef struct{
    uint32_t DPalette : 2;
    uint32_t DXOffset : 10;
    uint32_t DYOffset : 9;
    uint32_t DWidth : 5;
    uint32_t DHeight : 5;
    uint32_t DReserved : 1;
} SLargeSpriteControl, *SLargeSpriteControlRef;

volatile BGControl *BG_CONTROL = (volatile BGControl *)(0x500FF100);
volatile char *VIDEO_MEMORY = (volatile char *)(0x50000000 + 0xFE800);
volatile uint32_t *VIDEO_MODE = (volatile uint32_t *)(0x500FF414);
volatile SColor *BGP = (volatile SColor *)(0x500FC000);
//board -> image0
volatile uint8_t *BGI = (volatile uint8_t *)(0x50000000);
//blue piece -> image1
volatile uint8_t *BLUEI = (volatile uint8_t *)(0x50024000);
//yellow piece -> image2
volatile uint8_t *YELLOWI = (volatile uint8_t *)(0x50048000);

volatile uint8_t *BLK = (volatile uint8_t *)(0x5006C000);


//blue sprite:
volatile uint8_t *B_SPRITE = (volatile uint8_t *)(0x500B4000);
volatile uint8_t *Y_SPRITE = (volatile uint8_t *)(0x500B5000);
//control 2:
volatile uint8_t *SPRITE1 = (volatile uint8_t *)(0x500B6000);

volatile SLargeSpriteControl *LARGE_SPRITE_CONTROL = (volatile SLargeSpriteControl *)(0x500FF114);
volatile SColor *SPRITE_PALETTES = (volatile SColor *)(0x500FD000);




int main() {
    int a = 4;
    int b = 12;
    int last_global = 42;
      
   //initial video mode board
   for(int i=0;i<236;i++){
      BGP[i]=Palette[i];
      SPRITE_PALETTES[i]=Palette[i];
   }

   int c=0;
   for(int i=0;i<240;i++){
      for(int j=0;j<280;j++){
         BGI[i*512+j]=BoardImage[c];
         c++;
      }
   }
   BG_CONTROL[0].DPalette=0;
   BG_CONTROL[0].DXOffset=628;
   BG_CONTROL[0].DYOffset=336;
   BG_CONTROL[0].DZ=2;
  
   
   //sprites:
   c=0;
   for(int i=0;i<32;i++){
      for(int j=0;j<32;j++){
         B_SPRITE[i*64+j]=BluePieceImage[c];
         c++;
      }
   }
   LARGE_SPRITE_CONTROL[0].DPalette=0;
   LARGE_SPRITE_CONTROL[0].DHeight = 0;
   LARGE_SPRITE_CONTROL[0].DWidth = 0;
   LARGE_SPRITE_CONTROL[0].DXOffset = 304;
   LARGE_SPRITE_CONTROL[0].DYOffset = 64;
   c=0;
   for(int i=0;i<32;i++){
      for(int j=0;j<32;j++){
         Y_SPRITE[i*64+j]=YellowPieceImage[c];
         c++;
      }
   }
   LARGE_SPRITE_CONTROL[1].DPalette=0;
   LARGE_SPRITE_CONTROL[1].DHeight = 0;
   LARGE_SPRITE_CONTROL[1].DWidth = 0;
   LARGE_SPRITE_CONTROL[1].DXOffset =0;
   LARGE_SPRITE_CONTROL[1].DYOffset =0;
   //video mode here
   *VIDEO_MODE=1;
   
   
   

   
   
  
//print broad in text mode
   VIDEO_MEMORY[6] = 'X';

   for(int i=0;i<6;i++)
   {
      for(int j=0; j<12;j++)
      {
         VIDEO_MEMORY[64*(i+1)+j] = ' ';
         VIDEO_MEMORY[64*(i+1)+j+1] = 'I';
         j++;
      }
   }
   
   
   //initialize game 2d array
   for (int i=0;i<6;i++)
   {
      for(int j=0;j<7;j++)
      {
         game[i][j]='0';
      }
   }
   
   
    while (1) {
        int c = a + b + global;


        if(global != last_global){
            if(controller_status){
               //left
                if(controller_status & 0x1)
                {
                    if(pos>=2)
                    {
                       //text mode:
                       VIDEO_MEMORY[pos] = ' ';
                       pos-=2;
                       VIDEO_MEMORY[pos] = piece;
                       
                       //video mode:
                       x_pos-=40;
                       LARGE_SPRITE_CONTROL[round].DXOffset = x_pos;
                       
                       //infinite loop to secure only one operation
                       while(controller_status!=0x0){}
                    }
                }
               //right
                if(controller_status & 0x8)
                {
                   if(pos<=10)
                   {
                      //text mode:
                      VIDEO_MEMORY[pos] = ' ';
                      pos+=2;
                      VIDEO_MEMORY[pos] = piece;
                      
                      //video mode:
                      x_pos+=40;
                      LARGE_SPRITE_CONTROL[round].DXOffset = x_pos;
                      
                      while(controller_status!=0x0){}
                   }
                }
               //drop the piece,store in array,& win check
                if(controller_status & 0x10)
                {
                   for(int i=5;i>=0;i--)
                   {
                      if(game[i][pos/2]=='0')
                      {
                         //assign value to game array
                         game[i][pos/2]=piece;
                         row=i;
                         col=(pos/2);
                         
                         //print text
                         VIDEO_MEMORY[64*(i+1)+pos] = piece;
                         
                         //video mode:
                         int c=0;
                         if(round==0){
                            for(int q=(51+(i*40));q<(51+i*40+32);q++){
                               for(int j=(121+(pos/2)*40);j<(121+32+((pos/2)*40));j++){
                                  BLUEI[q*512+j]=BluePieceImage[c];
                                  c++;
                               }
                            }
                         }
                         else{
                            for(int q=(51+(i*40));q<(51+i*40+32);q++){
                               for(int j=(121+(pos/2)*40);j<(121+32+((pos/2)*40));j++){
                                  YELLOWI[q*512+j]=YellowPieceImage[c];
                                  c++;
                               }
                            }
                         }
                        
                         LARGE_SPRITE_CONTROL[round].DXOffset = 0;
                         LARGE_SPRITE_CONTROL[round].DYOffset = 0;
                         
                           BG_CONTROL[1].DPalette=0;
                           BG_CONTROL[1].DXOffset=512;
                           BG_CONTROL[1].DYOffset=288;
                           BG_CONTROL[1].DZ=0;
                           BG_CONTROL[2].DPalette=0;
                           BG_CONTROL[2].DXOffset=512;
                           BG_CONTROL[2].DYOffset=288;
                           BG_CONTROL[2].DZ=0;
                         
                         //switch piece text mode:
                         if(piece=='X'){
                            piece='O';
                         }
                         else if(piece=='O'){
                            piece='X';
                         }
                         VIDEO_MEMORY[pos]=piece;
                         
                         //switch piece video mode:
                         if(round==0){
                            round=1;
                         }else{
                            round=0;
                         }
                         
                         LARGE_SPRITE_CONTROL[round].DXOffset = x_pos;
                         LARGE_SPRITE_CONTROL[round].DYOffset = 64;

                         
                         i=-1;
                      }
                   }
                   while(controller_status!=0x0){}
                   
                   //win check
                   if(piece=='X'){
                      piece='O';
                   }else{
                      piece='X';
                   }
                        if(south(row,col,game,piece,0)){
                           result=true;
                           dir='x';
                        }
                        if(game[row][col]==game[row][col+1] && result==false){
                           int t1=row;
                           int t2=col;
                           while(game[row][col]==game[row][col+1]){
                              col+=1;
                           }
                           if(west(row,col,game,piece,0)){
                              result=true;
                              dir='a';
                           }
                           else{
                              row=t1;
                              col=t2;
                           }
                        }
                        if(west(row,col,game,piece,0) && result==false){
                           result=true;
                           dir='a';
                        }
                        if(game[row][col]==game[row+1][col+1] && result==false){
                           int t1=row;
                           int t2=col;
                           while(game[row][col]==game[row+1][col+1]){
                              row+=1;
                              col+=1;
                           }
                           if(Nwest(row,col,game,piece,0)){
                              result=true;
                              dir='q';
                           }else{
                              row=t1;
                              col=t2;
                           }
                        }
                        if(Nwest(row,col,game,piece,0) && result==false){
                           result=true;
                           dir='q';
                        }
                        if(game[row][col]==game[row-1][col+1] && result==false){
                           int t1=row;
                           int t2=col;
                           while(game[row][col]==game[row-1][col+1]){
                              row-=1;
                              col+=1;
                           }
                           if(Swest(row,col,game,piece,0)){
                              result=true;
                              dir='z';
                           }else{
                              row=t1;
                              col=t2;
                           }
                        }
                        if(Swest(row,col,game,piece,0) && result==false){
                           result=true;
                           dir='z';
                        }
                   if(piece=='X'){
                      piece='O';
                   }else{
                      piece='X';
                   }
                   

                      
                  if(result==true){
                     VIDEO_MEMORY[pos]=' ';
                     
                     
                     //infinity loop to pause the game until
                     //cmd is hit
                     while(getout){}
                     getout=1;
                   }
                   
                   
                }
               
            }
         
           
            last_global = global;
        }
    }
    return 0;
}


   //detecting win from game[r][c] to the south see if there
   //are 4 strike by recursion
bool south(int r, int c,char g[6][7],char type,int count){
   
   if(count==4)
   {
      return true;
   }
   
   if(r>=0){
      if(g[r][c]==type){
         return south((r+1),c,g,type,(count+1));
      }else{
         return false;
      }
      
   }else{
      return false;
   }
   
}

//test win strike pieces from game[r][c] to the west
bool west(int r, int c,char g[6][7],char type,int count){
      
   if(count==4)
   {
      return true;
   }
   
   if(c>=0){
      if(g[r][c]==type){
         return west(r,(c-1),g,type,(count+1));
      }else{
         return false;
      }
      
   }else{
      return false;
   }
   
}


//northwest
bool Nwest(int r, int c,char g[6][7],char type,int count){
      
   if(count==4)
   {
      return true;
   }
   
   if((r>=0)&&(c>=0)){
      if(g[r][c]==type){
         return Nwest((r-1),(c-1),g,type,(count+1));
      }else{
         return false;
      }
      
   }else{
      return false;
   }
   
}


//southwest
bool Swest(int r, int c,char g[6][7],char type,int count){
      
   if(count==4)
   {
      return true;
   }
   
   if((r<=5)&&(c>=0)){
      if(g[r][c]==type){
         return Swest((r+1),(c-1),g,type,(count+1));
      }else{
         return false;
      }
      
   }else{
      return false;
   }
   
}



