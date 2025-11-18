#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

#include "raylib.h"
#include <string>
#include <unordered_map>

// Lightweight audio manager for global volume control and playback.
class AudioManager
{
public:
    static void init();
    static void shutdown();
    static void update();

    static void playBGM(const std::string& path, bool loop = true);
    static void stopBGM();
    static void playSFX(const std::string& path);

    static void setMasterVolume(float volume);
    static float getMasterVolume();

private:
    static float sMasterVolume;
    static ::Music sCurrentMusic;
    static bool sMusicLoaded;
    static std::string sCurrentMusicPath;
    static std::unordered_map<std::string, Sound> sSoundCache;
    static bool sAudioReady;
};

#endif
