#pragma once

#include <Arduino.h>

namespace http {

bool get(const char* url, String& body);
// Mede o tempo até a resposta (inclui a conexão TLS); false se o endereço não respondeu com sucesso
bool probe(const char* url, uint32_t& elapsedMs);
// Baixa um arquivo binário para um buffer já alocado; retorna o tamanho (0 em caso de falha)
size_t download(const char* url, uint8_t* buffer, size_t capacity);

}  // namespace http
