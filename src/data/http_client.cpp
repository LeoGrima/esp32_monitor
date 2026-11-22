#include "http_client.h"

#include <HTTPClient.h>
#include <WiFiClientSecure.h>

namespace http {
namespace {

constexpr uint32_t kTimeoutMs = 10000;
constexpr char kUserAgent[] = "Mozilla/5.0 (ESP32-S3) desk-dashboard";

bool begin(HTTPClient& client, WiFiClientSecure& transport, const char* url) {
  transport.setInsecure();  // sem validação de certificado: dados públicos, somente leitura
  client.setTimeout(kTimeoutMs);
  client.setConnectTimeout(kTimeoutMs);
  client.setUserAgent(kUserAgent);
  client.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!client.begin(transport, url)) return false;

  const int status = client.GET();
  if (status != HTTP_CODE_OK) {
    log_w("HTTP %d em %s", status, url);
    client.end();
    return false;
  }
  return true;
}

}  // namespace

bool get(const char* url, String& body) {
  WiFiClientSecure transport;
  HTTPClient client;
  if (!begin(client, transport, url)) return false;
  body = client.getString();
  client.end();
  return body.length() > 0;
}

size_t download(const char* url, uint8_t* buffer, size_t capacity) {
  WiFiClientSecure transport;
  HTTPClient client;
  if (!begin(client, transport, url)) return 0;

  const int size = client.getSize();
  size_t received = 0;
  if (size > 0 && static_cast<size_t>(size) <= capacity) {
    transport.setTimeout(kTimeoutMs);
    received = transport.readBytes(buffer, size);
  }
  client.end();
  return received == static_cast<size_t>(size) ? received : 0;
}

}  // namespace http
