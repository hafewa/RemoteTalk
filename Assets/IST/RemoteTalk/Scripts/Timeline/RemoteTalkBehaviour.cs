using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    [Serializable]
    public class RemoteTalkBehaviour : PlayableBehaviour
    {
        public PlayableDirector director;
        public RemoteTalkTrack track;
        public TimelineClip clip;

        public Talk talk = new Talk();
        public AudioSource audioSource;
        public AudioClip audioClip;
        public RemoteTalkProvider remoteTalk;

        bool m_export;


        public AudioSource GetOutput()
        {
            AudioSource ret = null;
            foreach(var output in track.outputs)
            {
                ret = director.GetGenericBinding(output.sourceObject) as AudioSource;
                if (ret != null)
                    break;
            }
            return ret;
        }

        public override void OnBehaviourPlay(Playable playable, FrameData info)
        {
            var output = GetOutput();
            if (output != null)
            {
                if (audioClip != null)
                    output.PlayOneShot(audioClip);
                else
                {
                    var provider = talk.provider;
                    if (provider != null)
                    {
                        provider.output = output;
                        if (provider.Play(talk))
                        {
                            m_export = true;
                            track.OnTalk(this);
                        }
                    }
                }
            }
        }

        public override void OnBehaviourPause(Playable playable, FrameData info)
        {
            var output = GetOutput();
            if (output != null)
            {
                if (audioClip != null)
                    output.Stop();
                //else
                //    talk.Stop();
            }
        }

        public void OnAudioClipImport(Talk t, AudioClip ac)
        {
            if (!m_export)
                return;
            m_export = false;

            var rtc = (RemoteTalkClip)clip.asset;
            if (rtc.UpdateCachedClip())
            {
                audioClip = rtc.audioClip.defaultValue as AudioClip;
                track.OnAudioClipUpdated(this);
            }
        }

        public override void OnPlayableCreate(Playable playable)
        {
            RemoteTalkProvider.onAudioClipImport += OnAudioClipImport;
        }

        public override void OnPlayableDestroy(Playable playable)
        {
            RemoteTalkProvider.onAudioClipImport -= OnAudioClipImport;
        }
    }
}
