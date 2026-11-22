#pragma once

#include <Arduino.h>
#include <LovyanGFX.hpp>

// Credenciais do Wi-Fi gravadas na memória da placa e configuração pela tela de toque
namespace wifi_setup {

// Retorna false se ainda não há rede configurada
bool loadCredentials(String& ssid, String& password);

// Fluxo interativo: escolher a rede, digitar a senha e conectar. Só retorna depois de conectar.
void run(lgfx::LGFX_Device& display, LGFX_Sprite& canvas);

}  // namespace wifi_setup
