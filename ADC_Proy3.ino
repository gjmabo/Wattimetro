
// Global preamble
#include "Arduino.h"
#include "wiring_private.h"
//

#define PIN 10
#ifdef _VARIANT_ARDUINO_ZERO_
volatile uint32_t *setPin = &PORT->Group[g_APinDescription[PIN].ulPort].OUTSET.reg;
volatile uint32_t *clrPin = &PORT->Group[g_APinDescription[PIN].ulPort].OUTCLR.reg;
const uint32_t  PinMASK = (1ul << g_APinDescription[PIN].ulPin);
#endif

// ADCsync se encarga de sincronizar los dominios de reloj entre el ADC y
// el resto del microcontrolador.
// Lee elregistro STATUS del ADC y espera hasta que se libere.
static __inline__ void ADCsync() __attribute__((always_inline, unused));
static void   ADCsync() {
  while (ADC->STATUS.bit.SYNCBUSY == 1); // Esperar hasta que el ADC se libere
}

// Variables definidas
uint32_t Status = 0x00000000;
uint32_t ulPin_V = A1;      // Pin analogico para leer voltaje
uint32_t ulPin_C = A2;      // Pin analogico para leer corriente

// Arreglos para almacenar los datos de las conversiones.
// Almacenan valores entre 0 y 1023, para luego ser procesados.
// El tipo es uint32_t porque almacenan directamente el valor 
// tomado directamente del registro de salida del ADC. 
uint32_t rawDataV[1000], rawDataI[1000];           

int pinCrucePorCero = 11;   // Pin para leer el ancho del pulso
unsigned long pulseLenght, freq;
int pinADCRead, range;      // range toma el valor del rango en el que se encuentra la frecuencia de entrada

// Vectores para almacenar los valores ya procesados
float Vvector[1000];
float Ivector[1000];

// Variables auxiliares para el procesamiento de datos
float V;
float I;

void setup()
{
  Serial.begin(115200);          //  setup serial
  pinMode(PIN, OUTPUT);        // setup timing marker

  pinMode(pinCrucePorCero, INPUT);

  //###################################################################################
  // ADC setup stuff (basado en las bibliotecas de Arduino)
  //###################################################################################
  ADCsync();
  ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_1X_Val;      // Gain select as 1X
  //ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_VREFA_Val; //  External Reference
  ADC->REFCTRL.bit.REFSEL = 0x3; //  External Reference
  
  // Set sample length and averaging
  ADCsync();
  ADC->AVGCTRL.reg = 0x00 ;       //Single conversion no averaging
  ADCsync();
  ADC->SAMPCTRL.reg = 0x0A;  ; //sample length in 1/2 CLK_ADC cycles Default is 3F
  
  //Control B register
  int16_t ctrlb = 0x024;       // Control register B hibyte = prescale, lobyte is resolution and mode 
  ADCsync();
  ADC->CTRLB.reg = ctrlb; 

}




void loop()
{
    // PulseIn da el valor de la duracion de un pulso en usegs
    pulseLenght = pulseIn(pinCrucePorCero, HIGH);
    // Con ese valor se calcula la frecuencia, utilizando una función 
    // de mejor ajuste obtenida empíricamente.
    freq = 272480*pow(pulseLenght,-0.909);
    
    // Se escoge el rango según la frecuencia obtenida
    if ( freq > 2500 ){
        range = 1;
      }

    else if ( (2500 >= freq ) && (freq > 1000) ){
        range = 2;
      }
    
    else if ( (1000 >= freq ) && (freq > 500) ){
        range = 3;
      }

    else if ( (500 >= freq ) && (freq > 200) ){
        range = 4;
      }

    else if ( (200 >= freq ) ){
        range = 5;
      }
  
  // Configurar el prescaler del ADC y el pin de lectura de Voltaje
  configADC(1, range);

  for (int i=0; i<1000; i++){           // Para tener 1000 muestras
      rawDataV[i] = continuousADCRead();
    }
  
  // Configurar el prescaler del ADC y el pin de lectura de Voltaje  
  configADC(2, range);

  for (int i=0; i<1000; i++){           // Para tener 1000 muestras
      rawDataI[i] = continuousADCRead();
    }

  //---------------------Conversion----------------------------

  // Se pasan los valores sin procesar a los valores reales, segun el 
  // acondicionamiento de la señal utilizado. 
  // Los valores procesados se almacenan en el arreglo correspondiente.
  
     for (int i=0; i<1000; i++){        
        V = rawDataV[i] * ((3.3)/1023.0);
        V = (V-0.9)*27.7245*2;
        Vvector[i] = V;         
      }


     for (int i=0; i<1000; i++){
        I = rawDataI[i] * ((3.3)/1023.0);
        I=((I-0.8)-0.0614)/(6.1068);
        Ivector[i] = I;         
      }

//-------------------Valores Pico-----------------------

  // Se obtienen los valores pico de las señales leídas.
  
  float max_auxV, max_auxI, Vpico, Ipico;
  max_auxV = 0;
  max_auxI = 0;

    for(int i=0; i<1000; i++){
      max_auxV = max(max_auxV, Vvector[i]);
      }

    for(int i=0; i<1000; i++){
      max_auxI = max(max_auxI, Ivector[i]);
      }

  Vpico = max_auxV;
  Ipico = max_auxI;

//-------------------RMS--------------------------------

  // Se obtienen los valores RMS de las señales leídas.
  
  float auxV = 0;
  float auxI = 0;

    for (int i=0; i<=1000; i++){
      
      auxV = auxV + ((Vvector[i])*(Vvector[i]));
      auxI = auxI + ((Ivector[i])*(Ivector[i]));
      
    }
    
  float aux1V = auxV/1000 ;
  float Vrms = sqrt(aux1V);
  float aux1I = auxI/1000 ;
  float Irms = sqrt(aux1I);
  
//------------ Pontencia Promedio------------------- 

  // Calculo de la potencia promedio
  
  float auxP = 0;
  float auxP1 = 0;
  
    for (int i=0; i<=1000; i++){
        
      auxP1 = auxP + ((Vvector[i])*(Ivector[i]));
      
    }

  float PotProm = auxP1/1000 ;

//--------------------Potencia Aparente----------------------
  
  // Calculo de la potencia aparente

  float P_Aparente = ((Vrms)*(Irms));

//------------------Factor de Potencia-----------------------
  
  // Calculo del factor de potencia

  float FP = ((PotProm)/(P_Aparente));

//----------------Envio de los datos------------------------
  
  // Se envían de esta manera para que el programa Processing
  // pueda hacer la interfaz gráfica
  
  for(int i=0; i<1000; i++){
     Serial.print(Vvector[i]);
     Serial.print(" ");
     Serial.print(Ivector[i]);
     Serial.print(" ");
     Serial.print(Vrms);
     Serial.print(" ");
     Serial.print(Vpico);
     Serial.print(" ");
     Serial.print(Irms);
     Serial.print(" ");
     Serial.print(Ipico);
     Serial.print(" ");
     Serial.print(PotProm);
     Serial.print(" ");
     Serial.print(P_Aparente);
     Serial.print(" ");
     Serial.print(FP);
     Serial.print(" ");
     Serial.println(freq);
    
   
  }
  
}

// Funcion para implementar la lectura del registro de salida del ADC
uint32_t continuousADCRead() {
  
  ADCsync();

  while ( ADC->INTFLAG.bit.RESRDY == 0 );   // Esperar a que la conversion termine
  ADCsync();
  uint32_t valueRead = ADC->RESULT.reg;

  ADC->INTFLAG.bit.RESRDY = 1;              // Limpiar la bandera de 'Datos listos'

  return valueRead;
}

// Funcion para implementar la lectura en modo free-running
void configADC(int pin, int range){

  // Configura el pin desde el que se lee (voltaje o corriente)
  ADCsync();
  if (pin == 1){        // Voltaje
    ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[ulPin_V].ulADCChannelNumber; 
  }
  else if (pin == 2){   // Corriente
    ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[ulPin_C].ulADCChannelNumber;
  }

  // Configura la frecuencia para el modo 'free running'
  // Se modifica el valor del prescaler.
  ADCsync();
  if (range == 1){
    //ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PPRESCALER_DIV32_Val;
    ADC->CTRLB.bit.PRESCALER = 0x3;
    }
  else if (range == 2){
    //ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PPRESCALER_DIV64_Val;
    ADC->CTRLB.bit.PRESCALER = 0x4;
    }
  else if (range == 3){
    //ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PPRESCALER_DIV128_Val;
    ADC->CTRLB.bit.PRESCALER = 0x5;
    }
  else if (range == 4){
    //ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PPRESCALER_DIV256_Val;
    ADC->CTRLB.bit.PRESCALER = 0x6;
    }
  else if (range == 5){
    //ADC->CTRLB.bit.PRESCALER = ADC_CTRLB_PPRESCALER_DIV512_Val;
    ADC->CTRLB.bit.PRESCALER = 0x7;
    }

  // Habilita el ADC
  ADCsync();
  ADC->CTRLA.bit.ENABLE = 0x01;

  // Limpia la bandera de 'datos listos'
  ADC->INTFLAG.bit.RESRDY = 1;
    
  // Inicia la conversion
  ADCsync();
  ADC->SWTRIG.bit.START = 1; 
  
}

// Funcion para deshabilitar el ADC luego de que se terminan las lecturas
void disableADC(){

  ADCsync();
  ADC->CTRLA.bit.ENABLE = 0x00;              // Deshabilita el ADC
  ADCsync();
  ADC->SWTRIG.reg = 0x01;                    // Se limpia como buena medida 
  
}
