//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "herald/datatype/immediate_send_data.h"

namespace herald {
namespace datatype {

ImmediateSendData::ImmediateSendData() : Data() { } // forces Data copy ctor
ImmediateSendData::ImmediateSendData(const Data& data) : Data(data) { } // forces Data copy ctor

} // end namespace
} // end namespace