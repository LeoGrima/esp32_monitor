#pragma once

#include <Arduino.h>

namespace http {

bool get(const char* url, String& body);
// Baixa um arquivo binário para um buffer já alocado; retorna o tamanho (0 em caso de falha)
size_t download(const char* url, uint8_t* buffer, size_t capacity);

}  // namespace http
