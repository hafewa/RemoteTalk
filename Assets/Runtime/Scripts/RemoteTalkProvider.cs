using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    [Serializable]
    public class TalkParam
    {
        public string name;
        public float value, rangeMin, rangeMax;


        public void Validate()
        {
            if (rangeMax > rangeMin)
                value = Mathf.Clamp(value, rangeMin, rangeMax);
        }

        public TalkParam Clone()
        {
            return new TalkParam {
                name = name,
                value = value,
                rangeMin = rangeMin,
                rangeMax = rangeMax
            };
        }

        public static TalkParam[] Clone(TalkParam[] src)
        {
            return src.Select(a => a.Clone()).ToArray();
        }
        public static List<TalkParam> Clone(List<TalkParam> src)
        {
            return src.Select(a => a.Clone()).ToList();
        }

        public static int Merge(TalkParam[] dst, IEnumerable<TalkParam> src)
        {
            if (dst == null || src == null)
                return 0;
            int ret = 0;
            foreach (var sp in src)
            {
                var i = Array.FindIndex(dst, a => a.name == sp.name);
                if (i != -1)
                {
                    var dp = dst[i];
                    dp.value = sp.value;
                    dp.Validate();
                    ++ret;
                }
            }
            return ret;
        }

        public static int Merge(List<TalkParam> dst, IEnumerable<TalkParam> src)
        {
            if (dst == null || src == null)
                return 0;
            int ret = 0;
            foreach (var sp in src)
            {
                var i = dst.FindIndex(a => a.name == sp.name);
                if (i != -1)
                {
                    var dp = dst[i];
                    dp.value = sp.value;
                    dp.Validate();
                    ++ret;
                }
                else
                {
                    dst.Add(sp);
                }
            }
            return ret;
        }

        public static bool Compare(TalkParam[] dst, IEnumerable<TalkParam> src)
        {
            if (dst == null || src == null)
                return false;
            foreach (var sp in src)
            {
                var i = Array.FindIndex(dst, a => a.name == sp.name);
                if (i != -1 && dst[i].value != sp.value)
                    return false;
            }
            return true;
        }

#if UNITY_EDITOR
        public static void Copy(SerializedProperty dst, TalkParam src)
        {
            dst.FindPropertyRelative("name").stringValue = src.name;
            dst.FindPropertyRelative("value").floatValue = src.value;
            dst.FindPropertyRelative("rangeMin").floatValue = src.rangeMin;
            dst.FindPropertyRelative("rangeMax").floatValue = src.rangeMax;
        }
#endif
    }

    [Serializable]
    public class Cast
    {
        public string hostName;
        public int id;
        public string name;
        public TalkParam[] paramInfo;

        public RemoteTalkProvider provider
        {
            get { return RemoteTalkProvider.FindByHostName(hostName); }
        }
    }

    [Serializable]
    public class Talk : IEquatable<Talk>
    {
        public string castName = "";
        public TalkParam[] param = new TalkParam[0];
        [Range(0, 10)] public float wait = 0.0f;
        public string text = "";
#if UNITY_EDITOR
        public bool foldParams;
#endif

        public Cast cast
        {
            get { return RemoteTalkProvider.FindCast(castName); }
        }
        public RemoteTalkProvider provider
        {
            get { return RemoteTalkProvider.FindByCastName(castName); }
        }
        public AudioClip audioClip
        {
            get
            {
                var prov = provider;
                if (prov != null)
                    return prov.FindClip(this);
                else
                {
                    foreach (var p in RemoteTalkProvider.instances)
                    {
                        var clip = p.FindClip(this);
                        if (clip != null)
                            return clip;
                    }
                }
                return null;
            }
        }

        public bool ValidateParams()
        {
            var c = cast;
            if (c != null)
            {
                var prev = param;
                param = TalkParam.Clone(c.paramInfo);
                TalkParam.Merge(param, prev);
                return true;
            }
            return false;
        }

        public TalkParam FindParam(string key)
        {
            var i = Array.FindIndex(param, a => a.name == key);
            return i != -1 ? param[i] : null;
        }

        public Talk Clone()
        {
            return new Talk {
                castName = (string)castName.Clone(),
                param = TalkParam.Clone(param),
                text = (string)text.Clone(),
            };
        }

        public override int GetHashCode()
        {
            var tparam = rtTalkParams.defaultValue;
            tparam.Assign(param);
            return castName.GetHashCode() ^ text.GetHashCode() ^ (int)tparam.hash;
        }

        // compare cast name, text and param values (param names, ranges and other fields are ignored)
        public bool Equals(Talk v)
        {
            return v.GetHashCode() == GetHashCode();
        }
    }

    public abstract class RemoteTalkProvider : MonoBehaviour
    {
        static List<RemoteTalkProvider> s_instances = new List<RemoteTalkProvider>();

        public static List<RemoteTalkProvider> instances { get { return s_instances; } }


        public static event Action<Talk> onTalkStart;
        public static event Action<Talk, bool> onTalkFinish;
#if UNITY_EDITOR
        public static event Action<Talk, AudioClip> onAudioClipImport;
#endif

        protected static void FireOnTalkStart(Talk t)
        {
            if (onTalkStart != null)
                onTalkStart(t);
        }
        protected static void FireOnTalkFinish(Talk t, bool succeeded)
        {
            if (onTalkFinish != null)
                onTalkFinish(t, succeeded);
        }
#if UNITY_EDITOR
        protected static void FireOnAudioClipImport(Talk t, AudioClip ac)
        {
            if (onAudioClipImport != null)
                onAudioClipImport(t, ac);
        }
#endif

        public static IEnumerable<Cast> allCasts
        {
            get
            {
                var providers = instances;
                if (providers.Count == 0)
                    return new Cast[0] { };
                IEnumerable<Cast> ret = providers[0].casts;
                for (int i = 1; i < providers.Count; ++i)
                    ret = ret.Concat(providers[i].casts);
                return ret;
            }
        }

        public static Cast FindCast(string castName)
        {
            return allCasts.FirstOrDefault(a => a.name == castName);
        }

        public static RemoteTalkProvider FindByHostName(string hostName)
        {
            return instances.FirstOrDefault(p => p.hostName == hostName);
        }

        public static RemoteTalkProvider FindByCastName(string castName)
        {
            return instances.FirstOrDefault(p =>
            {
                return p.casts.FirstOrDefault(c => c.name == castName) != null;
            });
        }


        public abstract string hostName { get; }
        public abstract Cast[] casts { get; }
        public abstract bool isPlaying { get; }
        public abstract bool isReady { get; }
        public abstract AudioSource output { get; set; }
        public abstract bool exportAudio { get; }

        public abstract bool Play(Talk talk);
        public abstract void Stop();

        public abstract AudioClip FindClip(Talk talk);


        protected virtual void OnEnable()
        {
            instances.Add(this);
        }

        protected virtual void OnDisable()
        {
            instances.Remove(this);
        }
    }
}
