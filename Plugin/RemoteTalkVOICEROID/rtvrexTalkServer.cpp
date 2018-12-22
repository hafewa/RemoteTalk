#include "pch.h"
#include "rtvrexCommon.h"
#include "rtvrexHookHandler.h"
#include "rtvrexTalkServer.h"
#include "rtvrexTalkInterface.h"

namespace rtvrex {

static void RequestUpdate()
{
    ::PostMessageW((HWND)0xffff, WM_TIMER, 0, 0);
}

void TalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);

    // force call GetMessageW()
    RequestUpdate();
}

bool TalkServer::isReady()
{
    return false;
}

TalkServer::Status TalkServer::onStats(StatsMessage& mes)
{
    auto& ifs = TalkInterface::getInstance();
    ifs.setupControls();

    auto& stats = mes.stats;
    stats.host = ifs.getClientName();
    stats.plugin_version = ifs.getPluginVersion();
    stats.protocol_version = ifs.getProtocolVersion();

    ifs.getParams(mes.stats.params);
    mes.stats.casts.push_back(*ifs.getCastInfo(0));

    return Status::Succeeded;
}

TalkServer::Status TalkServer::onTalk(TalkMessage& mes)
{
    auto& ifs = TalkInterface::getInstance();
    if(ifs.isPlaying())
        return Status::Failed;

    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.clear();
    }
    ifs.setAudioCallback([this](const rt::AudioData& data) {
        auto tmp = std::make_shared<rt::AudioData>(data);
        {
            std::unique_lock<std::mutex> lock(m_data_mutex);
            m_data_queue.push_back(tmp);
        }
    });

    ifs.setupControls();

    DSoundHandler::getInstance().mute = mes.params.mute;

    ifs.setParams(mes.params);
    if (!ifs.setText(mes.text.c_str()))
        return Status::Failed;
    if (!ifs.play())
        return Status::Failed;

    mes.task = std::async(std::launch::async, [this, &mes]() {
        std::vector<rt::AudioDataPtr> tmp;
        for (;;) {
            {
                std::unique_lock<std::mutex> lock(m_data_mutex);
                tmp = m_data_queue;
                m_data_queue.clear();
            }

            for (auto& ad : tmp) {
                ad->serialize(*mes.respond_stream);
            }

            if (!tmp.empty() && tmp.back()->data.empty())
                break;
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    return Status::Succeeded;
}

TalkServer::Status TalkServer::onStop(StopMessage& mes)
{
    auto& ifs = TalkInterface::getInstance();
    if (!ifs.isPlaying())
        return Status::Failed;

    ifs.setupControls();
    ifs.stop();
    return Status::Succeeded;
}

#ifdef rtDebug
TalkServer::Status TalkServer::onDebug(DebugMessage& mes)
{
    auto& ifs = TalkInterface::getInstance();
    ifs.setupControls();
    return Status::Succeeded;
}
#endif

} // namespace rtvrex
