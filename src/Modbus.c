////////////////////////////////////////////////////////////////////////////////
////                       IMPLEMENTACION MODBUS
////
////FECHA:  07/12/09
////REVISION: 0
////AUTOR:  CONTROLARG
////////////////////////////////////////////////////////////////////////////////
//
//REVISIONES:
//FECHA: 18/04/11
//Agregado de la funcion 0x10 para escritura de multiples registro
//Probado: OK
//Faltaria hacer comprobacion de datos osea, que lo que se escribio, realmente
//es lo que se queria escribir- comparar registros

//FECHA 10/04/11
//Se agrega la funcion 0x06, escritura de un solo registro



////////////////////////////////////////////////////////////////////////////////
////                       CALCULO DEL CRC DE O BITS
////////////////////////////////////////////////////////////////////////////////
int16 CRC_Calc (int *MSG,int msgLen){
int16 crc=0xffff;
int i=0 ;   //contador de bytes
int j=0;    //contador de bits
int16 dato=0;
for (i=0;i<=msgLen;i++){   //bucle para cada uno de los bytes del mensaje
   dato=*(msg+i);
   crc=crc^dato;
      for (j=0;j<=7;j++){
         if (shift_right(&crc,2,0))
                  crc=crc^0xA001; //si el bit a la derecha es 1
      }
}
crc = (crc << 8) | (crc >> 8);
fParami=0;
return crc;
}



////////////////////////////////////////////////////////////////////////////////
////                       PROCESAR DATO RECIBIDO
////////////////////////////////////////////////////////////////////////////////
/*Al llegar un dato nuevo al buffer de RX, se envia a esta rutina, si el dato 
  es para este esclavo se lo procesa, de otra forma se rechaza para reducir tiempo
*/
void ModBusRX (char data){
   SET_TIMER2(0);   
   if ((rxPuntero==0) & (data==ModbusAddress))fParami=1;
   if(fParami){
      Rxbuff[rxPuntero]=data;
      //rxpuntero++;
   }
    rxpuntero++;
}



////////////////////////////////////////////////////////////////////////////////
////                       TRANSMITIR BUFFER
////////////////////////////////////////////////////////////////////////////////
/*Al llegar un dato nuevo al buffer de RX, se envia a esta rutina, si el dato 
  es para este esclavo se lo procesa, de otra forma se rechaza para reducir tiempo
*/
void ModBusTX (){
   if(txlen-->0){
      output_high(PIN_C5);
      output_high(pLed2);
      putc(txbuff[txpoint]);//txbuff
      txpoint++;
   }
   else{
      //disable_interrupts(int_tbe);
      fTX_Enable=0;
      inicbuffTX();
      delay_us(500);
      output_low(PIN_C5);
      output_low(pLed2);
   }
}

////////////////////////////////////////////////////////////////////////////////
////                       PROCESAR COMANDO MODBUS
////////////////////////////////////////////////////////////////////////////////
/* Analiza el buffer de recepcion y elabora respuesta.
*/
void ModBus_exe (){
char  comando=0;
int   cant=0;
int   offset=0;
int   *DirIni;
int8 i =0;
int16 CRC=0;
int1 MdbError=0;
flagcommand=0;
comando = rxBuff[ModBusCmd];  //recupero comando
offset = make16(rxBuff[ModbusIni],rxbuff[ModBusIni+1]);    //recupero direccion del registro como offset del inicio de la memoria
cant = make16(rxBuff[ModbusCant],rxbuff[ModBusCant+1]);    //recupero cantidad de registros para comando 0x03
DirIni = &SP + offset;
      if ((Cant > 8) | (Offset > 2)){
         Modbus_Exep(ModbusAddress,3,2);
         MdbError=1;
         fECom=1;
      }
      if (comando == 0x04){
         Modbus_Exep(ModbusAddress,4,1);
         MdbError=1;
         fECom=1;
      }
   if (!MdbError){ 
      fECom=0;
      switch (comando){
         case 0x03:     //leer registros
            //armo respuesta modubs para comando 0x03
            txbuff[ModbusAdd]=ModbusAddress;       //Direccion esclavo; 
            txbuff[ModbusCmd]=comando;             //Comando
            //busco los datos que esta en memoria
            for (i=1;i<=cant;i++){      
                  txbuff[i*2+1]=*(DirIni+1);
                  txbuff[i*2+2]=*(DirIni);
                  DirIni+=2;
            }
            txlen=cant*2;
            txbuff[ModbusBCount]=txlen;            //Cantidad de Bytes
            CRC=CRC_Calc(txBuff,txlen+2);          //Calculo checksum del mensaje
            //cargo CRC en buffer
            txBuff[txlen+3]=make8(CRC,1);          //Checksum parte alta
            txBuff[txlen+4]=make8(CRC,0);          //Checksum parte baja
            txlen=txlen+5;                         //Cantidad de bytes en el buffer de transmision
            break;
         case 0x04:
            break;
         case 0x06://Setear un registro
                  //00 esclavo
                  //01 comando
                  //02 DirH
                  //03 DirL
                  //04 DataH
                  //05 DataL                
                  *(DirIni+1) =rxbuff[4];
                  *(DirIni)   =rxbuff[5];
                  //ARmo respuesta que es repetir la pregunta
                  if (DirIni > 2)fEscEEPROM=1;
            Modbus_RespOK(ModbusAddress, comando); 
            txlen=8;
            break;            
         case 0x10://Setear Multiples Registros
            for (i=1;i<=cant;i++){      
                  *(DirIni+1) =rxbuff[i*2+5];
                  *(DirIni)   =rxbuff[i*2+6];
                  DirIni+=2;
            }
            if (DirIni > 2)fEscEEPROM=1;
            //Armo respuesta
            Modbus_RespOK(ModbusAddress, comando);
            txlen=8;
            break;            
         case 0x11:
            break;
         }
      txpoint=0;
      //enable_interrupts(int_tbe); 
      fTX_Enable=1;
      delay_us(1);
      }
}

//Arma respuesta error de excepcion
void ModBus_Exep(int SlvAdd, int Comando, int ExepNro){
int16 CRC=0;
inicbuffTX();
//01 Illegal function
//02 Illegal Data address
//03 Illegal Data Value
   txbuff[ModbusAdd]=SlvAdd;       //Direccion esclavo; 
   txbuff[ModbusCmd]=Comando+0x80;        //Comando
   txbuff[2]=ExepNro;
   CRC=CRC_Calc(txBuff,2);         //Calculo checksum del mensaje
   txBuff[3]=make8(CRC,1);          //Checksum parte alta
   txBuff[4]=make8(CRC,0);          //Checksum parte baja
   txpoint=0;
   txlen=5;
   //enable_interrupts(int_tbe); 
   fTX_Enable=1;
   delay_us(1); 
}


void Modbus_RespOK(int SlvAdd,int Cmd){
int16 CRC=0;
            txbuff[ModbusAdd]=SlvAdd;                //Direccion esclavo; 
            txbuff[ModbusCmd]=cmd;             //Comando
            txBuff[ModbusIni]=rxBuff[ModbusIni];
            txBuff[ModbusIni+1]=rxBuff[ModbusIni+1];
            txBuff[ModbusCant]=rxBuff[ModbusCant];
            txBuff[ModbusCant+1]=rxBuff[ModbusCant+1];
            CRC=CRC_Calc( txBuff,5);          //Calculo checksum del mensaje
            //cargo CRC en buffer
            txBuff[6]=make8(CRC,1);          //Checksum parte alta
            txBuff[7]=make8(CRC,0);          //Checksum parte baja
}

void inicbuffRX(void){                   // Inicia a \0 cbuff -------------------
   int i;
   for(i=0;i<lenbuff;i++){             // Bucle que pone a 0 todos los
      Rxbuff[i]=0x00;                   // caracteres en el buffer
   }
   rxPuntero=0x00;                         // Inicializo el indice de siguiente
}

void inicbuffTX(void){                   // Inicia a \0 cbuff -------------------
   int i;
   for(i=0;i<lenbuff;i++){             // Bucle que pone a 0 todos los
       txbuff[i]=0;
   }
   rxPuntero=0x00;                        // Inicializo el indice de siguiente                                      // caracter
}
