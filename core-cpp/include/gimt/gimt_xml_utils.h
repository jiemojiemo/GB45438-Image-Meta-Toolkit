//
// XML Utility Functions for GIMT
//

#ifndef GIMT_XML_UTILS_H
#define GIMT_XML_UTILS_H

#include <string>

namespace gimt {

// XML entity escape - converts special characters to XML entities
std::string xmlEscape(const std::string &str);

// XML entity unescape - converts XML entities back to special characters
std::string xmlUnescape(const std::string &str);

// Build XMP payload with AIGC JSON data embedded
std::string buildXmpPayload(const std::string &escapedJson);

// Extract AIGC JSON from XMP string
bool extractAigcJsonFromXmp(const std::string &xmpStr, std::string &outJson);

} // namespace gimt

#endif // GIMT_XML_UTILS_H
