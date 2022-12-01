/*********************************START OF FILE********************************/
/*******************************************************************************
  * @file    DYPlayer.h
  * @author	 Atakan ERTEKiN , atakanertekinn@gmail.com
  * @version V1.0.0
  * @date	 30.11.2022
  * @rev     V1.0.0
  * @brief	 UART Control of DY-XXXX mp3 modules C Driver
********************************************************************************/
/************************************DEFINES***********************************/

/*************************** NOTICE: DONT FORGET TO*****************************/
/*  extern UART_HandleTypeDef huartx; --> main.h file						   */
/*******************************************************************************/


#ifndef DY_PATHS_IN_HEAP
#define DY_PATH_LEN 40
#endif

/************************************INCLUDES***********************************/

#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "main.h"



/**
 * Storage devices reported by module and to choose from when selecting a
 * storage device.
 */
typedef enum Device
{
    Usb      = 0x00,  /* USB Storage device.                                    */
    Sd       = 0x01,  /* SD Card.                                               */
    Flash    = 0x02,  /* Onboard flash chip (usually winbond 32, 64Mbit flash). */
    Failed   = 0xfe,  /* UART failure, can't be `-1` (so this can be uint8_t).  */
    NoDevice = 0xff   /* No storage device is online.                           */
} device_t;

/**
 * The current module play state.
 */
typedef enum PlayState
{
    Fail      = -1, /* UART Failure, can be a connection or a CRC problem.     */
    Stopped   = 0,  /* Music in stop state                                     */
    Playing   = 1,  /* Music in play state                                     */
    Paused    = 2   /* Music in pause state                                    */
}play_state_t;

/*
 * Equalize settings.
 */
typedef enum Eq
{
    Normal,        /* Equalize mod is   Normal,                                */
    Pop,           /* Equalize mod is   Pop,                                   */
    Rock,          /* Equalize mod is   Rock,                                  */
    Jazz,          /* Equalize mod is   Jazz,                                  */
    Classic        /* Equalize mod is   Classic                                */
} eq_t;

/**
 * Play modes are basically whatever you commonly find on a media player,
 * i.e.:
 * Repeat 1, Repeat all, Repeat list (dir), playlist (by dir), random play.
 *
 * The default is perhaps somewhat unexpected: `DY::PlayMode::OneOff`. Often
 * these modules will be used in toys or information displays where you can
 * press a button and hear a corresponding sound. To get default media player
 * behaviour, you should probably set `DY::PlayMode::Sequence` to just continue
 * playing the next song until all are played or skipped, then stop.
 */
typedef enum PlayMode
{
    Repeat,      /* Play all music in sequence, and repeat.                       */
    RepeatOne,   /* Repeat current sound.                                         */
    OneOff,      /* Play sound file and stop.                                     */
    Random,      /* Play random sound file.                                       */
    RepeatDir,   /* Repeat current directory.                                     */
    RandomDir,   /* Play random sound file in current folder.                     */
    SequenceDir, /* Play all sound files in current folder in sequence, and stop. */
    Sequence     /* Play all sound files on device in sequence, and stop.         */
} play_mode_t;

/**
 * The `DY::DYPlay_previousDir()` method expects this type as its argument.
 * Imagine you would press a button on a media player that selects the
 * previous directory/playlist, do you expect it to play the first song of
 * that list, or the last one? Depending on what you find logical or on your
 * requirement, this enumeration allows you to choose what happens when you
 * go to the previous directory.
 */
typedef enum PreviousDirSound
{
    FirstSound, /* When navigating to the previous dir, play the first sound.   */
    LastSound   /* When navigating to the previous dir, play the last sound.    */
}playDirSound_t;




/**
 * Function Declerations
 */

void          serialWrite(const uint8_t *buffer, uint8_t len);
void          serialWrite_crc(uint8_t crc);
uint8_t       serialRead(uint8_t *buffer, uint8_t len);
play_state_t  checkPlayState(void);
void          play(void);
void          pause(void);
void          stop(void);
void          previous(void);
void          next(void);
void          playSpecified(uint16_t number);
void          playSpecifiedDevicePath(device_t device, char *path);
device_t      getPlayingDevice(void);
void          setPlayingDevice(device_t device);
uint16_t      getSoundCount(void);
uint16_t      getPlayingSound(void);
void          previousDir(playDirSound_t song);
uint16_t      getFirstInDir(void);
uint16_t      getSoundCountDir(void);
void          setVolume(uint8_t volume);
void          volumeIncrease(void);
void          volumeDecrease(void);
void          interludeSpecified(device_t device, uint16_t number);
void          interludeSpecifiedDevicePath(device_t device, char *path);
void          stopInterlude(void);
void          setCycleMode(play_mode_t mode);
void          setCycleTimes(uint16_t cycles);
void          setEq(eq_t eq);
void          select(uint16_t number);
void          combinationPlay(char *sounds[], uint8_t len);
void          endCombinationPlay(void);
uint8_t       checksum(uint8_t *data, uint8_t len);
bool          validateCrc(uint8_t *data, uint8_t len);
void          sendCommand_nocrc(uint8_t *data, uint8_t len);
void          sendCommand(const uint8_t *data, uint8_t len, uint8_t crc);
bool          getResponse(uint8_t *buffer, uint8_t len);
void          byPathCommand(uint8_t command, device_t device, char *path);


/**
 * Method pointer-function struct definition
 */
typedef struct
{
    void (*serialWrite)(const uint8_t *buffer, uint8_t len);
    void (*serialWrite_crc)(uint8_t crc);
    uint8_t (*serialRead)(uint8_t *buffer, uint8_t len);
    play_state_t (*checkPlayState)(void);
    void (*play)(void);
    void (*pause)(void);
    void (*stop)(void);
    void (*previous)(void);
    void (*next)(void);
    void (*playSpecified)(uint16_t number);
    void (*playSpecifiedDevicePath)(device_t device, char *path);
    device_t (*getPlayingDevice)(void);
    void (*setPlayingDevice)(device_t device);
    uint16_t (*getSoundCount)(void);
    uint16_t (*getPlayingSound)(void);
    void (*previousDir)(playDirSound_t song);
    uint16_t (*getFirstInDir)(void);
    uint16_t (*getSoundCountDir)(void);
    void (*setVolume)(uint8_t volume);
    void (*volumeIncrease)(void);
    void (*volumeDecrease)(void);
    void (*interludeSpecified)(device_t device, uint16_t number);
    void (*interludeSpecifiedDevicePath)(device_t device, char *path);
    void (*stopInterlude)(void);
    void (*setCycleMode)(play_mode_t mode);
    void (*setCycleTimes)(uint16_t cycles);
    void (*setEq)(eq_t eq);
    void (*select)(uint16_t number);
    void (*combinationPlay)(char *sounds[], uint8_t len);
    void (*endCombinationPlay)(void);
    uint8_t (*checksum)(uint8_t *data, uint8_t len);
    bool (*validateCrc)(uint8_t *data, uint8_t len);
    void (*sendCommand_nocrc)(uint8_t *data, uint8_t len);
    void (*sendCommand)(const uint8_t *data, uint8_t len, uint8_t crc);
    bool (*getResponse)(uint8_t *buffer, uint8_t len);
    void (*byPathCommand)(uint8_t command, device_t device, char *path);
}DYPlayer_st;



/*
 * Control Commands Const Structure
 */

#define     COMMANDCODE                 (uint8_t)0xAA
#define     RFU                         (uint8_t)0x00   /* These are not used parameters */

#define     SIZEOF_CONTROLCOMMANDS      10
#define     SIZEOF_QUERYCOMMANDS        7
#define     SIZEOF_SETTINGCOMMANDS      10


#define     SIZEOF_COMMANDS             (SIZEOF_CONTROLCOMMANDS + \
                                         SIZEOF_QUERYCOMMANDS   + \
                                         SIZEOF_SETTINGCOMMANDS)

#define     LENGTHOF_COMMANDS           3   /* Setting cmds more than it. */
#define     LENGTHOF_CRC                1

/* Main Struct Pointer Object */
extern const DYPlayer_st DYPlayer;

/*
 * Control Commands Index Enumarators
 */

#define CMD_CRC_INDEX   3

enum
{
    /* control commands index enumarators */
    PLAY_CMD = 0,
    PAUSE_CMD,
    STOP_CMD,
    PREV_CMD,
    NEXT_CMD,
    VOLUME_INC,
    VOLUME_DEC,
    PREV_FILE,
    NEXT_FILE,
    STOP_PLAYING,
    /* query commands index enumarators */
    QPLAY_CMD,
    QCURRENTDEV_CMD,
    QCURRENTPLAY_CMD,
    QNUMBEROFSONG_CMD,
    QCURRENTSONG_CMD,
    QFOLDERDIR_CMD,
    QFOLDERNUMBER_CMD,
    /* settings commands index enumarators */
    SETVOLUME_CMD,
    SETLOOPMODE_CMD,
    SETCYCTIMES_CMD,
    SETEQ_CMD,
    SPECIFIEDSONG_CMD,
    SPECIFIEDPATH_CMD,
    SWTICHDRIVE_CMD,
    SPECSONGINTER_CMD,
    SPECPATHINTER_CMD,
    SLCTBUTNOPLAY_CMD,
};
