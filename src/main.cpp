#include "main.hpp"
#include "config.hpp"
#include "slices.hpp"

// BSML replaces QuestUI in BS 1.37.0
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML/Components/Settings/ToggleSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/IncrementSetting.hpp"

using namespace GlobalNamespace;

static ModInfo modInfo;

Logger& getLogger() {
    static auto logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

bool successfulInit = true;

#include "GlobalNamespace/AudioTimeSyncController.hpp"

MAKE_HOOK_MATCH(AudioTimeSyncController_Start, &AudioTimeSyncController::Start, void, AudioTimeSyncController* self) {

    AudioTimeSyncController_Start(self);

    if(getModConfig().Enabled.GetValue() && successfulInit)
        successfulInit = Init();
}

MAKE_HOOK_MATCH(AudioTimeSyncController_StartSong, &AudioTimeSyncController::StartSong, void, AudioTimeSyncController* self, float startTimeOffset) {

    if(getModConfig().Enabled.GetValue() && successfulInit)
        successfulInit = MakeSprites();

    AudioTimeSyncController_StartSong(self, startTimeOffset);
}

MAKE_HOOK_MATCH(AudioTimeSyncController_Update, &AudioTimeSyncController::Update, void, AudioTimeSyncController* self) {

    AudioTimeSyncController_Update(self);

    if(getModConfig().Enabled.GetValue() && successfulInit)
        Update();
}

#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"

MAKE_HOOK_MATCH(NoteController_SendNoteWasCutEvent, &NoteController::SendNoteWasCutEvent, void, NoteController* self, ByRef<NoteCutInfo> noteCutInfo) {

    if(getModConfig().Enabled.GetValue() && successfulInit && noteCutInfo->get_allIsOK() && self->noteData->gameplayType != NoteData::GameplayType::BurstSliderElement)
        CreateSlice(noteCutInfo.heldRef);

    NoteController_SendNoteWasCutEvent(self, noteCutInfo);
}

// In BS 1.37.0, GameplayCoreSceneSetupData constructor signature changed.
// ColorScheme is now accessed differently, and several legacy parameters were removed/rearranged.
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "GlobalNamespace/BeatmapDataCache.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"

MAKE_HOOK_MATCH(GameplayCoreSceneSetupData_ctor, &GameplayCoreSceneSetupData::_ctor, void,
    GameplayCoreSceneSetupData* self,
    GlobalNamespace::BeatmapKey beatmapKey,
    GlobalNamespace::BeatmapLevel* beatmapLevel,
    GlobalNamespace::GameplayModifiers* gameplayModifiers,
    GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
    GlobalNamespace::PracticeSettings* practiceSettings,
    GlobalNamespace::EnvironmentInfoSO* environmentInfo,
    GlobalNamespace::ColorScheme* colorScheme,
    GlobalNamespace::BeatmapDataCache* beatmapDataCache) {

    SetColors(colorScheme->saberAColor, colorScheme->saberBColor);
    successfulInit = true;

    GameplayCoreSceneSetupData_ctor(self, beatmapKey, beatmapLevel, gameplayModifiers, playerSpecificSettings, practiceSettings, environmentInfo, colorScheme, beatmapDataCache);
}

// BSML settings menu
void SettingsDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if(firstActivation) {
        BSML::parse_and_inject_async(self->get_gameObject(), R"(
<vertical child-control-height='false' child-force-expand-height='false' spacing='1'>
  <toggle-setting text='Mod Enabled' value='enabled' bind-value='true' apply-on-change='true'/>
  <toggle-setting text='Dynamic Fade Speed' value='dynamic' bind-value='true' apply-on-change='true' hover-hint='When enabled, slice visuals will fade faster during faster parts of songs'/>
  <increment-setting text='Fade Speed' value='fadeSpeed' bind-value='true' apply-on-change='true' min='0.1' max='2' increment='0.1' digits='1' hover-hint='The speed that slice visuals fade at, a higher value means it fades quicker'/>
</vertical>
        )", self);
    }
}

extern "C" void setup(CModInfo* info) {
    info->id = MOD_ID;
    info->version = VERSION;
    modInfo.assign(*info);

    getModConfig().Init(modInfo);
}

extern "C" void late_load() {
    // Register settings with BSML
    BSML::Register::RegisterSettingsMenu(modInfo, SettingsDidActivate, false);

    LOG_INFO("Installing hooks...");
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_Start);
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_StartSong);
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_Update);
    INSTALL_HOOK(getLogger(), NoteController_SendNoteWasCutEvent);
    INSTALL_HOOK(getLogger(), GameplayCoreSceneSetupData_ctor);
    LOG_INFO("Installed all hooks!");
}
