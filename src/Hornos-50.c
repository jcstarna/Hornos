// control de lamparas por angulo de fase.
// Version 2 control de disparo por modulo CCP
//  03/07/08   agregado de arranque del modulo pwm para disparo de triac
//  Revision   2.1
//  08/07/08   implementacion de direccion para comunicacion
//  Revision 
//  07/10/08   Se saca interrupcion por comparacion y se precarga el timer1 y se apaga la
//             interrupcion cuando desborda el timer 1
//  Revision 
//  04/01/09   Implementacin de escritura del Set point de potencia desde PC
// 
//  Revision 
//  04/01/10   Modificacion del maximo tiempo de disparo para lograr apagado total
//             de la lamapara
//  Revison
//  07-11-12   Modificacion de micro a 16F883
//
//  Revision   Error de disparo al iniciar placa provoca pestañeo de la lampara
//  17/4/14    Cambiamos orden de activacion de interrupciones
//             se seteo el PV y el AcuErr en cero antes del main
//             se desactivo fPid antes de entrar al main
//             en rutina de PID, se corrigio los limites del error agregando 
//             else if para la condicion >=500
//
//  Nueva version de hard, sin trafos, con medicion de corriente.
//  28/03/17   
//             Deteccion de falla por cruce por cero, deteccion de falla de 
//             set point no alcanzado.
//             reset el error acumulado ante falla de cruce por cero para 
//             evitar inicio con potencia alta.
//             Comunicacion modbus estable
//  26-5-21   
//             Release con medicion de corriente calibrada
//             Estados via modbus probado.
//             correccion de senial de habilitacion
//

#include <16f886.h>                          // Definiciones del PIC 16F886


#byte OPTION_REG = 0x081
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
#fuses XT,NOWDT,PROTECT,NOLVP,NOBROWNOUT,NODEBUG,PUT,MCLR
#use delay(clock=4000000)                     // Oscilador a 4 Mhz
#use rs232(baud=19200, xmit=PIN_C6, rcv=PIN_C7, ERRORS)// RS232 Estándar
#priority EXT,TIMER1,RDA,RTCC,TBE
#pragma use fast_io(A)
#pragma use fast_io(B)
#pragma use fast_io(C)
#rom int8 0x2100={1,00,1}

#define pTiristor_On    output_high(pin_C4)
#define pTiristor_Off   output_low(pin_C4)

// VARIABLES EN RAM ///////////////////////////////////////////////////////////

// Variables PID
//tabla para lectura desde PC//
int16 SP=0;                //Dir: 0 Set point de potencia
int16 PV=0;                //Dir: 1 Valor actual de potencia
int16 I=0;                 //Dir: 2 Corriente
int16 rTempI=0;
int16 Error=0;             //Dir: 3 Registro para cargar errores
int16 SP_Calc=1023;        //Dir: 4 Sp resultante de la comparacion del pv con el sp deseado(lectura Pote)
int16 rSPTimer1=0;         //Dir: 5 
signed int16 acuErr=10;    //Dir: 6 Error en para el calculo de PID
int16 REM_SP=0;            //Dir: 7 Set point remoto (desde PC)
INT16  rTDisp=0;
int16  rPID=0;

int16 uno=0;
int16 dos=0;
int16 tres=0;
int16 cuatro=0;
int16 cinco=0;
int16 seis=0;
int16 siete=0;
//fin tabla//

#bit LR        = REM_SP.15
//DEFINICIONES DE LOS BITS DE ERROR//
#bit fReg      =Error.5    //probado ok
#bit fELamp    =Error.4
#bit fECom     =Error.3    //probado ok     
#bit fETriac   =Error.2
#bit fESp      =Error.1    //probado ok
#bit fECero    =Error.0    //probado ok

//TIEMPOS EN LAS FALLAS//
#define kT_falla_SP     6000
#define kT_falla_Cero   10
#define kT_falla_triac  7000 //x 500ms = 30seg

//Patrones para estado
#define kOk    0b0000000000000001   //estado ok, led
#define kfCero 0b0000000011111111   //error cruce por cero
#define kLamp  0b0000111100001111   //Error lampara quemada
#define kSP    0b0011001100110011   //SP no alcanzado
#define kTriac 0b0101010101010101   //triac quemado
#define kCom   0b0000000000001111   //error de com
#define kReg   0b1111001111001111   //buscando SP

//CANALES AD
#define  kTi     0       //medicion de corriente
#define  kPV     1       //medicion de tension
#define  kSPext  2       //medicion de potenciometro o 4-20mA

//PINES
#define  pLed1       PIN_C1   //
#define  pLed2       PIN_C2   //
#define  pStatus     PIN_C3   // pLed3
#define  pDisp       PIN_C4   //
#define  pEnable     PIN_A3
#define  pRemSP      PIN_B5   //SP REMOTO POR RED CUANDO ESTA EN 1

#include "ModBus.h"
#include "Modbus.c"

INT  rDelay=10;            //Para lograr un tiempo determinado con la int del TMR0
int  rLeds=0;              //Imagen de los leds
int1 ctrlPot=0;            //define desde donde se controla la potencia 0=desde pote, 1=desde comunicacion
int1 fDisparo=1;           //esta disparando el triac
int1 fPID=0;
int1 T2=0;                 //salto el T2
int  rTarea=1;             //Maquina de estados


//  Variables para deteccion de errores
int  rT_falla_SP  =    kT_falla_SP;      
int  rT_falla_Cero=    kT_falla_Cero; 
int  rT_falla_triac=   kT_falla_triac;
 
// Declaración de Funciones ///////////////////////////////////////////////////

void inicbuffTx(void);                    // Borra buffer
void inicbuffRx(void);                    // Borra buffer
void procesa_comando(void);               // Procesa comando
void Control(void);                       //Calcula set point, error, determina SP_Calc.
INT16  PID(int16 SP,INT16 PV, KP, KI,signed int16 *aError);
int16 ScaleAI(int16 rawEA);               //escalar corriente
void statusLed(void);
int1 testTriac(int16 SP,int16 PV);         //chequea estado triac                            

// INTERRUPCIONES /////////////////////////////////////////////////////////////
//++++++++++++++++++++++++++++++++++++++
//  RECEPCION SERIE.
//++++++++++++++++++++++++++++++++++++++
#int_rda
void serial_isr() {                       // Interrupción recepción serie USART
rcvchar=0x00;                             // Inicializo caracter recibido
   if(kbhit()){                           // Si hay algo pendiente de recibir ...
      output_high(pled1);
      rcvchar=getc();                     // lo descargo y ...
      ModBusRX(rcvchar);                  // lo añado al buffer y ... 
   }   
   setup_timer_2(T2_DIV_BY_16,0xf0,05);   // set temporizador para time out de recepcion de datos (deteccion de fin de trama RTU)
}

//++++++++++++++++++++++++++++++++++++++
//  TRANSMISION SERIE.
//++++++++++++++++++++++++++++++++++++++
#INT_TBE
void serial_tx(){
  // ModBusTX ();
}

#int_RTCC
void RTCC_isr() {                // Interrupción Timer 0
   if(rDelay--==0){
      rDelay=2;
      if (rTarea++>3)rTarea=1;
      fPID=1;
      //output_toggle(pled3);
      
//Deteccion de errores

//falla no se puede alcanzar el sp
   if(rT_falla_SP--==0 & !fESp)fESp=1;
//falla triac
   if(rT_falla_triac--==0 & !fETriac)fETriac=1;
   }
//falla falta de cruce por cero
   if(rT_falla_Cero--==0 & !fECero)fECero=1;
}

///////// INTERRUPCION POR TIMER 2 ////////////
#int_TIMER2
void tmr2_isr(){
//int16 CRC=0;
//int16 CRC_Leido=0;
output_low(pled1);
t2=1;
/*   IF(fParami){                              //Calculo checksum de dato recibido
         CRC=CRC_Calc(rxbuff,rxpuntero-3);   //
         CRC_Leido= make16(rxBuff[rxpuntero-2],rxBuff[rxpuntero-1]);
         if(CRC_Leido==CRC)flagcommand=1;
            setup_timer_2(T2_DISABLED,0,1);
            rxpuntero=0;
         }
   else{
         setup_timer_2(T2_DISABLED,0,1);
         rxpuntero=0;
         inicbuffRX();
         rxpuntero=0;
         }    
         */
}
         
///////// INTERRUPCION POR CRUCE POR CERO ////////////
#INT_EXT
void ext_isr(){
int16 rTemp1=0;
     pTiristor_off;                       //prueba 4/08/09
     rT_falla_Cero=kT_falla_Cero;         //recargo contador para falla cruce por cero
     fECero=0;
     rTemp1=65535-(SP_Calc);
     if (rTemp1 > 64335)rTemp1=64335;     //maximo anterior (version 2.2 64035)
     if (rTemp1 < 58035)rTemp1=58035;     //minimo    65535-58535=     57700
     set_timer1(rTemp1);
     rSPTimer1=rTemp1;
     #ASM bcf  0xd,0
          bcf  0xc,2
     #ENDASM  
     clear_interrupt(int_timer1);
     set_timer1(rTemp1);
     enable_interrupts(int_timer1);       // Habilito la interrpción INT_CCP2
     fDisparo=0;
}


///////// INTERRUPCION IGUALACION DE CCP_2 Y TIMER1 ////////////
#int_TIMER1                            //Timer 1 overflow
void TIMER1_int(){
int16 rTempT1;
   clear_interrupt(int_timer1);
   if (fDisparo){                      //ya esta disparando
      pTiristor_off;                   //apago 
      enable_interrupts(int_ext);      //habilito interrupcion por cruce por cero
      fDisparo=0;
      disable_interrupts(int_timer1);
   }
   else{ // tiene que empezar a disparar
         if (!INPUT(pEnable)) pTiristor_on;                 //disparo
         set_timer1 (64900);           //(rTempT1);//64535
         fDisparo=1;                   //marco que estoy disparando
         enable_interrupts(int_timer1);     
   }
}

////////////////////////////////////////////////////////////////////
////////////////////////  Programa Principal  //////////////////////
////////////////////////////////////////////////////////////////////

void main() {
   int16 rTemp=0;
   
   int8  Temp=0;
   delay_ms(500);
//++++++++++++++++++Configura puertos analogicos++++++++++++++++++++++++++++++
   setup_adc(  ADC_CLOCK_INTERNAL  );
   setup_adc_ports( sAN0|sAN1|sAN2 );           // Ver que puertos van a ser analogicos
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_128);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);//
   setup_timer_2(T2_DIV_BY_4,0xF0,10);          //control de inactividad del bus
   setup_comparator(NC_NC_NC_NC);               //no comparadores
   
   //Seteos puertos  
   set_tris_a (0b00001111);
   set_tris_b (0b11111111);
   output_c(0);
   set_tris_c (0b10000001);
   rDelay = 0;
   rLeds  = 1;
            
   inicbuffRX();                                // Borra buffer al inicio
   inicbuffTX();                                // Borra buffer al inicio
   ext_int_edge(L_TO_H);

   temp=input_b()/2;
   ModbusAddress = temp & 0b00001111;
   //ModbusAddress=1;
   fPID=0;
   rTarea=1;
   set_adc_channel(kSPext);
   clear_interrupt(int_timer1);
   disable_interrupts(INT_TIMER1);
   SP_Calc=10230;
   acuErr=10;
   PV=0;
   enable_interrupts(int_rda);                  // Habilita Interrupción RDA
   enable_interrupts(int_RTCC);                 // Habilita Interrupción TIMER0
   enable_interrupts(int_TIMER2);               // Habilita Interrupción TIMER2  
   enable_interrupts(int_ext);                  // Habilita Interrupción RB0, PASO DE BAJO A ALTO
   enable_interrupts(global);                   // Habilita interrupciones
   
   
   /////////BUCLE PRINCIPAL//////////
   do {
      if(flagcommand){                          // Si hay comando pendiente lo procesa
               ModBus_exe();
               //output_toggle(pled3);
               flagcommand=0;
               inicbuffRX();
               rxpuntero=0; 
         }
      ///agregamos esto para no hacer por interrupcion la transmision
      if (fTX_Enable && TRMT) {
         ModBusTX ();
      }    

      if(fPID & !fTX_Enable){
               switch (rTarea){
                     case 1://leo SET POINT //HACER ACA LOGICA PARA SABER SI TOMAR REGISTRO MODBUS O CONVERSOR AD
                            if (!INPUT(pRemSP)) SP=read_adc();
                            set_adc_channel(kPV);
                            if (INPUT(pEnable)){
                              SP=0;
                              SP_Calc=10230;
                              acuErr=10;
                              PV=0;
                            }
                            break;
                     case 2://leo PV       
                            PV=read_adc();
                            set_adc_channel(kTi);
                            break;
                     case 3://LEO CORRIENTE
                            rTempI=read_adc();
                            set_adc_channel(kSPext);
                            break;
               }
               fPID=0;
               rPID=PID (SP, PV,2,2,&acuErr);   //read_adc(); esta salida  varia entre 0 y 1023  TEMP
               if (fECero)acuErr=0;             //resetear acumulador de error para evitar disparo al 100 al iniciar
               SP_Calc=10000-(rPID*10); 
               //calculo diferencia para resetear tiempo de error
               if (sp>pv){
                  if((SP-PV)<60){
                     rT_falla_SP=kT_falla_SP;
                     fESp=0;    //reseteo contador
                     fReg=0;
                  }
                  else
                     fReg=1;                  
               }
               else{
                  if((PV-SP)<60){
                     rT_falla_SP=kT_falla_SP;
                     fESp=0;    //reseteo contador
                     fReg=0;
                  }
                  else
                     fReg=1;
               }
               //rTemp=(SP_Calc+1300);
               //if (rTemp<9000)rTdisp=1500;       //si el  se va a hacer antes de los 9 ms disparo 1.5ms
               //if (rTemp>=9000){                 //si el disparo va a ser posterior a los 9 ms
               //   rTdisp=9500-SP_Calc;           //calculo tiempo de disparo
               //}
               statusLed();
      }
      //expiro timer 2, inactividad de comunicacion
      //revisar si es para mi el mensaje recibido
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
      if(!fTX_Enable){
         //escalado de corriente
         I=ScaleAI(rTempI);
         //lampara quemada (revisar tension y corriente)
         //la corriente depende de la potencia total que maneja la placa
         //hay que configurar la cantidad de lamparas o de potencia o ambas (ver)
         
         //falla de tiristor cuando el tiempo de disparo es muy alto para el sp
         if(!testTriac(SP,PV) | fESp){ //aca si hay error de triac
            rT_falla_triac=kT_falla_triac;
            fETriac=0;
         }
         //ver otras
         //estado del SP vs PV
         }
   } while (TRUE);

}

//chequeo del triac
/*
Calcula el valor teorico del PV (tiempo de disparo)segun el SP
y lo compara con el PV que se le pasa a la funcion, si la 
diferencia es mayor al 30% (teoricamente 50%), el disp de control esta
con uno de los tiristores quemados
*/
int1 testTriac(int16 SP,int16 PV){
float PV_teorico=0;
float relPV=0;
float rSP = (float) SP;
float rPV = (float) PV;

PV_teorico = (-5.76*rSP+7731.3);
relPV=PV_teorico/rPV;
if (relPV<0.75)
   return 1;
else
   return 0;
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

      if((-500 >= Error) ){
         Error=-500;
      }
      else if (Error >= 500){
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



//Rutina para cambio calculo de corriente.
int16 ScaleAI(int16 rawEA){
float Corriente=0;
float EA=0;
EA=(float)rawEA;
   Corriente = ((float)rawEA/1024)*100; //cambio a 100 por resistenca
   Corriente = -0.000025 * EA*EA + 0.092 * EA + 8.5;
   return (int16)Corriente*10;
}

// indicacion en led de estado.
void statusLed(void){
int1 res=0;
static int i=0;
static int16 ppp;
int16 patron=0;

//Cargar patron segun error
if(fECom)         //error de comunicacion
   patron=kCom;
else if (fETriac) //error de triac
   patron=kTriac;
else if (fESp)    //error SP no alcanzado
   patron=kSp;
else if (fELamp)  //error lampara quemada
   patron=kLamp;
else if  (fReg)
   patron=kReg;
else
   patron=kOk;
   
//activar led
   res = shift_right(&ppp,2,0);
   if (res)
      output_high (pStatus);
   else
      output_low (pStatus);
if (++i >15){
   i=0;
   ppp=patron;
}
}
