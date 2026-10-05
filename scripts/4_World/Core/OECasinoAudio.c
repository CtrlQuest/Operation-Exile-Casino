class OECasinoAudio
{
    static void PlayCardFlip()
    {
        PlayUiSound("OECasino_CardFlip_SoundSet");
    }

    static void PlayDiceRoll()
    {
        PlayUiSound("OECasino_DiceRoll_SoundSet");
    }

    static void PlaySpinTheWin()
    {
        PlayUiSound("OECasino_SpinTheWin_SoundSet");
    }

    static void PlayRouletteWheel()
    {
        PlayUiSound("OECasino_RouletteWheel_SoundSet");
    }

    static float GetDiceRollDuration()
    {
        return 0.83;
    }

    static float GetSpinTheWinDuration()
    {
        return 11.05;
    }

    static float GetRouletteWheelDuration()
    {
        return 12.10;
    }


    static void PlaySlotSpin()
    {
        PlayUiSound("OECasino_SlotSpin_SoundSet");
    }

    static void PlaySlotReelStop()
    {
        PlayUiSound("OECasino_SlotReelStop_SoundSet");
    }

    static void PlaySlotWin()
    {
        PlayUiSound("OECasino_SlotWin_SoundSet");
    }

    static void PlaySlotLose()
    {
        PlayUiSound("OECasino_SlotLose_SoundSet");
    }

    static void PlaySlotJackpot()
    {
        PlayUiSound("OECasino_SlotJackpot_SoundSet");
    }

    static void PlaySlotPayout()
    {
        PlayUiSound("OECasino_SlotPayout_SoundSet");
    }

    static float GetSlotSpinDuration()
    {
        return 3.84;
    }

    protected static void PlayUiSound(string soundSet)
    {
        if (!GetGame() || GetGame().IsDedicatedServer()) return;
        EffectSound sound = SEffectManager.PlaySound(soundSet, vector.Zero);
        if (sound) sound.SetAutodestroy(true);
    }
}
