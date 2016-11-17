//  Test routine heavily inspired by wiring_analog.c from GITHUB site
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

// This is an C/C++ code to insert repetitive code sections in-line pre-compilation
// Wait for synchronization of registers between the clock domains
// ADC
static __inline__ void ADCsync() __attribute__((always_inline, unused));
static void   ADCsync() {
  while (ADC->STATUS.bit.SYNCBUSY == 1); //Just wait till the ADC is free
}

// Variables defined
uint32_t Status = 0x00000000;
  uint32_t ulPin_V = A1;      //This is the analog pin to read voltage
uint32_t ulPin_C = A2;      //This is the analog pin to read current
uint32_t rawDataV[50], rawDataI[50];           // variable to store the value read

int pinCrucePorCero = 11;
unsigned long pulseLenght, freq;
int pinADCRead, range;

float Vvector[50];
float Ivector[50];

float V;
float I;

void setup()
{
  Serial.begin(9600);          //  setup serial
  pinMode(PIN, OUTPUT);        // setup timing marker

  pinMode(pinCrucePorCero, INPUT);

  //###################################################################################
  // ADC setup stuff
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

    pulseLenght = pulseIn(pinCrucePorCero, HIGH);
    freq = 272480*pow(pulseLenght,-0.909);
    
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

  configADC(1, range);

  for (int i=0; i<50; i++){
      rawDataV[i] = continuousADCRead();
    }
    
  configADC(2, range);

  for (int i=0; i<50; i++){
      rawDataI[i] = continuousADCRead();
    }

  //---------------------Conversion----------------------------

     for (int i=0; i<=50; i++){ //Para tener 50 muestras
        V = rawDataV[i] * ((3.3)/1023.0);
        Vvector[i] = V;         
      }


     for (int i=0; i<=50; i++){ //Para tener 50 muestras
        I = rawDataI[i] * ((3.3)/1023.0);
        Ivector[i] = I;         
      }

//-------------------Valores Pico-----------------------

float max_auxV, max_auxI, Vpico, Ipico;
max_auxV = 0;
max_auxI = 0;

    for(int i=0; i<50; i++){
      max_auxV = max(max_auxV, Vvector[i]);
      }

    for(int i=0; i<50; i++){
      max_auxI = max(max_auxI, Ivector[i]);
      }

Vpico = max_auxV;
Ipico = max_auxI;

//-------------------RMS--------------------------------

  float auxV = 0;
  float auxI = 0;

    for (int i=0; i<=50; i++){
      
      auxV = auxV + ((Vvector[i])*(Vvector[i]));
      auxI = auxI + ((Ivector[i])*(Ivector[i]));
      
    }
    
  float aux1V = auxV/50 ;
  float Vrms = sqrt(aux1V);
  float aux1I = auxI/50 ;
  float Irms = sqrt(aux1I);
  
//------------ Pontencia Promedio------------------- 

  float auxP = 0;
  float auxP1 = 0;
  
    for (int i=0; i<=50; i++){
        
      auxP1 = auxP + ((Vvector[i])*(Ivector[i]));
      
    }

  float PotProm = auxP1/50 ;

//--------------------Potencia Aparente----------------------

  float P_Aparente = ((Vrms)*(Irms));

//------------------Factor de Potencia-----------------------

  float FP = ((PotProm)/(P_Aparente));

//----------------Envio de los datos------------------------
  
  for(int i=0; i<50; i++){
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
      Serial.println(FP);
  }
  
}

uint32_t continuousADCRead() {
  
  ADCsync();

  while ( ADC->INTFLAG.bit.RESRDY == 0 );   // Wait till conversion done
  ADCsync();
  uint32_t valueRead = ADC->RESULT.reg;

  ADC->INTFLAG.bit.RESRDY = 1;              // Data ready flag cleared

  return valueRead;
}

void configADC(int pin, int range){

  // Configura el pind desde el que se lee (voltaje o corriente)
  ADCsync();
  if (pin == 1){
    ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[ulPin_V].ulADCChannelNumber; 
  }
  else if (pin == 2){
    ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[ulPin_C].ulADCChannelNumber;
  }

  // Configura la frecuencia para el modo 'free running'
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

void disableADC(){

  ADCsync();
  ADC->CTRLA.bit.ENABLE = 0x00;             // Disable the ADC 
  ADCsync();
  ADC->SWTRIG.reg = 0x01;                    //  and flush for good measure
  
}

