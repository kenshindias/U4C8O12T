# Projeto: Controle de LEDs e Display com Joystick

Este projeto utiliza um Raspberry Pi Pico para controlar LEDs RGB e um display OLED via um joystick analógico. O código lê os valores do joystick, calibra sua posição central e usa suas leituras para alterar a intensidade dos LEDs e exibir uma interface gráfica no display OLED.

## Funcionalidades
- Controle da intensidade dos LEDs RGB baseado no movimento do joystick.
- Alternância entre borda simples e dupla no display OLED ao pressionar o botão do joystick.
- Ativação e desativação do PWM dos LEDs ao pressionar o botão A.
- Exibição da posição do joystick em um display OLED.
- Calibração automática do joystick ao iniciar o sistema.

## Demonstração
Confira o funcionamento do projeto neste vídeo:

[![Vídeo de demonstração]()]()

## Requisitos de Hardware
- Raspberry Pi Pico
- Joystick analógico com botão
- Display OLED SSD1306 (I2C)
- LEDs RGB (com suporte a PWM)
- Resistores conforme necessário
- Fios e protoboard para conexões

## Conexões
### Joystick:
- **Eixo X**: GPIO 27 (ADC0)
- **Eixo Y**: GPIO 26 (ADC1)
- **Botão**: GPIO 22

### Botão A:
- **Pino**: GPIO 5

### LEDs:
- **LED Vermelho (PWM)**: GPIO 13
- **LED Verde**: GPIO 11
- **LED Azul (PWM)**: GPIO 12

### Display OLED (SSD1306 via I2C):
- **SDA**: GPIO 14
- **SCL**: GPIO 15
- **Endereço I2C**: 0x3C

## Instalação e Uso
1. Clone este repositório e compile o código para o Raspberry Pi Pico.
2. Conecte os dispositivos conforme a seção "Conexões".
3. Faça o upload do firmware para o Pico.
4. O sistema calibra automaticamente o joystick na inicialização.
5. Mova o joystick para controlar os LEDs e a interface do display.

## Controles
- **Movimento do Joystick**: Altera a intensidade dos LEDs R e B.
- **Botão do Joystick (GPIO 22)**: Alterna entre borda simples e dupla no display.
- **Botão A (GPIO 5)**: Liga/desliga o controle PWM dos LEDs.

## Licença
Este projeto é de código aberto e pode ser usado livremente para fins educacionais ou pessoais.  
Sinta-se à vontade para contribuir ou modificar conforme necessário!

---

Se você tiver dúvidas ou sugestões, **abra uma issue ou envie um pull request!** 
Autor: **@kenshindias**
