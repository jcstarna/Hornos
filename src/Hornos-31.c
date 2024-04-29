//control de lamparas por angulo de fase.
//Version 3 control de disparo por optoacoplador
//  03/07/08  agregado de arranque del modulo pwm para disparo de triac
//  Revision 2.1
//  08/07/08  implementacion de direccion para comunicacion
//  Revision 
//  07/10/08  Se saca interrupcion por comparacion y se precarga el timer1 y se apaga la
//             interrupcion cuando desborda el timer 1
//  Revision 
//  04/01/09  Implementacin de escritura del Set point de potencia desde PC
// 
//  Revision 
//  04/01/10  Modificacion del maximo tiempo de disparo para lograr apagado total
//             de la lamapara
//  Revison
//  07-11-12    Modificacion de micro a 16F883
//
//  Revision
//  19-06-13    Modificacion de disparo por optoacoplador
//             Agregado de TI para medicion de corriente
/*  Revision
    15/01/14   Agregado de comunicacion modbus
*/

#include <16f886.h>                          // Definiciones del PIC 16F873


#byte   OPTION_REG = 0x081
#bit    PS0 = OPTION_REG.0
#bit    PS1 = OPTION_REG.1
#bit    PS2 = OPTION_REG.2
#bit    PSA = OPTION_REG.3
#bit    T0SE = OPTION_REG.4
#bit    T0CS = OPTION_REG.5
#bit    INTEDG = OPTION_REG.6
#bit    RBPU = OPTION_REG.7

#byte   TXSTA_REG = 0x098
#bit    TRMT = TXSTA_REG.1

#DEVICE ADC=10
#fuses XT,NOWDT,NOPROTECT,NOLVP,NOBROWNOUT,MCLR,NODEBUG,PUT
#use delay(clock=4000000)                     // Oscilador a 4 Mhz
//#use rs232(baud=19200, xmit=PIN_C6, rcv=PIN_C7, enable=PIN_c3, ERRORS)// RS232 Estándar , enable=PIN_c3
#priority EXT,TIMER1,RTCC,RDA,TBE
#pragma use fast_io(A)
#pragma use fast_io(B)
#pragma use fast_io(C)

#use rs232(baud=57600, xmit=PIN_C6, rcv=PIN_C7, enable=PIN_c3, ERRORS)// RS232 Estándar , enable=PIN_c3

#define pTiristor_On output_high(pin_b1)
#define pTiristor_Off output_low(pin_b1)

// VARIABLES EN RAM ///////////////////////////////////////////////////////////

// Variables PID
//tabla para lectura desde PC//
int16 COM=0;               //Dir: 0 Comando desde PC
int16 EST=0;               //Dir: 1 Estado de la placa
int16 SP=0;                //Dir: 2 Set point de potencia
int16 PV=0;                //Dir: 3 Valor actual de potencia
int16 TI=0;                //Dir: 4 Medicion del TI
int16 Error=0;             //Dir: 5 Registro para cargar errores
int16 SP_Calc=1023;        //Dir: 6 Sp resultante de la comparacion del pv con el sp deseado(lectura Pote)
signed int16 acuErr=10;    //Dir: 7 Error en para el calculo de PID
INT16 TEMP=0;              //Dir: 8 Temporal
//fin tabla//
//tabla para escritura desde PC//
int16 REM_SP=0;            //Dir: 6 Set point remoto (desde PC)
#bit LR        = REM_SP.15

//DEFINICIONES DE LOS BITS DE ESTADO//
#bit fEstHab   =Est.0


//DEFINICIONES DE LOS BITS DE ERROR//
#bit fELamp    =Error.4
#bit fECom     =Error.3
#bit fETriac   =Error.2
#bit fESp      =Error.1
#bit fECero    =Error.0

//TIEMPOS EN LAS FALLAS
#define kT_falla_SP     100
#define kT_falla_Cero   50


INT  rDelay=10;            // Para lograr un tiempo determinado con la int del TMR0
int  rLeds=0;              // Imagen de los leds
int1 ctrlPot=0;            //define desde donde se controla la potencia 0=desde pote, 1=desde comunicacion
int1 fDisparo=1;           //esta disparando el triac
int1 fPID=0;
int1 fTX=0;
INT1 fHab=0;               //habilitacion de disparo.
int1 T2=0;                 //salto el T2
int  rTarea=1;             //Maquina de estados
INT16  rTDisp=0;

#include "ModBus.h"
#include "Modbus.c"

//Variables para deteccion de errores
int  rT_falla_SP  =    kT_falla_SP;      
int  rT_falla_Cero=    kT_falla_Cero; 

// Variables comunicacion
int1 fEscOK=0;       //el mensaje recibido es para este esclavo
int  nEsclavo=0;     //Direccion de la placa
int  rTinactiv=2;    //Tiempo para asegurar inactividad en la red
int  rCHKSUM=0;      //registro de checksum

// Declaración de Funciones ///////////////////////////////////////////////////
void Control(void);                    //Calcula set point, error, determina SP_Calc.
INT16  PID(int16 SP,INT16 PV, KP, KI,signed int16 *aError);
                            


// INTERRUPCIONES /////////////////////////////////////////////////////////////
//++++++++++++++++++++++++++++++++++++++
//  RECEPCION SERIE.
//++++++++++++++++++++++++++++++++++++++
#int_TIMER2
void TMR2_isr(){
int16 CRC=0;
int16 CRC_Leido=0;
t2=1;
/*
   IF(fParami){                        //Calculo checksum de dato recibido
         CRC=CRC_Calc(rxbuff,rxpuntero-3); //
         CRC_Leido= make16(rxBuff[rxpuntero-2],rxBuff[rxpuntero-1]);
         if(CRC_Leido==CRC)flagcommand=1;
         setup_timer_2(T2_DISABLED,0,1);
         rxpuntero=0;
         }
   else{
         inicbuffRX();
         rxpuntero=0;
         }
         setup_timer_2(T2_DISABLED,0,1);
*/
}

#int_rda
void serial_isr() {                    // Interrupción recepción serie USART
rcvchar=0x00;                        // Inicializo caracter recibido
   if(kbhit()){                           // Si hay algo pendiente de recibir ...
      rcvchar=getc();                     // lo descargo y ...
      ModBusRX(rcvchar);                // lo añado al buffer y ... 
   }  
   setup_timer_2(T2_DIV_BY_16,0xF0,5);
}

//++++++++++++++++++++++++++++++++++++++
//  TRANSMISION SERIE.
//++++++++++++++++++++++++++++++++++++++
#INT_TBE
void serial_tx(){
   ModBusTX ();
}

#int_RTCC
void RTCC_isr() {                // Interrupción Timer 0
   if(rDelay--==0){
      rDelay=3;
      if (rTarea++>3)rTarea=1;
      fPID=1;
      }
//Deteccion de errores
//falla falta de cruce por cero
   if(rT_falla_Cero--==0 & !fECero)fECero=1;
//falla no se puede alcanzar el sp
   if(rT_falla_SP--==0 & !fESp)fESp=1;
}


///////// INTERRUPCION POR CRUCE POR CERO ////////////

#INT_EXT
void ext_isr(){
int16 rTemp1=0;
     pTiristor_off;   //prueba 4/08/09
     output_toggle(pin_b4);
     rT_falla_Cero=    kT_falla_Cero;
     fECero=0;
     rTemp1=65535-(SP_Calc);
     if (rTemp1 > 64335)rTemp1=64335;     //maximo anterior (version 2.2 64035)
     if (rTemp1 < 57700)rTemp1=57700;     //minimo    65535-58535=

     set_timer1(rTemp1);
     #ASM bcf  0xd,0
          bcf  0xc,2
     #ENDASM  
     clear_interrupt(int_timer1);
     set_timer1(rTemp1);
     enable_interrupts(int_timer1);             // Habilito la interrpción INT_CCP2
     fDisparo=0;
  /*   if (fTX){ enable_interrupts(int_tbe);
               fTX=0;
    }   */ 
}
///////// INTERRUPCION IGUALACION DE CCP_2 Y TIMER1 ////////////

#int_TIMER1   //Timer 1 overflow
void TIMER1_int(){
int16 rTempT1;
   clear_interrupt(int_timer1);
   if (fDisparo){//ya esta disparando
      pTiristor_off;                   //apago pwm
      enable_interrupts(int_ext);      //habilito interrupcion por cruce por cero
      fDisparo=0;
      disable_interrupts(int_timer1);
   }
   else{ // tiene que empezar a disparar
         if(fHab) pTiristor_on;                 //disparo
      //   rTempT1=65535-rTdisp;             //SETEO TIEMPO DE DISPARO
         set_timer1 (64535);//(rTempT1);
         fDisparo=1;                   //marco que estoy disparando
         enable_interrupts(int_timer1);
/*     if (fTX){ enable_interrupts(int_tbe);
               fTX=0;
    }  */        
   }
}

// Desarrollo de Funciones ////////////////////////////////////////////////////




//,,,,,,,,,,,,Programa Principal /////////////////////////////////////////////////////////

void main() {
   int16 rTemp=0;
   delay_ms(500);
//++++++++++++++++++Configura puertos analogicos++++++++++++++++++++++++++++++
   setup_adc(  ADC_CLOCK_INTERNAL  );
   setup_adc_ports( sAN0|sAN1|sAN2 );                 // Ver que puertos van a ser analogicos
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_128);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);//
   setup_comparator(NC_NC_NC_NC);            //no comparadores
   setup_timer_2(T2_DIV_BY_4,0xF0,5);        //control de inactividad del bus

   
   //Seteos puertos  
   set_tris_a (0b00000111);
   set_tris_b (0b00000001);
   output_c(0);
   set_tris_c (0b10010000);
;
   rDelay = 0;
   rLeds  = 1;
   //leo nro de esclavo de la placa
   ModbusAddress =2;// swap(input_b() & 0b11110000);

   enable_interrupts(int_rda);                  // Habilita Interrupción RDA
   ext_int_edge(L_TO_H);
   enable_interrupts(int_RTCC);                 // Habilita Interrupción TIMER0
   enable_interrupts(int_ext);                  // Habilita Interrupción RB0, PASO DE BAJO A ALTO
   enable_interrupts(int_TIMER2); 
   setup_timer_2(T2_DISABLED,0,1);
   
   set_timer1(0);
   output_b(0);

   fPID=1;
   rTarea=1;
   set_adc_channel(0);
   clear_interrupt(int_timer1);
   disable_interrupts(INT_TIMER1);
   clear_interrupt(int_rda);
   clear_interrupt(int_tbe);
   enable_interrupts(global);                   // Habilita interrupciones
   SP_Calc=10230;
   ctrlPot=1;  //control por red
   fHab=1;     //habilitar disparo
   fEstHab=1;  //indicar que esta habilitar
   t2=0;
   /////////BUCLE PRINCIPAL//////////
   do {
     if(fPID){
               switch (rTarea){
                     case 1://leo potenciometro
                            if (!ctrlPot) SP=read_adc();
                            set_adc_channel(1);
                            break;
                     case 2://leo PV       
                            PV=read_adc();
                            set_adc_channel(2);
                            
                            //calculo diferencia para resetear tiempo de error
                            if (sp>pv)
                                 if((SP-PV)<60)rT_falla_SP=kT_falla_SP;    //reseteo contador
                            else
                                 if((PV-SP)<60)rT_falla_SP=kT_falla_SP;    //reseteo contador
                            break;
                     case 3://Leo TI
                           TI=read_adc();
                           set_adc_channel(0);
                            break;
               }
               fPID=0;
               TEMP=PID (SP, PV,2,2,&acuErr);    //esta salida  varia entre 0 y 1023
               SP_Calc=10000-(TEMP*10); 
               rTemp=(SP_Calc+1300);
               if (rTemp<9000)rTdisp=1500; //si el disparo se va a hacer antes de los 9 ms disparo 1.5ms
               if (rTemp>=9000){           //si el disparo va a ser posterior a los 9 ms
                  rTdisp=9500-SP_Calc;     //calculo tiempo de disparo
               }
      }
 
      if(flagcommand){         // Si hay comando pendiente lo procesa
               ModBus_exe();        
               flagcommand=0;
               inicbuffRX();
               rxpuntero=0; 
         }
         ///agregamos esto para no hacer por interrupcion la transmision
      if (fTX_Enable && TRMT) {
         ModBusTX ();
      }
      if (t2){
         int16 CRC=0;
         int16 CRC_Leido=0;
               t2=0;
               IF(fParami){                        //Calculo checksum de dato recibido
                     CRC=CRC_Calc(rxbuff,rxpuntero-3); //
                     CRC_Leido= make16(rxBuff[rxpuntero-2],rxBuff[rxpuntero-1]);
                     if(CRC_Leido==CRC)flagcommand=1;
                     setup_timer_2(T2_DISABLED,0,1);
                     rxpuntero=0;
                     }
               else{
                  inicbuffRX();
                  rxpuntero=0;
                  }
                  setup_timer_2(T2_DISABLED,0,1);
      }
      if (sp < 50) fHab=0;
      else fHab=1;
 /*     if (COM != 0){
            switch (COM){
                  
                  case 1:  //habilitar placa
                        fHab=1;
                        fEstHab=1;
                        break;
                  case 2:  //deshabilitar placa
                        fHab=0;
                        fEstHab=0;
                        break;
                  DEFAULT:
                        break;
            
            }
            COM=0;
      }*/
      
   } while (TRUE);
}




//++++++++++++++++++ RUTINA PID ++++++++++++++++++++++++++++++
INT16  PID(int16 SP, INT16 PV, KP, KI,signed int16 *aError)  {
signed int16 A=0;
signed int16 P=0;
signed int16 rI=0;
signed int16 Error=0;
int16   OUT=0;                     //Error actual
INT1  marca=0;

      Error=(signed int16)(SP-PV);

      if((-500 >= Error) && (Error >= 500)){
         Error=500;
      }

      P=Error*(signed int16)KP/100;         // Componente proporcional
      rI=(signed int16)KI;               // Componente integral
      rI=rI*Error*3/100;
      rI=rI+*aError;
      A=P+rI;
      if (A>900){                // Evaluo salida antes 1023
         *aError=900-P;          // antes 1023
         OUT=900;                // antes 1023
         marca=1;
         }                       // Si es mas que el max, sale el max
      if (A<0){
         *aError=0-P;
         OUT=0;
         marca=1;
         }
      if (marca==0){
         *aError=rI;
         OUT=(INT16)(P+rI);
         }
      return OUT;
}

