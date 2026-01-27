//
// Implementation of XML Utility Functions
//

#include "gimt/gimt_xml_utils.h"

namespace gimt {

std::string xmlEscape(const std::string &str) {
  std::string result;
  result.reserve(str.size());

  for (char ch : str) {
    switch (ch) {
    case '&':
      result += "&amp;";
      break;
    case '<':
      result += "&lt;";
      break;
    case '>':
      result += "&gt;";
      break;
    case '"':
      result += "&quot;";
      break;
    case '\'':
      result += "&apos;";
      break;
    default:
      result += ch;
      break;
    }
  }

  return result;
}

std::string xmlUnescape(const std::string &str) {
  // IMPORTANT: &amp; must be replaced LAST to avoid double-unescaping
  static const std::pair<std::string, std::string> entities[] = {
      {"&quot;", "\""},
      {"&lt;", "<"},
      {"&gt;", ">"},
      {"&apos;", "'"},
      {"&amp;", "&"}};

  std::string result = str;
  size_t start_pos = 0;
  for (const auto &entity : entities) {
    start_pos = 0;
    while ((start_pos = result.find(entity.first, start_pos)) != std::string::npos) {
      result.replace(start_pos, entity.first.length(), entity.second);
      start_pos += entity.second.length();
    }
  }
  return result;
}

std::string buildXmpPayload(const std::string &escapedJson) {
  std::string payload;
  payload.reserve(256);
  payload += "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\">";
  payload += "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" "
             "xmlns:TC260=\"http://ns.adobe.com/xap/1.0/tc/\">";
  payload += "<rdf:Description rdf:about=\"\" TC260:AIGC=\"";
  payload += escapedJson;
  payload += "\"/>";
  payload += "</rdf:RDF>";
  payload += "</x:xmpmeta>";
  return payload;
}

bool extractAigcJsonFromXmp(const std::string &xmpStr, std::string &outJson) {
  // Try attribute format first: TC260:AIGC="..."
  const std::string attrKey = "TC260:AIGC=\"";
  size_t startPos = xmpStr.find(attrKey);
  if (startPos != std::string::npos) {
    startPos += attrKey.length();
    size_t endPos = xmpStr.find("\"", startPos);
    if (endPos != std::string::npos && endPos > startPos) {
      std::string escapedJson = xmpStr.substr(startPos, endPos - startPos);
      outJson = xmlUnescape(escapedJson);
      return true;
    }
  }

  // Try tag format: <TC260:AIGC>...</TC260:AIGC>
  const std::string tagStart = "<TC260:AIGC>";
  const std::string tagEnd = "</TC260:AIGC>";
  startPos = xmpStr.find(tagStart);
  if (startPos != std::string::npos) {
    startPos += tagStart.length();
    size_t endPos = xmpStr.find(tagEnd, startPos);
    if (endPos != std::string::npos && endPos > startPos) {
      std::string escapedJson = xmpStr.substr(startPos, endPos - startPos);
      outJson = xmlUnescape(escapedJson);
      return true;
    }
  }

  return false;
}

} // namespace gimt
