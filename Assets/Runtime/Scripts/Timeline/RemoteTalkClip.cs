#if UNITY_2017_1_OR_NEWER
using System;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    [Serializable]
    public class RemoteTalkClip : PlayableAsset, ITimelineClipAsset
    {
        // note: non-public template means unrecordable
        RemoteTalkBehaviour template = new RemoteTalkBehaviour();

        public Talk talk = new Talk();
        public ExposedReference<AudioClip> audioClip;


        public ClipCaps clipCaps
        {
            get { return ClipCaps.None; }
        }

        public AudioClip GetAudioClip()
        {
            return audioClip.defaultValue as AudioClip;
        }

        public string GetDisplayName()
        {
            return talk.text + "_" + talk.castName;
        }

        public override Playable CreatePlayable(PlayableGraph graph, GameObject owner)
        {
            talk.ValidateParams();
            var ret = ScriptPlayable<RemoteTalkBehaviour>.Create(graph, template);
            var behaviour = ret.GetBehaviour();
            behaviour.talk = talk;
            behaviour.clipHash = GetHashCode();
            return ret;
        }


        public bool UpdateCachedClip(bool dryRun = false)
        {
            var provider = talk.provider;
            if (provider != null)
            {
                var ac = provider.FindClip(talk);
                if (audioClip.defaultValue != ac)
                {
                    if (!dryRun)
                        audioClip.defaultValue = ac;
                    return true;
                }
            }
            return false;
        }

        public override double duration
        {
            get
            {
                var ac = audioClip.defaultValue as AudioClip;
                if (ac == null)
                    return base.duration;
                return (double)ac.samples / (double)ac.frequency;
            }
        }
    }
}
#endif
