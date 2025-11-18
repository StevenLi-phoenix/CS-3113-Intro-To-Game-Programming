#include "Music.h"
#include <algorithm>

float AudioManager::sMasterVolume = 1.0f;
::Music AudioManager::sCurrentMusic = {};
bool AudioManager::sMusicLoaded = false;
std::string AudioManager::sCurrentMusicPath = "";
std::unordered_map<std::string, Sound> AudioManager::sSoundCache = {};
bool AudioManager::sAudioReady = false;

static float clampVolume(float volume)
{
    if (volume < 0.0f) return 0.0f;
    if (volume > 1.0f) return 1.0f;
    return volume;
}

void AudioManager::init()
{
    if (!IsAudioDeviceReady())
    {
        InitAudioDevice();
    }
    sAudioReady = IsAudioDeviceReady();
    sMasterVolume = 1.0f;
}

void AudioManager::shutdown()
{
    if (!sAudioReady) return;

    stopBGM();
    for (auto& entry : sSoundCache)
    {
        UnloadSound(entry.second);
    }
    sSoundCache.clear();
    CloseAudioDevice();
    sAudioReady = false;
}

void AudioManager::update()
{
    if (!sAudioReady) return;
    if (sMusicLoaded)
    {
        UpdateMusicStream(sCurrentMusic);
    }
}

void AudioManager::playBGM(const std::string& path, bool loop)
{
    if (!sAudioReady) return;

    if (sMusicLoaded && path == sCurrentMusicPath)
    {
        return;
    }

    stopBGM();

    sCurrentMusic = LoadMusicStream(path.c_str());
    if (sCurrentMusic.stream.buffer == nullptr)
    {
        sMusicLoaded = false;
        return;
    }

    sCurrentMusic.looping = loop;
    SetMusicVolume(sCurrentMusic, sMasterVolume);
    PlayMusicStream(sCurrentMusic);
    sMusicLoaded = true;
    sCurrentMusicPath = path;
}

void AudioManager::stopBGM()
{
    if (!sMusicLoaded) return;
    StopMusicStream(sCurrentMusic);
    UnloadMusicStream(sCurrentMusic);
    sMusicLoaded = false;
    sCurrentMusicPath.clear();
}

void AudioManager::playSFX(const std::string& path)
{
    if (!sAudioReady) return;

    auto it = sSoundCache.find(path);
    if (it == sSoundCache.end())
    {
        Sound sound = LoadSound(path.c_str());
        if (sound.stream.buffer == nullptr)
        {
            return;
        }
        sSoundCache[path] = sound;
        it = sSoundCache.find(path);
    }

    Sound sound = it->second;
    SetSoundVolume(sound, sMasterVolume);
    PlaySound(sound);
}

void AudioManager::setMasterVolume(float volume)
{
    sMasterVolume = clampVolume(volume);
    if (sMusicLoaded)
    {
        SetMusicVolume(sCurrentMusic, sMasterVolume);
    }
}

float AudioManager::getMasterVolume()
{
    return sMasterVolume;
}
