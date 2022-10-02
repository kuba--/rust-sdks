//
// Created by Théo Monnom on 01/09/2022.
//

#include "livekit/data_channel.h"

#include <utility>

#include "libwebrtc-sys/src/data_channel.rs.h"

namespace livekit {

DataChannel::DataChannel(
    std::shared_ptr<RTCRuntime> rtc_runtime,
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)
    : rtc_runtime_(std::move(rtc_runtime)), data_channel_(std::move(data_channel)) {}

void DataChannel::register_observer(NativeDataChannelObserver& observer) {
  data_channel_->RegisterObserver(&observer);
}

void DataChannel::unregister_observer() {
  data_channel_->UnregisterObserver();
}

bool DataChannel::send(const DataBuffer& buffer) {
  return data_channel_->Send(webrtc::DataBuffer{
      rtc::CopyOnWriteBuffer(buffer.ptr, buffer.len), buffer.binary});
}

rust::String DataChannel::label() const {
  return data_channel_->label();
}

DataState DataChannel::state() const {
  return static_cast<DataState>(data_channel_->state());
}

void DataChannel::close() {
  return data_channel_->Close();
}

std::unique_ptr<NativeDataChannelInit> create_data_channel_init(
    DataChannelInit init) {
  auto rtc_init = std::make_unique<webrtc::DataChannelInit>();
  rtc_init->id = init.id;
  rtc_init->negotiated = init.negotiated;
  rtc_init->ordered = init.ordered;
  rtc_init->protocol = init.protocol.c_str();
  rtc_init->reliable = init.reliable;

  if (init.has_max_retransmit_time)
    rtc_init->maxRetransmitTime = init.max_retransmit_time;

  if (init.has_max_retransmits)
    rtc_init->maxRetransmits = init.max_retransmits;

  if (init.has_priority)
    rtc_init->priority = static_cast<webrtc::Priority>(init.priority);

  return rtc_init;
}

NativeDataChannelObserver::NativeDataChannelObserver(
    rust::Box<DataChannelObserverWrapper> observer)
    : observer_(std::move(observer)) {}

void NativeDataChannelObserver::OnStateChange() {
  observer_->on_state_change();
}

void NativeDataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer) {
  DataBuffer data{};
  data.ptr = buffer.data.data();
  data.len = buffer.data.size();
  data.binary = buffer.binary;
  observer_->on_message(data);
}

void NativeDataChannelObserver::OnBufferedAmountChange(
    uint64_t sent_data_size) {
  observer_->on_buffered_amount_change(sent_data_size);
}

std::unique_ptr<NativeDataChannelObserver> create_native_data_channel_observer(
    rust::Box<DataChannelObserverWrapper> observer) {
  return std::make_unique<NativeDataChannelObserver>(std::move(observer));
}
}  // namespace livekit