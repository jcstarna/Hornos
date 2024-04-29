////////////////////////////////////////////////////////////////////////////////
////                       IMPLEMENTACION MODBUS
////                    VARIABLES, CONSTANTES, FUNCIONES.
////FECHA:  07/12/09
////REVISION: 0
////AUTOR:  CONTROLARG
////////////////////////////////////////////////////////////////////////////////

//Constantes comando trama modbus
int const   ModbusAdd=0;               //posicion de la direccion en la trama modbus
int const   ModbusCmd=1;               //posicion del comando en la trama modbus
int const   ModbusIni=2;               //posicion del primer registro solicitado en trama modbus 2 bytes
int const   ModbusCant=4;              //posicion de la cantidad de registros en la trama modbus 2 bytes
int const   ModbusBCount=2;            //posicion de la cantidad de bytes en la respuesta modbus
int const   lenbuff=20;

char  Txbuff[lenbuff];                   // Buffer
char  Rxbuff[lenbuff];                   // Buffer
int   rxPuntero=0x00;                    // Índice: siguiente char en cbuff
INT8  rcvchar=0x00;                      // último caracter recibido
int   txlen=0;
int   txpoint=0;
int8  CRC8=0; 
int8  ModbusAddress=0;                 //direccion del esclavo


int1  flagcommand=0;                    // Flag para indicar comando disponible
int1  fParami =0;                       // Indica que el comando es par este esclavo
int1  fFinTrama=0;                      // Paso el tiempo de espera para considerar que termino la trama 3 bit time para RTU 
int1  fEscEEPROM=0;                     // Grabar datos en la eeprom
int1  fTX_Enable=0;

char CRC8 (char Dato);
void ModBusRX (char data);
void ModBus_exe (void);
void ModBusRX (VOID);
void ModBus_Exep(int SlvAdd,int Comando, int ExepNro);
void Modbus_RespOK(int SlvAdd, int Cmd);
void inicbuffTx(void);                   // Borra buffer
void inicbuffRx(void);                   // Borra buffer
