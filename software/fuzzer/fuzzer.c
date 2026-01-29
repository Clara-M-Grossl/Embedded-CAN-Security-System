/*
 * File:   fuzzer.c
 * Author: clara
 * Created on 20 de Novembro de 2025, 15:22
 */

#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma config FOSC = HS, WDTE = ON, PWRTE = ON, BOREN = OFF, LVP = OFF, CPD = OFF, WRT = OFF, CP = OFF
#define _XTAL_FREQ 20000000 //FREQUENCIA DE 20 MHz


//ENTRADAS
#define BT_ABORT  PORTBbits.RB0 
#define BT_MENU   PORTBbits.RB1
#define BT_SELECT PORTBbits.RB2 
#define BT_KILL   PORTBbits.RB4 


// LCD
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7


//VARIAVEIS GLOBAIS
volatile int abortar = 0;
int menu = 0; 

void EEPROM_Write(unsigned char addr, unsigned char data) {
    while(EECON1bits.WR) CLRWDT();
    EEADR=addr; EEDATA=data; EECON1bits.EEPGD=0; EECON1bits.WREN=1;
    INTCONbits.GIE=0; EECON2=0x55; EECON2=0xAA; EECON1bits.WR=1;
    while(EECON1bits.WR) CLRWDT();
    INTCONbits.GIE=1; EECON1bits.WREN=0;
}
unsigned char EEPROM_Read(unsigned char addr) {
    while(EECON1bits.WR) CLRWDT(); EEADR=addr; EECON1bits.EEPGD=0; EECON1bits.RD=1; return EEDATA;
}

void Lcd_Port(char a){ if(a&1)D4=1;else D4=0; if(a&2)D5=1;else D5=0; if(a&4)D6=1;else D6=0; if(a&8)D7=1;else D7=0; }
void Lcd_Cmd(char a){ RS=0;Lcd_Port(a);EN=1;__delay_ms(4);EN=0; }
void Lcd_Clear(){ Lcd_Cmd(0);Lcd_Cmd(1); }
void Lcd_Set_Cursor(char a, char b){ char t,z,y; if(a==1)t=0x80+b-1;else if(a==2)t=0xC0+b-1;else if(a==3)t=0x94+b-1;else t=0xD4+b-1; z=t>>4; y=t&0x0F; Lcd_Cmd(z);Lcd_Cmd(y); }
void Lcd_Write_Char(char a){ char t,y; t=a&0x0F; y=a&0xF0; RS=1; Lcd_Port(y>>4); EN=1; __delay_us(40); EN=0; Lcd_Port(t); EN=1; __delay_us(40); EN=0; }
void Lcd_Write_String(char *a){ int i; for(i=0;a[i]!='\0';i++)Lcd_Write_Char(a[i]); }
void Lcd_Init(){ Lcd_Port(0);__delay_ms(20);Lcd_Cmd(3);__delay_ms(5);Lcd_Cmd(3);__delay_ms(11);Lcd_Cmd(3);Lcd_Cmd(2);Lcd_Cmd(2);Lcd_Cmd(8);Lcd_Cmd(0);Lcd_Cmd(0x0C);Lcd_Cmd(0);Lcd_Cmd(0x06); }

void UART_Init(long baud){ SPBRG=(unsigned char)((_XTAL_FREQ/(16*baud))-1); TXSTAbits.TXEN=1; TXSTAbits.BRGH=1; RCSTAbits.SPEN=1; RCSTAbits.CREN=1; }
void UART_Write(char d){ while(!TXSTAbits.TRMT) CLRWDT(); TXREG=d; }
void UART_Write_String(char *s){ while(*s) UART_Write(*s++); }
char UART_Read(){ while(!PIR1bits.RCIF) CLRWDT(); if(RCSTAbits.OERR){RCSTAbits.CREN=0;RCSTAbits.CREN=1;} return RCREG; }
unsigned char UART_Data_Ready(){ return PIR1bits.RCIF; }

void ADC_Init(){ ADCON0=0x41; ADCON1=0x8E; }
unsigned int ADC_Read(){ __delay_us(20); GO_nDONE=1; while(GO_nDONE) CLRWDT(); return((unsigned int)ADRESH<<8)+ADRESL; }

void __interrupt() ISR(void) {
    if(INTCONbits.INTF) { abortar=1; INTCONbits.INTF=0; }
}

void main(void) {
    TRISD=0; 
    TRISB=0xFF; 
    OPTION_REGbits.nRBPU=0; 
    OPTION_REGbits.INTEDG=0;
    INTCONbits.GIE=1; 
    INTCONbits.INTE=1; 
    INTCONbits.PEIE=1;

    //INICIALIZA��O
    UART_Init(9600); 
    Lcd_Init(); 
    ADC_Init(); 
    Lcd_Clear();

    Lcd_Set_Cursor(1,1); 
    Lcd_Write_String("CAN ATTACK SIMULATOR");
    Lcd_Set_Cursor(4,1); 
    //INICIALIZA��O DE BOOTING
    for(int i=0;i<20;i++){ 
        Lcd_Write_Char(0xFF); 
        __delay_ms(50); 
        CLRWDT(); 
    }
    Lcd_Clear();

    Lcd_Set_Cursor(1,1); 
    Lcd_Write_String("CAN ATTACK SIMULATOR");
    Lcd_Set_Cursor(4,1); 
    Lcd_Write_String("[MENU] TO START");

    while(1) {
        CLRWDT();
        if(abortar) { //INTERRUP��O
            Lcd_Clear(); 
            Lcd_Set_Cursor(2,1); 
            Lcd_Write_String("!!! ABORTED !!!");
            __delay_ms(1000); 
            abortar=0; 
            menu=0; 
            Lcd_Clear();
            Lcd_Set_Cursor(1,1); 
            Lcd_Write_String("CAN ATTACK SIMULATOR");
            Lcd_Set_Cursor(4,1); 
            Lcd_Write_String("[MENU] TO START");
        }

        if(BT_MENU == 0) {
            __delay_ms(200); 
            while(BT_MENU==0) CLRWDT();
            menu++; 
            if(menu>3) menu=1;
            
            Lcd_Set_Cursor(2,1); 
            Lcd_Write_String("                    ");
            Lcd_Set_Cursor(3,1); 
            Lcd_Write_String("                    ");
            Lcd_Set_Cursor(4,1); 
            Lcd_Write_String("                    ");

            if(menu==1) { 
                Lcd_Set_Cursor(2,1); 
                Lcd_Write_String("> 1. SNIFFER KEY"); 
                Lcd_Set_Cursor(4,1); 
                Lcd_Write_String("[SEL] TO RECORD"); 
            }
            if(menu==2) { 
                Lcd_Set_Cursor(2,1); 
                Lcd_Write_String("> 2. REPLAY KEY"); 
                Lcd_Set_Cursor(4,1); 
                Lcd_Write_String("[SEL] TO INJECT"); 
            }
            if(menu==3) { 
                Lcd_Set_Cursor(2,1); 
                Lcd_Write_String("> 3. DoS ATTACK"); 
                Lcd_Set_Cursor(3,1); 
                Lcd_Write_String("USE POT TO FLOOD");
                Lcd_Set_Cursor(4,1); 
                Lcd_Write_String("[SEL] TO START");
            }
        }

        if(BT_SELECT == 0 && !abortar) {
            // SNIFFER: CAPTURA E GRAVA NA MEMORIA
            if(menu==1) {
                Lcd_Set_Cursor(3,1); 
                Lcd_Write_String("LISTENING...");
                while(BT_SELECT==0) CLRWDT();
                while(!abortar) {
                    CLRWDT(); 
                    if(BT_MENU==0) break;
                    if(UART_Data_Ready()) {
                        char c = UART_Read();
                        if(c=='u' || c=='l') { 
                            EEPROM_Write(0, c); 
                            Lcd_Set_Cursor(3,1); 
                            Lcd_Write_String("CAPTURED: KEY   "); 
                            if(c=='u') { 
                                Lcd_Set_Cursor(4,1); 
                                Lcd_Write_String("TYPE: UNLOCK    "); 
                            }
                            else { 
                                Lcd_Set_Cursor(4,1); 
                                Lcd_Write_String("TYPE: LOCK      "); 
                            }
                            __delay_ms(1500); break; 
                        }
                    }
                }
            }
            Lcd_Set_Cursor(2,1); 
            Lcd_Write_String("> 1. SNIFFER KEY"); 
            Lcd_Set_Cursor(3,1); 
            Lcd_Write_String("                    ");
            
            // REPLAY: PEGA DA MEM�RIA E APLICA NA UART
            if(menu==2) {
                while(BT_SELECT==0) CLRWDT();
                char k = EEPROM_Read(0);
                if(k == 'u' || k == 'l') {
                    Lcd_Set_Cursor(3,1); 
                    Lcd_Write_String("INJECTING KEY...");
                    UART_Write(k);
                    __delay_ms(500); 
                    Lcd_Set_Cursor(3,1); 
                    Lcd_Write_String("DONE.           ");
                } else {
                    Lcd_Set_Cursor(3,1);
                    Lcd_Write_String("ERROR: NO KEY!  "); 
                    __delay_ms(1000);
                }
            }
            Lcd_Set_Cursor(2,1); 
            Lcd_Write_String("> 2. REPLAY KEY"); 
            Lcd_Set_Cursor(3,1); 
            Lcd_Write_String("                    ");
            
            // DoS
            if(menu==3) { //INTENSIDADE DO ATAQUE COM BASE NO POTENCIOMETRO
                Lcd_Set_Cursor(3,1); 
                Lcd_Write_String("ADJUST INTENSITY");
                __delay_ms(500);
                while(BT_SELECT==0) CLRWDT();

                while(!abortar && BT_MENU==1) {
                    CLRWDT();
                    unsigned int pot = ADC_Read();
                    int intes = (int)((unsigned long)pot * 100 / 1023);
                    char s[20]; 
                    sprintf(s, "LOAD: %d%% [KILL]", intes);
                    Lcd_Set_Cursor(3,1); 
                    Lcd_Write_String(s);
                    if(BT_KILL == 0) {
                        while(BT_KILL == 0 && !abortar) {
                            if(intes <= 79) { //ATAQUE MAIS LEVE
                                Lcd_Set_Cursor(4,1); 
                                Lcd_Write_String(">>> SENDING ATTACK <<<");
                                UART_Write('B');
                            }
                            else if(intes >= 80 && intes < 95){ //ATAQUE MAIS FORTE
                                Lcd_Set_Cursor(4,1); 
                                Lcd_Write_String(">>> KILL <<<");
                                UART_Write('C');
                            }
                            else { //KILL TOTAL DO SITEMA POR ID DE ALTA PRIORIDADE
                                Lcd_Set_Cursor(4,1); 
                                Lcd_Write_String(">>> KILL <<<");
                                UART_Write(0x00);
                            }
                            
                            __delay_ms(50); CLRWDT();
                        }
                        Lcd_Set_Cursor(4,1); 
                        Lcd_Write_String("                       ");
                    }
                }
            }
        }        
    }
}