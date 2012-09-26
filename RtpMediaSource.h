/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Additions and refactorings by E.S.R.Labs GmbH
 */

#ifndef RTPMEDIASOURCE_H_
#define RTPMEDIASOURCE_H_

#include "mindroid/os/Handler.h"
#include "mindroid/os/Thread.h"
#include "mindroid/util/List.h"

class MediaAssembler;
namespace mindroid {
class Message;
class Buffer;
class DatagramSocket;
class Socket;
}

using mindroid::sp;

class RtpMediaSource :
	public mindroid::Handler
{
public:
	RtpMediaSource(const sp<mindroid::Message>& notifyVideoBuffer, uint16_t localPort); // UDP
	RtpMediaSource(const sp<mindroid::Message>& notifyVideoBuffer, mindroid::String serverHostName, uint16_t serverPort); // TCP
	virtual ~RtpMediaSource();

	bool start(sp<MediaAssembler> mediaAssembler);
	void stop();

	virtual void handleMessage(const sp<mindroid::Message>& message);

	sp< mindroid::List< sp<mindroid::Buffer> > > getMediaQueue() { return mQueue; }

private:
	class NetReceiver :
			public mindroid::Thread
	{
	public:
		NetReceiver(const sp<mindroid::Message>& notifyRtpPacket, const sp<mindroid::Message>& notifyRtcpPacket, const sp<mindroid::Message>& notifyVideoBuffer);
		virtual void run() = 0;
		virtual void stop() = 0;

	protected:
		sp<mindroid::Message> mNotifyRtpPacket;
		sp<mindroid::Message> mNotifyRtcpPacket;
		sp<mindroid::Message> mNotifyVideoBuffer;
		int mPipe[2];

		NO_COPY_CTOR_AND_ASSIGNMENT_OPERATOR(NetReceiver)
	};

	class UdpNetReceiver :
			public NetReceiver
	{
	public:
		UdpNetReceiver(uint16_t port, const sp<mindroid::Message>& notifyRtpPacket, const sp<mindroid::Message>& notifyRtcpPacket, const sp<mindroid::Message>& notifyVideoBuffer);
		virtual void run();
		virtual void stop();

	private:
		static const uint32_t MAX_UDP_PACKET_SIZE = 65536;

		sp<mindroid::DatagramSocket> mRtpSocket;
		sp<mindroid::DatagramSocket> mRtcpSocket;

		NO_COPY_CTOR_AND_ASSIGNMENT_OPERATOR(UdpNetReceiver)
	};

	class TcpNetReceiver :
			public NetReceiver
	{
	public:
		TcpNetReceiver(mindroid::String hostName, uint16_t port, const sp<mindroid::Message>& notifyRtpPacket, const sp<mindroid::Message>& notifyRtcpPacket, const sp<mindroid::Message>& notifyVideoBuffer);
		virtual void run();
		virtual void stop();

	private:
		static const uint32_t MAX_TCP_PACKET_SIZE = 65536;

		sp<mindroid::Socket> mRtpSocket;
		sp<mindroid::Socket> mRtcpSocket;
		mindroid::String mHostName;
		uint16_t mPort;
		bool mFirstTime;

		NO_COPY_CTOR_AND_ASSIGNMENT_OPERATOR(TcpNetReceiver)
	};

	static const uint32_t NOTIFY_RTP_PACKET = 0;
	static const uint32_t NOTIFY_RTCP_PACKET = 1;
	static const uint32_t RTP_HEADER_SIZE = 12;
	static const uint8_t EXT_HEADER_BIT = 1 << 4;
	static const uint8_t PADDING_BIT = 1 << 5;
	static const uint8_t MARK_BIT = 1 << 7;

	int parseRtpHeader(const sp<mindroid::Buffer>& buffer);
	void processRtpPayload(const sp<mindroid::Buffer>& buffer);

	sp<NetReceiver> mNetReceiver;
	sp< mindroid::List< sp<mindroid::Buffer> > > mQueue;
	sp<MediaAssembler> mMediaAssembler;
	uint32_t mRtpPacketCounter;
	uint32_t mHighestSeqNumber;

	NO_COPY_CTOR_AND_ASSIGNMENT_OPERATOR(RtpMediaSource)
};

#endif /* RTPMEDIASOURCE_H_ */
