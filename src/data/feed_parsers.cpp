#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_parsers.h"

namespace parsers {
namespace {

struct Entity {
  const char* name;
  const char* text;
};
constexpr Entity kEntities[] = {{"amp;", "&"}, {"lt;", "<"}, {"gt;", ">"}, {"quot;", "\""}, {"apos;", "'"}};

size_t appendUtf8(char* out, size_t capacity, unsigned long code) {
  char encoded[4];
  size_t length = 0;
  if (code < 0x80) {
    encoded[length++] = static_cast<char>(code);
  } else if (code < 0x800) {
    encoded[length++] = static_cast<char>(0xC0 | (code >> 6));
    encoded[length++] = static_cast<char>(0x80 | (code & 0x3F));
  } else if (code < 0x10000) {
    encoded[length++] = static_cast<char>(0xE0 | (code >> 12));
    encoded[length++] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
    encoded[length++] = static_cast<char>(0x80 | (code & 0x3F));
  } else {
    return 0;  // fora do plano básico (emojis): sem glifo na fonte
  }
  if (length > capacity) return 0;
  memcpy(out, encoded, length);
  return length;
}

// Copia [begin, end) decodificando entidades XML; trunca sem cortar um caractere UTF-8
void copyDecoded(const char* begin, const char* end, char* out, size_t outSize) {
  size_t used = 0;
  while (begin < end && used + 4 < outSize) {
    if (*begin != '&') {
      const size_t charLength = (*begin & 0xE0) == 0xC0 ? 2 : (*begin & 0xF0) == 0xE0 ? 3 : (*begin & 0xF8) == 0xF0 ? 4 : 1;
      if (charLength == 4) {
        begin += 4;
        continue;
      }
      for (size_t i = 0; i < charLength && begin < end; ++i) out[used++] = *begin++;
      continue;
    }
    ++begin;
    if (*begin == '#') {
      char* after = nullptr;
      const bool hex = begin[1] == 'x' || begin[1] == 'X';
      const unsigned long code = strtoul(begin + (hex ? 2 : 1), &after, hex ? 16 : 10);
      used += appendUtf8(out + used, outSize - 1 - used, code);
      begin = (*after == ';') ? after + 1 : after;
      continue;
    }
    bool matched = false;
    for (const Entity& entity : kEntities) {
      const size_t length = strlen(entity.name);
      if (strncmp(begin, entity.name, length) == 0) {
        out[used++] = entity.text[0];
        begin += length;
        matched = true;
        break;
      }
    }
    if (!matched) out[used++] = '&';
  }
  out[used] = '\0';
}

// Extrai o conteúdo de <tag>...</tag> dentro de [from, limit); retorna a posição após a tag
const char* extractTag(const char* from, const char* limit, const char* tag, char* out, size_t outSize) {
  char open[24];
  char close[24];
  snprintf(open, sizeof(open), "<%s", tag);
  snprintf(close, sizeof(close), "</%s>", tag);
  out[0] = '\0';

  const char* start = from;
  const size_t openLength = strlen(open);
  for (;;) {
    start = strstr(start, open);
    if (start == nullptr || (limit != nullptr && start >= limit)) return nullptr;
    if (start[openLength] == '>' || start[openLength] == ' ') break;  // ignora tags com o mesmo prefixo
    start += openLength;
  }
  const char* content = strchr(start, '>');
  if (content == nullptr) return nullptr;
  ++content;
  const char* end = strstr(content, close);
  if (end == nullptr) return nullptr;

  constexpr char kCdataOpen[] = "<![CDATA[";
  const char* contentEnd = end;
  if (strncmp(content, kCdataOpen, sizeof(kCdataOpen) - 1) == 0) {
    content += sizeof(kCdataOpen) - 1;
    contentEnd -= 3;  // "]]>"
  }
  if (contentEnd > content) copyDecoded(content, contentEnd, out, outSize);
  return end + strlen(close);
}

}  // namespace

uint8_t parseNewsFeed(const char* xml, char titles[][kFeedTitleSize], uint8_t maxTitles) {
  uint8_t count = 0;
  const char* cursor = xml;
  while (count < maxTitles && (cursor = strstr(cursor, "<item")) != nullptr) {
    const char* itemEnd = strstr(cursor, "</item>");
    if (itemEnd == nullptr) break;
    if (extractTag(cursor, itemEnd, "title", titles[count], kFeedTitleSize) != nullptr && titles[count][0] != '\0') {
      ++count;
    }
    cursor = itemEnd;
  }
  return count;
}

uint8_t parseVideoFeed(const char* xml, VideoEntry* entries, uint8_t maxEntries, char* channel, size_t channelSize) {
  const char* cursor = strstr(xml, "<entry");
  if (cursor == nullptr) return 0;
  extractTag(xml, cursor, "title", channel, channelSize);

  uint8_t count = 0;
  while (count < maxEntries && cursor != nullptr) {
    const char* entryEnd = strstr(cursor, "</entry>");
    if (entryEnd == nullptr) break;
    VideoEntry& entry = entries[count];
    char published[40];
    if (extractTag(cursor, entryEnd, "yt:videoId", entry.id, sizeof(entry.id)) != nullptr &&
        extractTag(cursor, entryEnd, "title", entry.title, sizeof(entry.title)) != nullptr &&
        extractTag(cursor, entryEnd, "published", published, sizeof(published)) != nullptr) {
      entry.publishedAt = parseIsoDate(published);
      ++count;
    }
    cursor = strstr(entryEnd, "<entry");
  }
  return count;
}

}  // namespace parsers
