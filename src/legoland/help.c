#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "legoland.h"
#include "obj_instance.h"

#include "challenge.h"
#include "help.h"
#include "icon.h"
#include "interface.h"
#include "nerps.h"
#include "objectives.h"
#include "popupinfo.h"
#include "text.h"
#include "timer.h"
#include "tooltip.h"

#include "image_sprite.h"
#include "stream.h"

// FUNCTION: LEGOLAND 0x0046ce20
void ClearAdvisorHelp(void) {
    if ((DAT_007fe040 & 0x3) != 0) {
        free((void *)AdvisorHelpText);
        UpdateScriptEndIcon((void *)1);
    }
    DAT_007fe040 &= ~0x3;
    FUN_00476000();
    SetNextAdvisorAnimState(0, 0);
    SpeechStop();
}

// FUNCTION: LEGOLAND 0x0046ce60
LEGO_EXPORT int DisplayAdvisorHelp(char *param_1, unsigned int param_2, unsigned int param_3) {
    if ((DAT_007fe040 & 1) != 0) {
        return 0;
    }
    DAT_007fe040 |= 1;
    AdvisorHelpText = (char *)malloc(strlen(param_1) + 1);
    strcpy(AdvisorHelpText, param_1);
    AdvisorHelpStartTime = GetGameTimer();
    DAT_007fe044 = param_2;
    UpdateScriptEndIcon((void *)0);
    return 1;
}

// FUNCTION: LEGOLAND 0x0046cee0
unsigned int FUN_0046cee0(void) {
    unsigned int result = GetGameTimer() - DAT_007fe050;
    if ((int)result >= 0) {
        DAT_00668618 = 0;
        return 1;
    }
    if (DAT_00668724 != NULL) {
        if (((struct ObjectiveEvent *)DAT_00668724)->type == 0) {
            if (DAT_00668618 == 0) {
                return 0;
            }
        }
    } else {
        if (DAT_00668618 == 0) {
            return 0;
        }
    }
    DAT_00668618 = 0;
    return 1;
}

// FUNCTION: LEGOLAND 0x0046cf20
unsigned int FUN_0046cf20(void) {
    int diff = (int)(GetGameTimer() - AdvisorHelpStartTime);

    if (diff > 0x7530 || (DAT_007fe044 != 0 && DAT_00668724 != NULL && ((struct ObjectiveEvent *)DAT_00668724)->type != 0)) {
        return 1;
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046cf60
LEGO_EXPORT void ProcessInGameHelp(void) {
    int delta;

    PostPendingObjectiveHints();

    delta = (int)(GetGameTimer() - DAT_007fe054);
    if (delta > 0xc8) {
        RunLevelScript();
        DAT_007fe054 = GetGameTimer();
    }

    if ((DAT_007fe040 & 0x1) != 0) {
        if (FUN_0046cf20() != 0) {
            ClearAdvisorHelp();
            DAT_007fe050 = GetGameTimer();
        } else {
            BubbleHelp(AdvisorHelpRect, AdvisorHelpText, 2);
            DAT_007fe050 = GetGameTimer();
        }
    } else {
        if (FUN_0046cee0() != 0) {
            ShowNextObjectiveMessage();
        }
    }

    RenderHelpIcons();
    UpdateAdvicePriority();
}

// FUNCTION: LEGOLAND 0x0046cff0
void ShowFocusedIconHelp(void) {
    RECT rect;
    struct IconNode *icon;

    icon = (struct IconNode *)FocussedIconPtr;
    if (icon != NULL && (icon->flags & 0x2000) != 0 && DAT_00668954 == 0) {
        int y = MousePos.y;
        int x = MousePos.x;
        rect.left = x;
        rect.top = y - 10;
        rect.right = x;
        rect.bottom = y;
        HTBubbleHelp(&rect, icon->string, 2);
        icon = (struct IconNode *)FocussedIconPtr;
        if ((icon->flags & 0x1000) != 0) {
            ShowHelpByName((UINT_PTR)((struct Ride *)icon->field_8)->element->name);
        } else {
            ShowHelpById(icon->string_id);
        }
    }
}

// FUNCTION: LEGOLAND 0x0046d080
LEGO_EXPORT void ProcessFrontEndHelp(void) {
    RECT rect;
    struct IconNode *icon;

    RenderHelpIcons();
    icon = (struct IconNode *)FocussedIconPtr;
    if (icon != NULL) {
        int y = MousePos.y;
        int x = MousePos.x;
        rect.left = x;
        rect.top = y - 10;
        rect.right = x;
        rect.bottom = y;
        if ((icon->flags & 0x2000) != 0) {
            HTBubbleHelp(&rect, icon->string, 2);
            icon = (struct IconNode *)FocussedIconPtr;
            if ((icon->flags & 0x1000) != 0) {
                ShowHelpText(*(const char **)icon->field_8);
            } else {
                ShowHelpById(icon->string_id);
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0046d100
LEGO_EXPORT void KillHelp(void) { FreeAllScriptLists(); }

// FUNCTION: LEGOLAND 0x0046d110
void UpdateSpeechPlayback(void) {
    char buf[256];

    if (HelpRequested == 0) {
        CurrentHelpKey = 0xffffffff;
    }
    if (DAT_006687ac == 0 && DAT_006687b0 != 0) {
        DAT_006687b0 = DAT_006687b0 - 1;
        return;
    }
    if ((int)CurrentHelpKind < 3 && HelpRequested != 0) {
        unsigned int prev = HelpChanged;
        HelpRequested = 0;
        if (prev != 0) {
            if (GetTickCount() - HelpShownTick < 500 && DAT_006687ac == 0) {
                DAT_006687b4 = 1;
                return;
            }
            DAT_006687b4 = 0;
            HelpChanged = 0;
            DAT_006687ac = 0;
            HelpShownTick = GetTickCount();
            SpeechCloseFile();
            if (CurrentHelpKey != 0xffffffff) {
                switch (CurrentHelpKind) {
                case 0:
                    // STRING: LEGOLAND 0x004ba858
                    sprintf(buf, "Text%04d.wav", (int)CurrentHelpKey);
                    break;
                case 1:
                    // STRING: LEGOLAND 0x004ba868
                    sprintf(buf, "%s.wav", (const char *)CurrentHelpKey);
                    break;
                case 2:
                    // STRING: LEGOLAND 0x004ba870
                    sprintf(buf, "%sz.wav", (const char *)CurrentHelpKey);
                    break;
                }
                SpeechLoadWavFile(buf);
                SpeechPlay();
            }
        }
    }
}

// FUNCTION: LEGOLAND 0x0046d230
void ShowHelpById(unsigned int a1) {
    if (SpeechIsPlaying() == 0) {
        if (a1 == 0xffffffff) {
            return;
        }
        if (a1 != CurrentHelpKey) {
            CurrentHelpKey = a1;
            HelpShownTick = GetTickCount();
            CurrentHelpKind = 0;
            HelpChanged = 1;
            ClearAdvicePriority();
        }
    }
    HelpRequested = 1;
}

// FUNCTION: LEGOLAND 0x0046d280
unsigned int PlaySpeech(unsigned int a1) {
    if (SpeechIsPlaying() == 0) {
        if (a1 != 0xffffffff && a1 != CurrentHelpKey) {
            CurrentHelpKey = a1;
            HelpShownTick = GetTickCount();
            CurrentHelpKind = 0;
            HelpChanged = 1;
            DAT_006687ac = 1;
            ClearAdvicePriority();
            HelpRequested = 1;
            UpdateSpeechPlayback();
            return 1;
        }
    }
    return 0;
}

// FUNCTION: LEGOLAND 0x0046d2f0
void ShowHelpByName(UINT_PTR a1) {
    if (SpeechIsPlaying() != 0) {
        HelpRequested = 1;
        return;
    }
    if (a1 != 0) {
        if (a1 != CurrentHelpKey) {
            CurrentHelpKey = a1;
            HelpShownTick = GetTickCount();
            CurrentHelpKind = 1;
            HelpChanged = 1;
        }
        HelpRequested = 1;
    }
}
