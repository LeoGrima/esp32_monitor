#pragma once

// Opcional: o Wi-Fi é escolhido na tela de toque. Para placas sem toque, copie este arquivo para
// secrets.h e preencha com a sua rede; ela é usada enquanto nenhuma outra for gravada pela tela.
namespace secrets {

constexpr char kWifiSsid[] = "NOME_DA_REDE";
constexpr char kWifiPassword[] = "SENHA_DA_REDE";

}  // namespace secrets
