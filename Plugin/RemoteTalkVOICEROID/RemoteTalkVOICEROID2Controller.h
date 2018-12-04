#pragma once
#include <string>
#include "RemoteTalk/rtTalkInterface.h"

class rtvr2TalkInterface : public rt::TalkInterface
{
public:
    virtual void onPlay() = 0;
    virtual void onStop() = 0;
    virtual void onUpdateBuffer(const rt::AudioData& ad) = 0;
};
