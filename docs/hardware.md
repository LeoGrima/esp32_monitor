# Hardware: módulo ES3C28P

Resumo do manual técnico (LCDWIKI, *ES3C28P&ES3N28P Specification*, V1.0) com o que o firmware usa.

| Item | Especificação |
|---|---|
| Controlador | ESP32-S3 (dual-core 240 MHz), 16 MB de flash QSPI, 8 MB de PSRAM OPI |
| Tela | IPS 2.8", 240x320, driver ILI9341V, SPI de 4 fios |
| Toque | Capacitivo FT6336G, I2C (endereço 0x38) — somente no ES3C28P |
| Alimentação | USB-C 5 V (também é a porta de gravação, USB nativa do ESP32-S3) |

## Pinos usados pelo firmware

| Função | GPIO | Observação |
|---|---|---|
| LCD CS | 10 | ativo em nível baixo |
| LCD DC | 46 | alto = dado, baixo = comando |
| LCD SCLK | 12 | |
| LCD MOSI | 11 | |
| LCD MISO | 13 | |
| LCD reset | CHIP_PU | compartilhado com o reset do ESP32-S3 |
| LCD backlight | 45 | alto = ligado; aceita PWM |
| Toque SDA | 16 | barramento I2C compartilhado com o codec de áudio |
| Toque SCL | 15 | |
| Toque reset | 18 | ativo em nível baixo |
| Toque interrupção | 17 | |
| Amplificador de áudio | 1 | baixo = ligado; o firmware mantém desligado |
| Tensão da bateria | 9 | ADC, por divisor resistivo |
| Botão BOOT | 0 | usado para avançar de tela |

## Pinos disponíveis e não usados

| Função | GPIO |
|---|---|
| LED RGB (WS2812) | 42 |
| Cartão microSD (SDIO) | 38 CLK, 40 CMD, 39 D0, 41 D1, 48 D2, 47 D3 |
| Áudio I2S | 4 MCLK, 5 BCLK, 6 DOUT, 7 LRCK, 8 DIN |
| Serial | 43 TX, 44 RX |
| Expansão | 2, 3, 14, 21 |

## Problemas comuns (do manual)

- **Tela apagada após gravar:** o backlight só acende com o GPIO45 em nível alto; confira se a gravação terminou sem erro.
- **Backlight aceso e sem imagem:** driver ou pinos incorretos. Se as cores saírem em negativo, troque `kDisplayInvertColors` em `include/app_config.h`.
- **Gravação falha:** segure **BOOT**, pressione e solte **RESET**, solte **BOOT** e grave de novo.
