/*
Instituto Tecnologico De Costa Rica
Escuela de Ing. Electrónica
Laboratprio de Estructuras de micropocesadores
II Semestre 2016.
Profesor: Ronny Garcia.
Estudiantes:
Amit Ferencz Appel 2013389858
Enrique Hernandez Bonilla 2013036085
Daniel León Gamboa 2013036468
Gabriel Madrigal Boza 2013012324

*/


import processing.serial.*; // importa la libreria serial de procesing
import controlP5.*;// importa la liebreria controP5
ControlP5 gui;// crea un ojeto de tipo controlP5
Serial myPort; // crea un ojeto de tipo Serial. 
float xPos = 1;  // Variable que controla la posicion en X de la grafica 
float  [] inByte;// arreglo donde se almacena los datos porvenientes del puerto Serial
float voltaje_instantaneo;// varible que maneja el primer dato mapeado
float I_instanea;// varialbe que maneja el segundo dato mapeado
float VRMS;// varialbe que maneja el tercer dato mapeado
float voltaje_pico;
float IRMS;
float Ipico;
float Preal;
float Paparente;
float FP;
float freq;
float I_vieja=0;
float V_viejo=0;
float ajuste_X;
String s=("Lab. Micros II_2016");
String amit=("Amit Ferencz");
String quique=("Enrique Hernandez");
String leon=("Daniel Leon");
String gabo=("Gabriel Madrigal");
int control = 0;// Variable de cotrol para determine el modo.


//********************* ciclo de setup*****************
void setup () {  
	size(1000,700);
background(0);// pone el fondo de color negro

//********************* creacion de botones *****************
// Esta seccion se escarga de crear los botnos para controlar la escala
// se utliza la biblioteca controlP5
gui = new ControlP5(this);  // crea un objeto
gui.addButton("Modo1")// crea un nuevo boton
.setPosition(width-190,80)//Define las coordenadsa del boton
.setSize(100,20)// define el tamano del boton
.setValue(0)//predefine el valor del boton a 0
.activateBy(ControlP5.RELEASE);// Activa el boton cuadno se libera
;
gui.addButton("Modo0")//crea un nuevo boton
.setPosition(width-190,50)
.setSize(100,20)
.setValue(0)
.activateBy(ControlP5.RELEASE);
;
gui.addButton("Modo2")//crea un nuevo boton.
.setPosition(width-190,110)
.setSize(100,20)
.setValue(0)
.activateBy(ControlP5.RELEASE);
;

//********************* declaracion de puerto serial *****************

myPort = new Serial(this, Serial.list()[5], 115200);
//almacena en el Buffer hasta el enter
myPort.bufferUntil('\n');
}
//********************* Loop general del codigo ***************** //<>//
// Esta en la parte del codigo que se ejecuta constantemente, se encarga de toda la parte visual,
void draw () {
line(0, height/10*i, width-200, height/10*i);// crea la linea 
fill(255,255,255);//elige el color del texto
textSize(14);//define el tamano del texto
if(control == 0){//Selector de escala
	for (int i = 0; i <= 13; i = i+1) {//for donde se crean las lines
	text(50-10*i,2,height/10*i);//escribe los numero de las lineas
} 
}
else if (control == 1){
	for (int i = 0; i <= 11; i = i+1) {
		text(25-5*i,2,height/10*i);
	}
}
else if (control == 2){
	for (int i = 0; i <= 11; i = i+1) {

		text(10-2*i,2,height/10*i);
	}
}
//********************* Crea un cuadrado negro para borrar los numeros instaneo *****************
stroke(0);//cuadrado sin linea
fill(0,0,0);/// color de relleno negro
rect(width-190,190,190,height-190);//crea el rectan //<>//
//********************* Impreme valores instaneo *****************
fill(0, 102, 153);// asigna  colo de la letra
textSize(18);// tama;o del texto
text("Voltaje RMS",width-190,160);// titulo 
text(VRMS,width-190,190);//impreme el valor "actual" de 
text("Volaje pico ",width-190,220);
text(voltaje_pico,width-190,250);
text("Corriente RMS",width-190,280);
text(IRMS,width-190,310);
text("Corriente",width-190,340);
text(Ipico,width-190,370);
text("Potencia Real",width-190,400);
text(Preal,width-190,430);
text("Potencia aparente",width-190,460);
text(Paparente,width-190,490);
text("Potencia aparente",width-190,510);
text(FP,width-190,540);
text(s,width-190, 570);
text(amit,width-190, 590);
text(quique,width-190, 610);
text(leon,width-190, 630);
text(gabo,width-190, 650);
stroke(255,255,255);
strokeWeight(1);
line(xPos, -1*(V_viejo-height) , xPos+ajuste_X, -1*(voltaje_instantaneo+4-height)); // dibula la linea entre los puntos 
V_viejo=voltaje_instantaneo; //Cambia el valor actual al "viejo" para poder hacer el grafico con lineas
stroke(150,255,0);// cambia de color 
line(xPos, -1*( I_vieja-height) , xPos+ajuste_X, -1*( I_instanea+4-height));// Dibuja la linea de la correinte 
I_vieja = I_instanea;// cambia el valor actua al valor viejo
strokeWeight(1);//cambia el grososr de la linea
fill(0,0,0); 
stroke(255,255,255);
textSize(14);
line(0, height/10*i, width-200, height/10*i);
// esta seccion se encarga de revisar si ya se llego al final de la region de graficado, si llego al final, borra la pantalla y vuelve poner los numeros y las lineas horizontales 
if (xPos >= width-200) {// 
	xPos = 0;
	background(0);
	if(control == 0){
		for (int i = 0; i <= 11; i = i+1) {
			text(50-10*i,2,height/10*i);
		} 
	}
	else if (control == 1){
		for (int i = 0; i <= 11; i = i+1) {
			text(25-5*i,2,height/10*i);
		}
	}
	else if (control == 2){
		for (int i = 0; i <= 11; i = i+1) {
			text(10-2*i,2,height/10*i);
		}
	}
}
else {
// ------------- ajueste de la componente X-----------------------------
// En esta seccion se evalua la frecuencia que proviene del circuito y se varia el espaciado entre los puntos para tener una merjor percepscion 
	if((3000 >= freq ) && (freq > 2000) ){
		ajuste_X=13;
	}
	else if((2000 >= freq ) && (freq > 1000) ){
		ajuste_X=10;
	}
	else if( (1000 >= freq ) && (freq > 500) ){
		ajuste_X=7;
	}
	else if((500 >= freq ) && (freq > 200)){
		ajuste_X=5;
	}
	else if((4000 >= freq ) && (freq > 3000)){
		ajuste_X=17;
	}
	else if((freq > 4000)){
		ajuste_X=20;
	}
	else{
		ajuste_X=1;
	}
	xPos= xPos + ajuste_X;// se cambia el valor inicial del nuevo punto
}
}

//--------------------------- Adquiscion de datos seriales--------------------
// En esta secion es donde se adquiere los datos del puerto serial.
// Esta funcion se encarga de corta los datos hasta encontrar un "\n"
// Una vez que obitne la "tira " de datos lo guarda en un verctor separados por " "
// esta funcion tambien utilza una funcion de processing que mapea el valor a una valor relazionada con la altura del pantalla 

void serialEvent (Serial myPort) {


	String inString = myPort.readStringUntil('\n');// Corta la informacion depues de \n

	if (inString != null) {
//Revisa que la comunicacion este estable
		inString = trim(inString);
// convert to an int and map to the screen height:
		inByte = float(split(inString,' '));
		if(control==0){// determina la escala 
			voltaje_instantaneo = map(inByte[0], -50, 50, 0, height);//mapa el valor de entrada en una valor dentro del tamano la pantalla
			I_instanea = map(inByte[1], -50, 50, 0, height);
		}
		if(control==1){
			voltaje_instantaneo = map(inByte[0], -20, 20, 0, height);
			I_instanea = map(inByte[1], -50, 50, 0, height);
		}
		if(control==2){
			voltaje_instantaneo = map(inByte[0], -10, 10, 0, height);
			I_instanea = map(inByte[1], -50, 50, 0, height);

		}
		VRMS = inByte[2];
		voltaje_pico = inByte[3];
		IRMS = inByte[4];
		Ipico = inByte[5];
		Preal = inByte[6];
		Paparente = inByte[7];
		FP = inByte[8];
		freq = inByte[9];
		println(freq);

	}
}

//---------------------------- Control del los botones----------------------
// Estas funciones se encargan de controlar las acciones de cada boton.
public void Modo0(int value){
control =0;
xPos = 0;
background(0);
}
public void Modo1(int value){
control =1;
xPos = 0;
background(0);
}
public void Modo2(int value){
control =2;
xPos = 0;
background(0);
}

public void controlEvent(ControlEvent theEvent) {

}