#include <stdint.h>
#include <stdbool.h>

extern uint8_t _erodata[];
extern uint8_t _data[];
extern uint8_t _edata[];
extern uint8_t _sdata[];
extern uint8_t _esdata[];
extern uint8_t _bss[];
extern uint8_t _ebss[];

// Adapted from https://stackoverflow.com/questions/58947716/how-to-interact-with-risc-v-csrs-by-using-gcc-c-code
__attribute__((always_inline)) inline uint32_t csr_mstatus_read(void){
    uint32_t result;
    asm volatile ("csrr %0, mstatus" : "=r"(result));
    return result;
}

__attribute__((always_inline)) inline void csr_mstatus_write(uint32_t val){
    asm volatile ("csrw mstatus, %0" : : "r"(val));
}

__attribute__((always_inline)) inline void csr_write_mie(uint32_t val){
    asm volatile ("csrw mie, %0" : : "r"(val));
}

__attribute__((always_inline)) inline void csr_enable_interrupts(void){
    asm volatile ("csrsi mstatus, 0x8");
}

__attribute__((always_inline)) inline void csr_disable_interrupts(void){
    asm volatile ("csrci mstatus, 0x8");
}

__attribute__((always_inline)) inline uint32_t csr_mcause_read(void){
    uint32_t result;
    asm volatile ("csrr %0, mcause" : "=r"(result));
    return result;
}


#define MTIME_LOW       (*((volatile uint32_t *)0x40000008))
#define MTIME_HIGH      (*((volatile uint32_t *)0x4000000C))
#define MTIMECMP_LOW    (*((volatile uint32_t *)0x40000010))
#define MTIMECMP_HIGH   (*((volatile uint32_t *)0x40000014))
#define CONTROLLER      (*((volatile uint32_t *)0x40000018))
#define CMIE     (*((volatile uint32_t *)0x40000000))
#define CMIP    (*((volatile uint32_t *)0x40000004))


typedef struct{
    uint32_t DPalette : 2;
    uint32_t DXOffset : 10;
    uint32_t DYOffset : 9;
    uint32_t DWidth : 5;
    uint32_t DHeight : 5;
    uint32_t DReserved : 1;
} LSControl, *LSControlRef;

typedef struct{
    uint32_t DPalette : 2;
    uint32_t DXOffset : 10;
    uint32_t DYOffset : 10;
    uint32_t DZ : 3;
    uint32_t DReserved : 7;
} BControl, *BControlRef;

void init(void){
    uint8_t *Source = _erodata;
    uint8_t *Base = _data < _sdata ? _data : _sdata;
    uint8_t *End = _edata > _esdata ? _edata : _esdata;

    while(Base < End){
        *Base++ = *Source++;
    }
    Base = _bss;
    End = _ebss;
    while(Base < End){
        *Base++ = 0;
    }

    csr_write_mie(0x888);       // Enable all interrupt soruces
    csr_enable_interrupts();// Global interrupt enable
    CMIE=0x4;
    MTIMECMP_LOW = 1;
    MTIMECMP_HIGH = 0;
}
extern volatile char *VIDEO_MEMORY;


extern volatile int global;
extern volatile uint32_t controller_status;
extern volatile int getout;
extern volatile char game[6][7];
extern volatile char piece;
extern volatile bool result;
extern volatile char *gp;
extern volatile int pos;
extern volatile uint32_t black;
volatile LSControl *SPRITE_CONTROL=(volatile LSControl *)(0x500FF114);
extern volatile uint8_t *BLUEI;
extern volatile uint8_t *YELLOWI;
extern volatile uint8_t *BLK;
volatile BControl *CONTROL=(volatile BControl *)(0x500FF100);
extern volatile int x_pos;
extern volatile int y_pos;
extern volatile int round;
extern volatile int row;
extern volatile int col;
extern volatile char dir;



void c_interrupt_handler(void){
   
   
   //identify cause
      uint32_t cause = csr_mcause_read();
   
   if(cause & 0x4){
     
      uint64_t NewCompare = (((uint64_t)MTIMECMP_HIGH)<<32) | MTIMECMP_LOW;
 
            
      if(result==true){
         
            for(int w=0;w<4;w++){
               if(dir=='x'){
                  for(int i=0;i<160;i++){
                     for(int j=0;j<32;j++){
                        BLK[i*512+j]=0xEB;
                     }
                  }
                     CONTROL[3].DZ=1;
                     CONTROL[3].DXOffset=632+col*40;
                     CONTROL[3].DYOffset=340+row*40;
               }
               else if(dir=='a'){
                  for(int i=0;i<40;i++){
                     for(int j=0;j<160;j++){
                        BLK[i*512+j]=0xEB;
                     }
                  }
                     CONTROL[3].DZ=1;
                     CONTROL[3].DXOffset=632+col*40-120;
                     CONTROL[3].DYOffset=340+row*40;
               }
               else if(dir=='q'){
                  for(int s=0;s<4;s++){
                     for(int i=s*40;i<s*40+40;i++){
                        for(int j=s*40;j<s*40+40;j++){
                           BLK[i*512+j]=0xEB;
                        }
                     }
                  }
                     CONTROL[3].DZ=1;
                     CONTROL[3].DXOffset=632+col*40-120;
                     CONTROL[3].DYOffset=340+row*40-120;
               }
               else if(dir=='z'){
                  for(int s=0;s<4;s++){
                     for(int i=s*40;i<s*40+40;i++){
                        for(int j=(3-s)*40;j<(3-s)*40+40;j++){
                           BLK[i*512+j]=0xEB;
                        }
                     }
                  }
                     CONTROL[3].DZ=1;
                     CONTROL[3].DXOffset=632+col*40-120;
                     CONTROL[3].DYOffset=340+row*40;
               }
            }
      
            uint64_t mt = (((uint64_t)MTIME_HIGH)<<32) | MTIME_LOW;
            uint64_t time = mt+10000;
            while(time>mt){
               mt=(((uint64_t)MTIME_HIGH)<<32) | MTIME_LOW;
            }
         NewCompare=mt+3000;
         
         CONTROL[3].DXOffset=0;
         CONTROL[3].DYOffset=0;
         
      }
      
      NewCompare += 100;
      MTIMECMP_HIGH = NewCompare>>32;
      MTIMECMP_LOW = NewCompare;
      global++;
      controller_status = CONTROLLER;
    
   }
   else if(cause & 0xb){
      
      for (int i=0;i<42;i++)
      {
         gp[i]='0';
      }
      
      for(int j=0;j<20;j++)
      {
         VIDEO_MEMORY[j]=' ';
      }
   //reset in text mode
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
      for(int q=1;q<7;q++)
      {
         VIDEO_MEMORY[q*64+12]=' ';
      }
      
      //reset video mode:
      SPRITE_CONTROL[0].DXOffset = 304;
      SPRITE_CONTROL[0].DYOffset = 64;
      SPRITE_CONTROL[1].DXOffset =0;
      SPRITE_CONTROL[1].DYOffset =0;
      
      for(int i=0;i<512;i++){
         for(int j=0; j<288;j++){
            BLUEI[j*512+i]=black;
            YELLOWI[j*512+i]=black;
            BLK[j*512+i]=black;
         }
      }
 

      x_pos=304;
      y_pos=64;
      round=0;
      
      result=false;
      piece='X';
      pos=6;
      
      
      global++;
      getout=0;
      controller_status = CONTROLLER;
      CMIP=0xF;
   }
   
      

}

