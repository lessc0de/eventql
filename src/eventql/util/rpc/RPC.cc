/**
 * Copyright (c) 2016 DeepCortex GmbH <legal@eventql.io>
 * Authors:
 *   - Paul Asmuth <paul@eventql.io>
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
#include "eventql/util/rpc/RPC.h"

AnyRPC::AnyRPC(
    const std::string& method) :
    method_(method),
    status_(eSuccess),
    is_ready_(false),
    autodelete_(false) {
  incRef();
}

AnyRPC::~AnyRPC() {}

const Buffer& AnyRPC::encoded() {
  if (encoded_request_.size() == 0) {
    RAISE(kRPCError, "RPC request must be encoded first");
  }

  return encoded_request_;
}

const std::string& AnyRPC::method() const {
  return method_;
};

void AnyRPC::wait() {
  ready_wakeup_.waitForWakeup(0);
  status_.raiseIfError();
}

void AnyRPC::onReady(std::function<void()> callback) {
  ready_wakeup_.onWakeup(0, callback);
}

void AnyRPC::ready(const Buffer& result) noexcept {
  try {
    decode_fn_(result);
  } catch (const std::exception& e) {
    error(e);
  }
}

void AnyRPC::ready() noexcept {
  std::unique_lock<std::mutex> lk(mutex_);
  is_ready_ = true;
  lk.unlock();
  ready_wakeup_.wakeup();
  decRef();
}

void AnyRPC::raiseIfError() const {
  status_.raiseIfError();
}

void AnyRPC::error(const std::exception& e) {
  status_ = Status(e);
  ready();
}

void AnyRPC::error(const Status& status) {
  status_ = status;
  ready();
}

bool AnyRPC::isSuccess() const {
  return status_.isSuccess();
}

bool AnyRPC::isFailure() const {
  return status_.isError();
}

const Status& AnyRPC::status() const {
  return status_;
}
