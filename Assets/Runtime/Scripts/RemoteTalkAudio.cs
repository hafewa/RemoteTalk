using System;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif
namespace IST.RemoteTalk
{
    [ExecuteInEditMode]
    [RequireComponent(typeof(AudioSource))]
    [AddComponentMenu("RemoteTalk/Audio")]
    public class RemoteTalkAudio : MonoBehaviour
    {
        public delegate bool SyncBuffers();

        AudioSource m_audioSource;
        AudioClip m_dummyClip;
        rtAudioData m_data;

        bool m_isPlaying;
        bool m_isFinished;
        int m_sampleRate;
        double m_samplePos;
        SyncBuffers m_syncBuffers;

        public bool isPlaying
        {
            get
            {
                Update();
                return m_audioSource != null && m_audioSource.isPlaying;
            }
        }

        public void Play(rtAudioData data, SyncBuffers cb)
        {
            m_data = data;
            m_syncBuffers = cb;
            if (!m_data || m_audioSource == null)
                return;

            if (m_dummyClip == null)
            {
                const int SampleRate = 44000;
                m_dummyClip = AudioClip.Create("Dummy", SampleRate, 1, SampleRate, false);
                var samples = new float[SampleRate];
                for (int i = 0; i < SampleRate; ++i)
                    samples[i] = 1.0f;
                m_dummyClip.SetData(samples, 0);
                m_dummyClip.hideFlags = HideFlags.DontSave;
            }
            m_audioSource.clip = m_dummyClip;
            m_audioSource.loop = true;
            m_audioSource.Play();

            m_sampleRate = AudioSettings.outputSampleRate;
            m_samplePos = 0.0;
            m_isPlaying = true;
        }

        public void Play(AudioClip clip)
        {
            m_audioSource.loop = false;
            m_audioSource.clip = clip;
            m_audioSource.Play();
        }


        public void Stop()
        {
            if (m_audioSource == null)
                return;
            m_isPlaying = false;
            m_isFinished = false;
            m_audioSource.Stop();
            m_audioSource.loop = false;
            m_audioSource.clip = null;
            m_data.Release();
        }


        void OnEnable()
        {
            m_audioSource = GetComponent<AudioSource>();
            m_audioSource.playOnAwake = false;
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying)
                EditorApplication.update += Update;
#endif
        }

        void OnDisable()
        {
#if UNITY_EDITOR
            if (!EditorApplication.isPlaying)
                EditorApplication.update -= Update;
#endif
        }


        void Update()
        {
            if (m_isFinished)
                Stop();
        }

        void OnAudioFilterRead(float[] dst, int channels)
        {
            if (!m_isPlaying || !m_data)
                return;

            bool eos = m_syncBuffers();
            m_samplePos = m_data.Resample(dst, m_sampleRate, channels, dst.Length, m_samplePos);
            if (eos && (m_data.channels == 0 || (int)m_samplePos == m_data.sampleLength / m_data.channels))
            {
                m_isPlaying = false;
                m_isFinished = true;
            }
        }
    }
}
