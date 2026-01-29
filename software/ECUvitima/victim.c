/* File:   vitima.c
 * Author: clara
 * Created on 20 de Novembro de 2025, 15:26
 */

#include <xc.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

// CONFIGURAÇÕES
#pragma config FOSC=HS, WDTE=ON, PWRTE=ON, BOREN=OFF, LVP=OFF, CPD=OFF, WRT=OFF, CP=OFF
#define _XTAL_FREQ 20000000 // FREQUENCIA DE 20MHz

// PINOS
#define KEY      RB0 
#define LED_G    RB1 
#define LED_Y    RB2 
#define BUZZER   RB3 
#define LED_R    RB4 
#define LED_T    RB5 


//LCD
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7


//VARIAVEIS GLOBAIS
char buffer[20];
int trancado=1, modo_hack=0;
int kmh_simulado = 0;  
volatile int contagem_tempo = 0;
volatile int flag_enviar_dados = 0; //CONTROLAR O TEMPO DE ENVIO POR MEIO DO TIMER => MAIS PRECISO QUE UM CONTADOR i++

void Leds_Off(){ LED_G=0; LED_Y=0; LED_R=0; LED_T=0; BUZZER=0; }
void Lcd_Port(char a){ if(a&1)D4=1;else D4=0; if(a&2)D5=1;else D5=0; if(a&4)D6=1;else D6=0; if(a&8)D7=1;else D7=0; }
void Lcd_Cmd(char a){ RS=0;Lcd_Port(a);EN=1;__delay_ms(4);EN=0; }
void Lcd_Clear(){ Lcd_Cmd(0);Lcd_Cmd(1); }
void Lcd_Set_Cursor(char a, char b){ char t,z,y; if(a==1){t=0x80+b-1;z=t>>4;y=t&0x0F;Lcd_Cmd(z);Lcd_Cmd(y);} else{t=0xC0+b-1;z=t>>4;y=t&0x0F;Lcd_Cmd(z);Lcd_Cmd(y);} }
void Lcd_Write_String(char *a){ int i; for(i=0;a[i]!='\0';i++){ char t=a[i]&0x0F,y=a[i]&0xF0;RS=1;Lcd_Port(y>>4);EN=1;__delay_us(40);EN=0;Lcd_Port(t);EN=1;__delay_us(40);EN=0; } }
void Lcd_Init(){ Lcd_Port(0);__delay_ms(20);Lcd_Cmd(3);__delay_ms(5);Lcd_Cmd(3);__delay_ms(11);Lcd_Cmd(3);Lcd_Cmd(2);Lcd_Cmd(2);Lcd_Cmd(8);Lcd_Cmd(0);Lcd_Cmd(0x0C);Lcd_Cmd(0);Lcd_Cmd(0x06); }

void UART_Init(long baud){ SPBRG=(unsigned char)((_XTAL_FREQ/(16*baud))-1); TXSTAbits.BRGH=1; TXSTAbits.TXEN=1; RCSTAbits.CREN=1; RCSTAbits.SPEN=1; }
void UART_Write(char d){ while(!TXSTAbits.TRMT) CLRWDT(); TXREG=d; }
void UART_Write_String(char *s){ while(*s){ UART_Write(*s++); } }
char UART_Read(){ while(!PIR1bits.RCIF) CLRWDT(); if(RCSTAbits.OERR){RCSTAbits.CREN=0;RCSTAbits.CREN=1;} return RCREG; }

void ADC_Init(){ ADCON0=0x41; ADCON1=0x8E; }
unsigned int ADC_Read(){ GO_nDONE=1; while(GO_nDONE) CLRWDT(); return((ADRESH<<8)+ADRESL); } 

void Timer1_Init() { T1CON=0b00110001; TMR1H=0x0B; TMR1L=0xDC; PIE1bits.TMR1IE=1; INTCONbits.PEIE=1; INTCONbits.GIE=1; }
void __interrupt() ISR() { 
    if(PIR1bits.TMR1IF) { 
        PIR1bits.TMR1IF = 0; 
        TMR1H = 0x0B; 
        TMR1L = 0xDC; 

        contagem_tempo++; 
        if(contagem_tempo >= 5){
            flag_enviar_dados = 1;
            contagem_tempo = 0;  
        }
    } 
}


void main(){
    TRISD=0; 
    TRISB=0b00000001; // RB0 Entrada, RB1-RB7 Saida
    OPTION_REGbits.nRBPU=0; // Pull-ups ON

    Leds_Off();
    LED_T = 1;
    
    UART_Init(9600); 
    Lcd_Init(); 
    Lcd_Clear(); 
    ADC_Init(); 
    Timer1_Init(); 

    Lcd_Set_Cursor(1,1); 
    Lcd_Write_String("STATUS: LOCKED  ");

    while(1){
        CLRWDT(); 

        // --- CHAVE ---
        if(KEY==0){
            __delay_ms(150); 
            if(KEY==0){ 
                trancado = !trancado; 
                if(trancado){
                    Leds_Off(); LED_T=1; 
                    Lcd_Set_Cursor(1,1); 
                    Lcd_Write_String("STATUS: LOCKED  ");
                    UART_Write('l');
                } else {
                    Leds_Off(); LED_G=1; 
                    Lcd_Set_Cursor(1,1); 
                    Lcd_Write_String("STATUS: UNLOCKED");
                    UART_Write('u');;
                }
                while(KEY==0) CLRWDT();
            }
        }

        // ---VELOCIDADE ---
        if(!trancado){
            int pot = ADC_Read(); 
            kmh_simulado = (int)((unsigned long)pot * 220 / 1023); //converter para km/h

            if(flag_enviar_dados == 1){
                char pkt[15]; 
                sprintf(pkt, "#100:%d$", kmh_simulado); 
                UART_Write_String(pkt); 

                char s[16]; 
                sprintf(s, "VEL: %d Km/h    ", kmh_simulado);
                Lcd_Set_Cursor(2,1); 
                Lcd_Write_String(s);

                flag_enviar_dados = 0; 
            }
        } else {
            Lcd_Set_Cursor(2,1); 
            Lcd_Write_String("ENGINE OFF      ");
        }

        // ---COMANDO EXTERNO ---
        if(PIR1bits.RCIF){
            char c = UART_Read();
            
            if(c == 'u') { // Unlock
                trancado = 0; 
                Leds_Off();
                LED_G = 1; 
                Lcd_Set_Cursor(1,1); 
                Lcd_Write_String("REMOTE UNLOCK   ");
            }
            else if(c == 'l') { // Lock
                trancado = 1; 
                Leds_Off(); 
                LED_T = 1; 
                Lcd_Set_Cursor(1,1); 
                Lcd_Write_String("REMOTE LOCK     ");
            }
            
            else if(c == 'B') { // Nivel Medio
                Leds_Off(); 
                LED_Y=1; 
                Lcd_Clear();
                Lcd_Set_Cursor(1,1); 
                Lcd_Write_String("BUS WARNING!");
                Lcd_Set_Cursor(2,1); 
                Lcd_Write_String("WDT RECOVER...");
                for(int k=0;k<30;k++){ 
                    __delay_ms(100); 
                    CLRWDT(); 
                }
                while(1); 
            }
            
            else if(c == 'C') {
                Leds_Off(); 
                LED_R=1; 
                BUZZER=1;
                Lcd_Clear(); 
                Lcd_Set_Cursor(1,1); 
                Lcd_Write_String("CRITICAL FAIL!");
                Lcd_Set_Cursor(2,1); 
                Lcd_Write_String("WDT RECOVER...");
                for(int k=0;k<30;k++){ 
                    __delay_ms(100); 
                    CLRWDT(); 
                }
                while(1); 
            }

            // --- DoS KILL (0x00) ---
            else if(c == 0x00) {
                Leds_Off(); 
                LED_R=1; 
                Lcd_Clear(); 
                Lcd_Set_Cursor(1,1); 
                Lcd_Write_String("!! FATAL ERROR !!");
                while(1) { 
                    LED_R = 0;
                    __delay_ms(20);
                    LED_R = 1;
                    LED_Y = 0;
                    __delay_ms(20);
                    LED_Y = 1;
                    LED_G = 0;
                    __delay_ms(20);
                    LED_G = 1;
                    BUZZER = 1; 
                    __delay_ms(20);
                    BUZZER= 0;
                    CLRWDT(); 
                }
            }
        }
    }
}