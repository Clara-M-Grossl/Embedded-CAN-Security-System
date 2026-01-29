# Simulação de Ataques DoS e Replay com PIC16F877A

Este projeto apresenta uma plataforma de simulação de ataques cibernéticos em sistemas embarcados automotivos, com foco em vulnerabilidades de comunicação e mecanismos de recuperação automática.

Devido a limitações do **Proteus**, que não permite a simulação fiel do barramento **CAN**, a comunicação entre os microcontroladores é realizada via **UART**, sendo utilizada como **abstração didática do protocolo CAN automotivo**. Dessa forma, o projeto mantém o foco nas **falhas conceituais de segurança do protocolo**, e não na camada física do barramento.

> [!IMPORTANT]
> Projeto desenvolvido para fins educacionais e que não representa uma implementação completa do protocolo CAN.

---

## Objetivos do Projeto

- Simular vulnerabilidades típicas de comunicação em redes automotivas
- Utilizar UART como meio alternativo para **emular o comportamento do CAN**
- Implementar ataques de:
  - Sniffing de dados
  - Replay Attack
  - Denial of Service (DoS)
- Validar mecanismos de recuperação automática em sistemas embarcados
- Aplicar conceitos de **cibersegurança automotiva** em ambiente de simulação

---

## Visão Geral do Sistema

O sistema é composto por dois microcontroladores **PIC16F877A**, comunicando-se via **UART (TX/RX)**, representando de forma abstrata a troca de mensagens em um barramento CAN.

### PIC Vítima (ECU simulada)
- Representa uma ECU automotiva
- Recebe comandos via UART (emulação de mensagens CAN)
- Controla LEDs/LCD
- Implementa mecanismos de defesa contra falhas e ataques:
  - Watch Dog Timer (WDT)
  - Interrupções por Timer

### PIC Atacante (Fuzzer)
- Atua como dispositivo malicioso conectado ao “barramento”
- Realiza:
  - Sniffing das mensagens UART
  - Armazenamento de pacotes na EEPROM
  - Replay de mensagens capturadas
  - Ataques de DoS por inundação do canal

---

## Comunicação UART como Abstração do CAN

- A UART é utilizada **exclusivamente para fins de simulação**
- O formato das mensagens imita pacotes simples de controle
- As vulnerabilidades exploradas refletem problemas reais do CAN, como:
  - Ausência de autenticação
  - Falta de verificação temporal (timestamp/nonce)
  - Suscetibilidade a Replay e DoS
- O foco do projeto está na **lógica de ataque e defesa**, não na implementação física do protocolo CAN

---

## Ataques Implementados

### Sniffing + Replay Attack
- Captura de mensagens válidas transmitidas pela vítima
- Armazenamento dos dados na EEPROM do atacante
- Reenvio posterior da mensagem capturada
- Demonstra a aceitação de comandos clonados pela ECU

### Denial of Service (DoS)
- Inundação do canal UART com bytes inválidos (`0x00`)
- Saturação do buffer de recepção da vítima
- Provoca:
  - Overrun Error
  - Travamento do sistema
- Intensidade do ataque controlada por potenciômetro

---

## Mecanismos de Defesa

- **Watch Dog Timer (WDT)**  
  - Prescaler 1:128 (~2,3s)
  - Reinicialização automática em caso de travamento

- **Timer por Interrupção**
  - Execução de rotinas independentes do loop principal
  - Aumenta a resiliência sob ataques de estresse

---

## Ambiente de Desenvolvimento

- Microcontrolador: **PIC16F877A**
- Clock: **20 MHz**
- Comunicação: **UART (simulação de CAN)**
- IDE: **MPLAB X**
- Simulação: **Proteus**
- Linguagem: **C (XC8)**

---

## Estrutura do Repositório

```text
/
├── software/
|      ├── fuzzer/            # Código do Fuzzer
|      └── ECUvitima/         # Código da Vítima
├── hardware/
|      ├── fuzzer/            # Simulação Proteus do Fuzzer (.pdsprj)
|      └── ECUvitima/         # Simulação Proteus da Vítima (.pdsprj)
├── docs/                     # Relatório e documentação
└── README.md
```

## Execução da Simulação

- Compile os projetos no MPLAB X
- Gere os arquivos `.hex`
- Abra o projeto no Proteus
- Associe cada `.hex` ao respectivo PIC
- Execute a simulação

## Referências

[1] MICROCHIP TECHNOLOGY. PIC16F87XA Data Sheet

[2] RAGHAVAN, S. S. CAN Bus Security: The Unseen Cybersecurity Battle in Connected Vehicles

[3] EUROSENS. How to Decode Vehicle’s CAN Bus Data

