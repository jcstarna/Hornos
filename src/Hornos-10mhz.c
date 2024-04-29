//control de lamparas por angulo de fase.
//Version 2 control de disparo por modulo CCP
//  03/07/08  agregado de arranque del modulo pwm para disparo de triac
//  08/07/08  implementacion de direccion para comunicacion

#include <16F873A.h>                          // Definiciones del PIC 16F873A
#DEVICE ADC=10
#fuses XT,NOWDT,NOPROTECT,NOLVP,PUT,BROWNOUT  // Los Fuses de siempre
#use delay(clock=10000000)                     // Oscilador a 4 Mhz
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, ERRORS)// RS232 Estándar

#pragma use fast_io(A)
#pragma use fast_io(B)
#pragma use fast_io(C)

#define pTiristor_On set_pwm1_duty(5)
#define pTiristor_Off set_pwm1_duty(0)


// CONSTANTES /////////////////////////////////////////////////////////////////
int const   lenbuff=23;
// VARIABLES EN RAM ///////////////////////////////////////////////////////////

INT  rDelay=10;            // Para lograr un tiempo determinado con la int del TMR0
int  rLeds=0;              // Imagen de los leds
int1 ctrlPot=0;            //define desde donde se controla la potencia 0=desde pote, 1=desde comunicacion
int1 fDisparo=1;           //esta disparando el triac

// Variables PID
int16 SP_Calc=0;        // Sp resultante de la comparacion del pv con el sp deseado(lectura Pote)
int16 SP=0;                // Tiempo en que se tiene que apagar V1
int16 PV=0;
signed int16 acuErr=10;
INT16 TEMP=0;
int1 fPID=0;

// Variables comunicacion
int1 fTX=0;          //Transmision pendiente
int  xbuff=0x00;     // Índice: siguiente char en cbuff
char Txbuff[lenbuff];// Buffer transmision
char Rxbuff[lenbuff];// Buffer recepcion
char rcvchar=0x00;   // último caracter recibido
int1 flagcommand=0;  // Flag para indicar comando disponible
int  txlen=0;        //longitud del buffer a transmitir
int  txpoint=0;      //Puntero del caracter a transmitir
int1 fEscOK=0;       //el mensaje recibido es para este esclavo
int  nEsclavo=0;     //Direccion de la placa


// Declaración de Funciones ///////////////////////////////////////////////////

void inicbuffTx(void);                 // Borra buffer
void inicbuffRx(void);                 // Borra buffer
int  addcbuff(char c);                 // añade caracter recibido al buffer
void echos(char c);                    // Eco selectivo sobre RS232
void procesa_comando(void);            // Procesa comando
void Control(void);                    //Calcula set point, error, determina SP_Calc.
INT16  PID(int16 SP,INT16 PV, KP, KI,signed int16 *aError);
                             // Lectura de la potencia actual en lampara


// INTERRUPCIONES /////////////////////////////////////////////////////////////

#int_rda
void serial_isr() {                    // Interrupción recepción serie USART
   rcvchar=0x00;                       // Inicializo caracter recibido
   if(kbhit()){                        // Si hay algo pendiente de recibir ...
      rcvchar=getc();                  // lo descargo y ...
      addcbuff(rcvchar);               // lo añado al buffer y ...
      //echos(rcvchar);                  // hago eco (si procede).
   }
}
//++++++++++++++++++++++++++++++++++++++
//  TRANSMISION SERIE.
//++++++++++++++++++++++++++++++++++++++
#INT_TBE
void serial_tx(){
   if(txlen!=0){
      putc(txbuff[txpoint]);
      txpoint++;
      txlen--;
   }
   else{
   disable_interrupts(int_tbe);
   inicbuffTX();
   txpoint=0;
   output_low(PIN_C3);
   }
}

#int_RTCC
void RTCC_isr() {                      // Interrupción Timer 0
   if(rDelay--==0){
      rDelay=9;
      fPID=1;}
}


///////// INTERRUPCION POR CRUCE POR CERO ////////////
#INT_EXT
void ext_isr(){
int16 rTemp1=0;
      disable_interrupts(int_ext);
      rTemp1=SP_Calc+1300;

     if(input(PIN_B0)){
         ext_int_edge(H_TO_L);
     }
     ELSE{
         ext_int_edge(L_TO_H);
     }
     set_timer1(0);
     CCP_2 = rTemp1;
     #ASM bcf  0xd,0
          bcf  0xc,2
     #ENDASM     
     enable_interrupts(int_ccp2);             // Habilito la interrpción INT_CCP2
     fDisparo=0;
    }    

///////// INTERRUPCION IGUALACION DE CCP_2 Y TIMER1 ////////////

#int_ccp2
void ccp2_int(){
int16 rTempT1;
   if (fDisparo){//ya esta disparando
      pTiristor_off;                //apago pwm
      enable_interrupts(int_ext);   //habilito interrupcion por cruce por cero
      fDisparo=0;
      disable_interrupts(int_ccp2);
   }
   else{ // tiene que empezar a disprara
      rTempT1=get_timer1();
      if (rTempT1<10500){
         pTiristor_on;                 //disparo
         CCP_2=rTempT1+100;            //SETEO TIEMPO DE DISPARO
         fDisparo=1;                   //marco que estoy disparando
      }
      else
         enable_interrupts(int_ext);   //habilito interrupcion por cruce por cero
   }
}

// Desarrollo de Funciones ////////////////////////////////////////////////////


void inicbuffRX(void){                   // Inicia a \0 cbuff -------------------
   int i;
   for(i=0;i<lenbuff;i++){             // Bucle que pone a 0 todos los
      Rxbuff[i]=0x00;                   // caracteres en el buffer
   }
   xbuff=0x00;                         // Inicializo el indice de siguiente
   }

void inicbuffTX(void){                   // Inicia a \0 cbuff -------------------
   int i;
   for(i=0;i<lenbuff;i++){             // Bucle que pone a 0 todos los
       txbuff[i]=0;
   }
   xbuff=0x00;                         // Inicializo el indice de siguiente
                                       // caracter
}

int addcbuff(char c){                  // Añade a cbuff -----------------------
      if (c == 0x0d)flagcommand=1;
      else Rxbuff[xbuff++]=c;
}


//,,,,,,,,,,,,Programa Principal /////////////////////////////////////////////////////////

void main() {

   delay_ms(500);
//++++++++++++++++++Configura puertos analogicos++++++++++++++++++++++++++++++
   setup_adc(  ADC_CLOCK_INTERNAL  );
   setup_adc_ports( ALL_ANALOG );                 // Ver que puertos van a ser analogicos
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_128);
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_8);        
   setup_ccp2(CCP_COMPARE_INT);    // Configura CCP2 en modo COMPARE interrumpe cuando son iguales
   
   //Seteos puertos  
   set_tris_a (0b00000011);
   set_tris_b (0b00000001);
   output_c(0);
   set_tris_c (0b11010000);
   rDelay = 0;
   rLeds  = 1;

   ///PWM para disparo del tiristor
   set_pwm1_duty(0);
   setup_ccp1(CCP_PWM);
   setup_timer_2( T2_DIV_BY_1 , 20,2); 
   set_pwm1_duty(0);
            
   inicbuffRX();                                // Borra buffer al inicio
   inicbuffTX();                                // Borra buffer al inicio
   enable_interrupts(int_rda);                  // Habilita Interrupción RDA
   ext_int_edge(H_TO_L);
   enable_interrupts(int_RTCC);                 // Habilita Interrupción TIMER0
   enable_interrupts(int_ext);                  // Habilita Interrupción RB0, PASO DE BAJO A ALTO
   //enable_interrupts(global);                 // Habilita interrupciones
   set_timer1(0);
   output_b(0);
   output_c(0);
   fPID=1;
   
   do {
      if(flagcommand) procesa_comando();         // Si hay comando pendiente
                                             // de procesar... lo procesa.
      if(fPID){
            set_adc_channel(0);
            delay_us(100);
            if (!ctrlPot) SP=read_adc()/10;
            set_adc_channel(1);             
            delay_us(100);
            PV=read_adc()/10;
            fPID=0;
            //TEMP=PID (SP, PV,2,2,&acuErr);    
            SP_Calc=(SP/100)*10500;
            enable_interrupts(global);
            }
      
      if(fTX){
         fTX=0;
         output_high(PIN_C3);
         enable_interrupts(int_tbe); 
      }
      
   } while (TRUE);

}

// Procesador de Comandos /////////////////////////////////////////////////////

void procesa_comando(void){
   int32 SP_RX=0;
   int i=0;
   char a;                   // Argumento de comando (si lo tiene)

   flagcommand=0;                       // Desactivo flag de comando pendiente.


   if(Rxbuff[0]=='\\'&& Rxbuff[1]=='D'){   // Comparo inicio del buffer con comando "\@"
         //output_high(PIN_C3);
         sprintf(TxBuff,"%lu,%lu,%lu,%lu,%ld\r\n",SP,SP_Calc,PV,TEMP,acuErr);
         txlen=21;
         //enable_interrupts(int_tbe);
         fTX=1;
   }

   if(Rxbuff[0]=='\\' && Rxbuff[1]=='w'){   // Comparo inicio del buffer con comando "\w"
      a=Rxbuff[2];
      SP_RX=(int32)a;
      IF (SP_RX<=100) SP_RX= (1023*SP_RX)/100 , SP = (INT16)SP_RX;
         //output_high(PIN_C3);
         sprintf(TxBuff,"w:%lu,%3u",SP_RX,a);
         txlen=10;
         //enable_interrupts(int_tbe);
         fTX=1;
      }

   if(Rxbuff[0]=='\\' && Rxbuff[1]=='c'){   // Comparo inicio del buffer con comando "\w"
      i=Rxbuff[2];
      IF (i==0x31)
         ctrlPot=1;
      else if (i==0x30)
         ctrlPot=0;
         
         //output_high(PIN_C3);
         sprintf(TxBuff,"c:%2u",i);
         txlen=4;
         //enable_interrupts(int_tbe); 
         fTX=1;
      }
   inicbuffRx();                          // Borro buffer.
}




//++++++++++++++++++ RUTINA PID ++++++++++++++++++++++++++++++
//REFORMADA PARA QUE DE UNA SALIDA DE 0 A 100
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
INT16  PID(int16 SP, INT16 PV, KP, KI,signed int16 *aError)  {
signed int16 A=0;
signed int16 P=0;
signed int16 rI=0;
signed int16 Error=0;
int16   OUT=0;                     //Error actual
INT1  marca=0;

      Error=(signed int16)(SP-PV);

      if((-50 >= Error) && (Error >= 50)){
         Error=50;
      }

      P=Error*(signed int16)KP/10;         // Componente proporcional
      rI=(signed int16)KI;               // Componente integral
      rI=rI*Error*3/10;
      rI=rI+*aError;
      A=P+rI;
      if (A>100){                // Evaluo salida
         *aError=100-P;
         OUT=100;
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
         OUT=100-OUT;
      return OUT;
}
