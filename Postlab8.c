/* 
 * File:   postlab8.c
 * Author: ANGELA
 *
 * Created on 19 de abril de 2023, 21:01
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)
// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>
#include <stdio.h>
#include <stdint.h>


//definicion de frecuencia para delay
#define _XTAL_FREQ 1000000 // Frecuencia para delays de 1MHz

//variables globales
int state_flag = 0;//bandera para el estado en edicion
uint8_t pot1;//variable para leer potenciometro
uint8_t opt_sel;//variable para almacenar opcion seleccionada
uint8_t portb_char;//variable para almacenar valor dado por interfaz
unsigned char vacio = '\r';//variable con caracter de enter para formatear texto

//funciones proto
void setup(void);//funcion setup
void item_list(void);//funcion para menu principal
void enter(int a);//funcion para colocar una cantidad establecida de enters

//configuracion principal
void setup(void){
    ANSEL = 0b00000001; 
    ANSELH = 0;
    
    TRISA = 0b00000001;
    TRISB = 0;
    PORTA = 0; 
    PORTB = 0; 
    
    //config de interrupciones
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
    
    //OSCCON
    OSCCONbits.IRCF = 0b0100;
    OSCCONbits.SCS = 1;
    
    //config ADC
    ADCON0bits.ADCS = 0b01;
    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0;
    
    ADCON0bits.CHS = 0b0000;
    ADCON1bits.ADFM = 0;
    ADCON0bits.ADON = 1;
    __delay_us(40);
    
    //config serial 
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 1;
    BAUDCTLbits.BRG16 = 1;
    SPBRGH = 0;
    SPBRG = 25;
    RCSTAbits.SPEN = 1;
    TXSTAbits.TXEN = 1;
    RCSTAbits.CREN = 1;
    PIE1bits.RCIE = 1;
}

//funcion para enters de cantidad especifica
void enter(int a){
    while(a>0){
        a--;
        __delay_ms(40);
        TXREG = vacio;
    }
}

//funcion de impresion
void tprint(char *string){
    while(*string != '\0'){
        while(TXIF != 1);
        TXREG = *string;
        *string++;
    }
}

//interrupciones
void __interrupt() isr(void){
    if (PIR1bits.ADIF){
        if (ADCON0bits.CHS == 0){
            pot1 = ADRESH;
        }
        PIR1bits.ADIF = 0;
    }
    if(RCIF){
        if(state_flag == 0){
            opt_sel = RCREG;
            RCREG = 0;
        }
        else if (state_flag == 1){
            state_flag = 0;
            portb_char = RCREG;
            TXREG = portb_char;
            PORTB = portb_char;
            RCREG = 0;
            enter(1);
            item_list();
        }
    }
    return;
}

void main(void){
    //config ejecutar
    setup();
    item_list();
    
    //loop main
    while(1){
        if (ADCON0bits.GO == 0){
            ADCON0bits.GO = 1;
        }
        if (opt_sel == 0x61){
            opt_sel = 0x00;
            
            tprint("Valor del potenciometro: ");
            __delay_ms(500);
            TXREG = pot1;
            __delay_ms(500);
            item_list();
        }
        else if (opt_sel == 0x62){
            state_flag = 1;
            tprint("Ingrese caracter en ASCII: ");
            opt_sel = 0x00;
        }
    }
    return;
}

//menu en hiperterminal
void item_list(){
    enter(2);//enter
    tprint("---------MAIN MENU----------");
    enter(1);
    tprint("a. Lectura de potenciometro");
    enter(1);
    tprint("b. Enviar ASCII");
    enter(2);
    
}





