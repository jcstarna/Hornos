//control de lamparas por angulo de fase.

#include <16f873.h>                          // Definiciones del PIC 16F876A
#fuses XT,NOWDT,NOPROTECT,NOLVP,PUT,BROWNOUT  // 
#use delay(clock=4000000)                     // Oscilador a 4 Mhz


#pragma use fast_io(A)
#pragma use fast_io(B)
#pragma use fast_io(C)

//    PINES   /////////////////////////////////////////////////////////////////
#define pLed1     PIN_A1
#define pLed2     PIN_A2
#define pLed3     PIN_A3
#define pLed4     PIN_A4

#define pSensor1  PIN_B7                 //sensor de botella 1
#define pSensor2  PIN_B6                 //sensor de botella 2
#define pContactor   PIN_B5
#define pTanque   PIN_B4
#define pPico2    PIN_B3
#define pPico1    PIN_B2
#define pMang2    PIN_B1
#define pMang1    PIN_B0

#define pEV1      PIN_C4                 //Electrovalvula botella 1
#define pEV2      PIN_C5                 //Electrovalvula botella 1

#define pE_lcd    PIN_D0                 //sensor de botella 2



// VARIABLES EN RAM ///////////////////////////////////////////////////////////

int1 f1_seg;                           // Flag de un segundo
int1 fcruce0;
int  rTimer1;                          // Tiempo de encendido V1
int  rTemp;                     	
int  rPulsos;                          // 
int  rTimeOff_V2;                      // 
int1 prevIn1;                          // 
int1 prevIn2;                          // 
INT  rDelay;                           // 
int  rLeds;                            // Imagen de los leds



// Declaración de Funciones ///////////////////////////////////////////////////


// INTERRUPCIONES /////////////////////////////////////////////////////////////


}
#int_RTCC
void RTCC_isr() {                      // Interrupción Timer 0
   rDelay++;


}

#int_TIMER1
void T1_isr() {                      // Interrupción Timer 1
      IF(input(PIN_B7))
      output_low(PIN_B7);
   ELSE
      OUTPUT_HIGH(PIN_B7);


}

#INT_EXT
void ext_isr(){
     if(input(PIN_B0)){
         ext_int_edge(H_TO_L);
         fcruce0=1;
     }
     ELSE{
         ext_int_edge(L_TO_H);
         fcruce0=1;
     }
     set_timer1(0);
}


#INT_TIMER2                            //Interrupcion por timer 2
void TIMER2_isr() {                    //cada 1ms para control de tiempo
}


// Desarrollo de Funciones ////////////////////////////////////////////////////





//,,,,,,,,,,,,Programa Principal /////////////////////////////////////////////////////////

void main() {

//++++++++++++++++++Configura puertos analogicos++++++++++++++++++++++++++++++
   setup_adc(  ADC_CLOCK_INTERNAL  );
   setup_adc_ports( RA0_ANALOG );                 // Ver que puertos van a ser analogicos
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_32);
   setup_timer_1(T1_INTERNAL|T1_DIV_8);

   set_tris_a (0b00000011);
   set_tris_b (0b00000001);
   set_tris_c (0b11000000);
   rDelay = 0;
   rLeds  = 1;


   ext_int_edge(H_TO_L);			//selecciono interrupcion por flanco descendente
   //enable_interrupts(int_RTCC);                  // Habilita Interrupción TIMER0
   enable_interrupts(int_ext);                   // Habilita Interrupción RB0

  //enable_interrupts(int_TIMER1);                // Habilita Interrupción TIMER1
  // enable_interrupts(int_TIMER2);                // Habilita Interrupción TIMER2
   enable_interrupts(global);                    // Habilita interrupciones

   output_b(0);

   do {

      rTimer1=get_timer1();
        
      if(fcruce0){                               // Si hay disparo pendiente
         if(rTimer1>=rTemp){
         fcruce0=0;
         do {
         output_high(PIN_C4);
         delay_us(10);
         output_low(PIN_C4);
         delay_us(10);
         }while(rPulsos++<20);
         rPulsos=0;
         rTemp=read_adc();
        }
      }

   } while (TRUE);

}



