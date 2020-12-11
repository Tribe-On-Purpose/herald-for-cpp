//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "datatype/payload_data.h"
#include "datatype/base64_string.h"

#include <vector>
#include <algorithm>
#include <string>

namespace herald {
namespace datatype {

PayloadData::PayloadData() {
  Data::Data();
}

PayloadData::PayloadData(const Data& from) {
  Data::Data(from);
}

PayloadData::PayloadData(const std::byte* data, std::size_t length) {
  Data::Data(data,length);
}

PayloadData::PayloadData(std::byte repeating, std::size_t count) {
  Data::Data(repeating, count);
}

std::string
PayloadData::shortName() const {
  if (size() == 0) {
    return "";
  }
  if (!(size() > 3)) {
    return Base64String::encode(*this).encoded();
  }
  const Data suffix = subdata(3, size() - 3);
  Base64String base64EncodedString = Base64String::encode(suffix);
  std::string asString = base64EncodedString.encoded();
  return asString.substr((std::size_t)0, std::min((std::size_t)6, asString.length())); // TODO figure out why we trim 6 chars? What if we don't have 6 chars???
}

std::string
PayloadData::toString() const {
  return shortName();
}

} // end namespace
} // end namespace