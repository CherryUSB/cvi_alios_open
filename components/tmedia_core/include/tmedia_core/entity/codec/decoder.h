/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#ifndef TM_DECODER_H
#define TM_DECODER_H

#include <string>

#include <tmedia_core/common/media_info.h>

#include <tmedia_core/entity/codec/codec.h>
#include <tmedia_core/common/packet.h>
#include <tmedia_core/common/frame.h>
#include <tmedia_core/entity/entity.h>

using namespace std;

class TMVideoDecoder : public TMCodec, public TMFilterEntity
{
public:
    TMVideoDecoder();
    virtual ~TMVideoDecoder();

    // TMCodec interface
    virtual int Open(TMCodecParams &codecParam, TMPropertyList *propList = NULL) = 0;
    virtual int SetConfig(TMPropertyList &propList) = 0;
    virtual int GetConfig(TMPropertyList &propList) = 0;
    virtual int Start() = 0;
    virtual int Flush() = 0;
    virtual int Stop()  = 0;
    virtual int Close() = 0;

    // TMFilterEntity interface
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;
    virtual TMSinkPad *GetSinkPad(int padID = 0) = 0;

    // TMVideoDecoder extend interface
    virtual int SendPacket(TMPacket &pkt, int timeout) = 0;
    virtual int RecvFrame(TMVideoFrame &frame, int timeout) = 0;
};

class TMAudioDecoder : public TMCodec, public TMFilterEntity
{
public:
    TMAudioDecoder();
    virtual ~TMAudioDecoder();

    // TMFilterEntity interface
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;
    virtual TMSinkPad *GetSinkPad(int padID = 0) = 0;

    // TMCodec interface
    virtual int Open(TMCodecParams &codecParam, TMPropertyList *propList = NULL) = 0;
    virtual int SetConfig(TMPropertyList &propList) = 0;
    virtual int GetConfig(TMPropertyList &propList) = 0;
    virtual int Start() = 0;
    virtual int Flush() = 0;
    virtual int Stop()  = 0;
    virtual int Close() = 0;

    // TMAudioDecoder extend interface
    virtual int SendPacket(TMAudioPacket &pkt, int timeout) = 0;
    virtual int RecvFrame(TMAudioFrame &frame, int timeout) = 0;
};

#endif  // TM_DECODER_H
