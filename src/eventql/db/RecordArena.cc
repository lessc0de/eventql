/**
 * Copyright (c) 2016 zScale Technology GmbH <legal@zscale.io>
 * Authors:
 *   - Paul Asmuth <paul@zscale.io>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License ("the license") as
 * published by the Free Software Foundation, either version 3 of the License,
 * or any later version.
 *
 * In accordance with Section 7(e) of the license, the licensing of the Program
 * under the license does not imply a trademark license. Therefore any rights,
 * title and interest in our trademarks remain entirely with us.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You can be released from the requirements of the license by purchasing a
 * commercial license. Buying such a license is mandatory as soon as you develop
 * commercial activities involving this program without disclosing the source
 * code of your own applications
 */
#include <eventql/db/RecordArena.h>

#include "eventql/eventql.h"

namespace eventql {

RecordArena::RecordArena() {}

bool RecordArena::insertRecord(const RecordRef& record) {
  ScopedLock<std::mutex> lk(mutex_);

  auto old = records_.find(record.record_id);
  if (old == records_.end()) {
    records_.emplace(record.record_id, record);
    return true;
  } else if(old->second.record_version < record.record_version) {
    old->second = record;
    return true;
  } else {
    return false;
  }
}

void RecordArena::fetchRecords(Function<void (const RecordRef& record)> fn) {
  ScopedLock<std::mutex> lk(mutex_); // FIXME

  for (const auto& r : records_) {
    fn(r.second);
  }
}

uint64_t RecordArena::fetchRecordVersion(const SHA1Hash& record_id) {
  ScopedLock<std::mutex> lk(mutex_);
  auto rec = records_.find(record_id);
  if (rec == records_.end()) {
    return 0;
  } else {
    return rec->second.record_version;
  }
}

size_t RecordArena::size() const {
  ScopedLock<std::mutex> lk(mutex_);
  return records_.size();
}

} // namespace eventql
