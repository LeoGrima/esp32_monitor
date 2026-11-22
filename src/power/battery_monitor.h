#pragma once

#include "../model/dashboard_data.h"

// Leitura da bateria pelo ADC do módulo
namespace battery {

void begin();
// Faz uma nova leitura quando o intervalo de amostragem venceu; retorna true se os dados mudaram
bool update(BatteryData& data);

}  // namespace battery
