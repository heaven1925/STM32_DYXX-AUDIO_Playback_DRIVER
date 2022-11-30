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



/************************************INCLUDES***********************************/
#include "DYPlayer.h"


/*******************************************************************************
  @func    : serialWrite
  @param   : uint8_t *buffer, uint8_t len
  @return  : void
  @date	   : 30.11.22
  @brief   : Virtual method that should implement writing from the module via UART.
********************************************************************************/
void serialWrite(const uint8_t *buffer, uint8_t len) {
     
	HAL_UART_Transmit(DYPLAYERUART , &buffer[0] , len , 100 );
	
}
/*******************************************************************************
  @func    : serialWrite_crc
  @param   : uint8_t crc
  @return  : void
  @date	   : 30.11.22
  @brief   : Map writing a single byte to the same method as writing a buffer of
             length 1. That buffer has crc value
********************************************************************************/
void serialWrite_crc(uint8_t crc) {
       
	uint8_t buf[1];
	buf[0] = crc;
		
    HAL_UART_Transmit(DYPLAYERUART, &buf[0] , 1 , 100);
}
/*******************************************************************************
  @func    : serialRead
  @param   : uint8_t *buffer, uint8_t len
  @return  : uint8_t
  @date	   : 30.11.22
  @brief   : Virtual method that should implement reading from the module via UART.
********************************************************************************/
uint8_t serialRead(uint8_t *buffer, uint8_t len) {

	HAL_UART_Receive(DYPLAYERUART, &buffer[0], len, 100);

    return true;
}
/*******************************************************************************
  @func    : checksum
  @param   : uint8_t *data, uint8_t len
  @return  : uint8_t
  @date	   : 30.11.22
  @brief   : Calculate the sum of all bytes in a buffer as a simple "CRC".
********************************************************************************/
uint8_t  checksum(uint8_t *data, uint8_t len) {
    uint8_t sum = 0;
    for (uint8_t i=0; i < len; i++) {
      sum = sum + data[i];
    }
    return sum;
}
/*******************************************************************************
  @func    : validateCrc
  @param   : uint8_t *data, uint8_t len
  @return  : bool
  @date	   : 30.11.22
  @brief   : Validate data buffer with CRC byte (last byte should be the CRC byte).
********************************************************************************/
bool validateCrc(uint8_t *data, uint8_t len) {
    uint8_t crc = data[len - 1];
    return checksum(data, len - 1) == crc;
}
/*******************************************************************************
  @func    : sendCommand_nocrc
  @param   : void
  @return  : uint8_t *data, uint8_t len
  @date	   : 30.11.22
  @brief   : Send a command to the module, adds a CRC to the passed buffer.
********************************************************************************/
void sendCommand_nocrc(uint8_t *data, uint8_t len) {

    uint8_t crc = checksum(data, len);
    serialWrite(data, len);
    serialWrite_crc(crc);
}
/*******************************************************************************
  @func    : sendCommand
  @param   : uint8_t *data, uint8_t len, uint8_t crc
  @return  : void
  @date	   : 30.11.22
  @brief   : data pointer to bytes to send to the module.
********************************************************************************/
void sendCommand (const uint8_t *data, uint8_t len, uint8_t crc) {
    serialWrite(data, len);
    serialWrite_crc(crc);
}
/*******************************************************************************
  @func    : getResponse
  @param   : uint8_t *buffer, uint8_t len
  @return  : bool
  @date	   : 30.11.22
  @brief   : Get a response to a command.
        	 Reads data from UART, validates the CRC, and puts it in the buffer.
********************************************************************************/
bool getResponse(uint8_t *buffer, uint8_t len) {
    if (serialRead(buffer, len) > 0) {
      if (DYPlayer.validateCrc(buffer, len)) {
        return true;
      }
    }
    return false;
}
/*******************************************************************************
  @func    : byPathCommand
  @param   : uint8_t command, device_t device, char *path
  @return  : void
  @date	   : 30.11.22
  @brief   : Send command with converted paths to  weird format required by the
             modules.

             - Any dot in a path should become a star (`*`)
             - Path ending slashes should be have a star prefix, except root.

             E.g.: /SONGS1/FILE1.MP3 should become: /SONGS1﹡/FILE1*MP3
             NOTE: This comment uses a unicode * look-a-alike (﹡) because ﹡/ end the
             comment.
********************************************************************************/
void byPathCommand(uint8_t command, device_t device, char *path) {
    uint8_t len = strlen(path);
    if (len < 1) return;
    uint8_t _len = len;
    // Count / in path and, except root slash and determine new length
    for (uint8_t i = 1; i < len; i++) {
      if (path[i] == '/')
        _len++;
    }
    #ifdef DY_PATHS_IN_HEAP
    uint8_t *_command = new uint8_t[_len + 4];
    #else
    uint8_t _command[DY_PATH_LEN+4];
    #endif

    _command[0] = 0xaa;
    _command[1] = command;
    _command[2] = _len + 1;
    _command[3] = (uint8_t) device;
    _command[4] = path[0];
    uint8_t j = 5;
    for (uint8_t i = 1; i < len; i++) {
      switch(path[i]) {
        case '.':
          _command[j] = '*';
          break;
        case '/':
          _command[j] = '*';
          j++;
          // fall-through
        default:
          _command[j] = toupper(path[i]);
      }
      j++;
    }
    sendCommand_nocrc(_command, len + 4);
    #ifdef DY_PATHS_IN_HEAP
    delete[] _command;
    #endif
}
/*******************************************************************************
  @func    : checkPlayState
  @param   : void
  @return  : play_state_t
  @date	   : 30.11.22
  @brief   : Check the current play state can, be called at any time.
********************************************************************************/
play_state_t checkPlayState(void) {
	/*
    uint8_t command[3] = { 0xaa, 0x01, 0x00 };
     sendCommand(command, 3, 0xab);
    */

    sendCommand( &controlCommands[QPLAY_CMD][0]            ,
    			 LENGTHOF_COMMANDS				  		  ,
				 controlCommands[QPLAY_CMD][CMD_CRC_INDEX] );

    uint8_t buffer[6];
    if (DYPlayer.getResponse(buffer, 6)) {
      return (play_state_t) buffer[3];
    }
    //return (play_state_t) PlayState.Fail;
    return Fail; //Fudge
}
/*******************************************************************************
  @func    : play
  @param   : void
  @return  : void
  @date	   : 30.11.22
  @brief   : Play the currently selected file from the start.
********************************************************************************/
void play(void) {
	/*
    uint8_t command[3] = {0xaa, 0x02, 0x00};
    */

    sendCommand( &controlCommands[PLAY_CMD][0]            ,
    			 LENGTHOF_COMMANDS				  		  ,
				 controlCommands[PLAY_CMD][CMD_CRC_INDEX] );
}
/*******************************************************************************
  @func    : pause
  @param   : void
  @return  : void
  @date	   : 30.11.22
  @brief   : Set the play state to paused.
********************************************************************************/
void pause() {
	/*
    uint8_t command[3] = {0xaa, 0x03, 0x00};
    */

    sendCommand( &controlCommands[PAUSE_CMD][0]            ,
    			 LENGTHOF_COMMANDS				  		   ,
				 controlCommands[PAUSE_CMD][CMD_CRC_INDEX] );

}
/*******************************************************************************
  @func    : stop
  @param   : void
  @return  : void
  @date	   : 30.11.22
  @brief   : Set the play state to stopped.
********************************************************************************/
void stop() {
  /*
  uint8_t command[3] = {0xaa, 0x04, 0x00};
  */

    sendCommand( &controlCommands[STOP_CMD][0]            ,
    			 LENGTHOF_COMMANDS				  		  ,
				 controlCommands[STOP_CMD][CMD_CRC_INDEX] );
}
/*******************************************************************************
  @func    : previous
  @param   : void
  @return  : void
  @date	   : 30.11.22
  @brief   : Play the previous file.
********************************************************************************/
void previous() {
  /*
  uint8_t command[3] = {0xaa, 0x05, 0x00};
  */
    sendCommand( &controlCommands[PREV_CMD][0]             ,
    			 LENGTHOF_COMMANDS				  		   ,
				 controlCommands[PREV_CMD][CMD_CRC_INDEX] );
}
/*******************************************************************************
  @func    : next
  @param   : void
  @return  : void
  @date	   : 30.11.22
  @brief   : Play the next file.
********************************************************************************/
void next() {
  /*
  uint8_t command[3] = {0xaa, 0x06, 0x00};
  */

    sendCommand( &controlCommands[PREV_CMD][0]             ,
    			 LENGTHOF_COMMANDS				  		   ,
				 controlCommands[PREV_CMD][CMD_CRC_INDEX] );
}
/*******************************************************************************
  @func    : playSpecified
  @param   : void
  @return  : void
  @date	   : 30.11.22
  @brief   :
********************************************************************************/
void playSpecified(uint16_t number) {
  /*
  uint8_t command[5] = { 0xaa, 0x07, 0x02, 0x00, 0x00 };
  */
  uint8_t command[5] = {0};

  memcpy( &command[0] ,
		  &controlCommands[SPECIFIEDSONG_CMD][0],
		  LENGTHOF_COMMANDS);

  command[3] = number >> 8;
  command[4] = number & 0xff;
  sendCommand_nocrc(command, 5);
}
/*******************************************************************************
  @func    : playSpecifiedDevicePath
  @param   : device_t device, char *path
  @return  : void
  @date	   : 30.11.22
  @brief   : Play a sound file by number, number sent as 2 bytes.
********************************************************************************/
void playSpecifiedDevicePath(device_t device, char *path) {
  DYPlayer.byPathCommand(0x08, device, path);
}
/*******************************************************************************
  @func    : setPlayingDevice
  @param   : void
  @return  : device_t
  @date	   : 30.11.22
  @brief   : Get the storage device that is currently used for playing sound files.
********************************************************************************/
device_t getPlayingDevice(void) {
/*
  uint8_t command[3] = { 0xaa, 0x0a, 0x00 };
  sendCommand(command, 3, 0xb4);
*/

  sendCommand( &controlCommands[QCURRENTPLAY_CMD][0]            ,
    		   LENGTHOF_COMMANDS				  		   		,
			   controlCommands[QCURRENTPLAY_CMD][CMD_CRC_INDEX] );

  uint8_t buffer[5];
  if (DYPlayer.getResponse(buffer, 5)) {
    return (device_t)buffer[3];
  }
  return Failed;
}
/*******************************************************************************
  @func    : getPlayingDevice
  @param   : device_t device
  @return  : void
  @date	   : 30.11.22
  @brief   : Set the device number the module should use.
     		 Tries to set the device but no guarantee is given, use `getDevice()`
     		 to check the actual current storage device.
********************************************************************************/
void setPlayingDevice(device_t device) {
  /*
  uint8_t command[4] = { 0xaa, 0x0b, 0x01, 0x00 };
  */

  uint8_t command[4] = {0};

  memcpy( &command[0] ,
		  &controlCommands[SWTICHDRIVE_CMD][0],
		  LENGTHOF_COMMANDS);

  command[3] = (uint8_t) device;
  sendCommand_nocrc(command, 4);
}
/*******************************************************************************
  @func    : getSoundCount
  @param   : void
  @return  : uint16_t
  @date	   : 30.11.22
  @brief   : Get the amount of sound files on the current storage device.
********************************************************************************/
uint16_t getSoundCount(void) {
/*
  uint8_t command[3] = { 0xaa, 0x0c, 0x00 };
  sendCommand(command, 3, 0xb6);
*/

  sendCommand( &controlCommands[QNUMBEROFSONG_CMD][0]            ,
    		   LENGTHOF_COMMANDS				  		   		 ,
			   controlCommands[QNUMBEROFSONG_CMD][CMD_CRC_INDEX] );

  uint8_t buffer[6];
  if(DYPlayer.getResponse(buffer, 6)) {
    return (buffer[3] << 8) | buffer[4];
  }
  return 0;
}
/*******************************************************************************
  @func    : getPlayingSound
  @param   : void
  @return  : uint16_t
  @date	   : 30.11.22
  @brief   : Get the currently playing file by number.
********************************************************************************/
uint16_t getPlayingSound(void) {
/*
  uint8_t command[3] = { 0xaa, 0x0d, 0x00 };
  sendCommand(command, 3, 0xb7);
*/

  sendCommand( &controlCommands[QCURRENTSONG_CMD][0]            ,
	    	   LENGTHOF_COMMANDS				  		   		 ,
			   controlCommands[QCURRENTSONG_CMD][CMD_CRC_INDEX] );

  uint8_t buffer[6];
  if(DYPlayer.getResponse(buffer, 6)) {
    return (buffer[3] << 8) | buffer[4];
  }
  return 0;
}
/*******************************************************************************
  @func    : previousDir
  @param   : playDirSound_t song
  @return  : void
  @date	   : 30.11.22
  @brief   : Select previous directory and start playing the first or last song.
********************************************************************************/
void previousDir (playDirSound_t song) {
 if (song == LastSound)
 {
    /*
    uint8_t command[3] = { 0xaa, 0x0e, 0x00 };
    sendCommand(command, 3, 0xb8);
    */
    sendCommand( &controlCommands[PREV_FILE][0]            ,
    		     LENGTHOF_COMMANDS				  		   		 ,
    			 controlCommands[PREV_FILE][CMD_CRC_INDEX] );

  }
  else /* FirstSound */
  {
	/*
    uint8_t command[3] = { 0xaa, 0x0f, 0x00 };
    sendCommand(command, 3, 0xb9);
    */
	sendCommand( &controlCommands[NEXT_FILE][0]            ,
	      		 LENGTHOF_COMMANDS				  		   		 ,
	      		 controlCommands[NEXT_FILE][CMD_CRC_INDEX] );

  }
}
/*******************************************************************************
  @func    : getFirstInDir
  @param   : void
  @return  : uint16_t
  @date	   : 30.11.22
  @brief   : Get number of the first song in the currently selected directory.
********************************************************************************/
uint16_t getFirstInDir(void) {
  /*
  uint8_t command[3] = { 0xaa, 0x11, 0x00 };
  sendCommand(command, 3, 0xbb);
  */

   sendCommand( &controlCommands[QFOLDERDIR_CMD][0]            ,
		      	LENGTHOF_COMMANDS				  		   		 ,
		      	controlCommands[QFOLDERDIR_CMD][CMD_CRC_INDEX] );

  uint8_t buffer[6];
  if(DYPlayer.getResponse(buffer, 6)) {
    return (buffer[3] << 8) | buffer[4];
  }
  return 0;
}
/*******************************************************************************
  @func    : getSoundCountDir
  @param   : void
  @return  : uint16_t
  @date	   : 30.11.22
  @brief   : Get the amount of sound files in the currently selected directory.
********************************************************************************/
uint16_t getSoundCountDir(void) {
  /*
  uint8_t command[3] = { 0xaa, 0x12, 0x00 };
  sendCommand(command, 3, 0xbc);
  */

  sendCommand( &controlCommands[QFOLDERNUMBER_CMD][0]            ,
	      	   LENGTHOF_COMMANDS				 		 ,
	      	   controlCommands[QFOLDERNUMBER_CMD][CMD_CRC_INDEX] );

  uint8_t buffer[6];
  if(DYPlayer.getResponse(buffer, 6)) {
    return (buffer[3] << 8) | buffer[4];
  }
  return 0;
}
/*******************************************************************************
  @func    : setVolume
  @param   : uint8_t volume
  @return  : void
  @date	   : 30.11.22
  @brief   : Set the playback volume between 0 and 30.
  	  	  	 Default volume if not set: 20.
********************************************************************************/
void setVolume(uint8_t volume) {
  /*
  uint8_t command[4] = { 0xaa, 0x13, 0x01, 0x00 };
  */

  uint8_t command[4] = {0};

  memcpy( &command[0] ,
		  &controlCommands[SETVOLUME_CMD][0],
		  LENGTHOF_COMMANDS);

  command[3] = volume;
  sendCommand_nocrc(command, 4);
}
/*******************************************************************************
  @func    : volumeIncrease
  @param   : void
  @return  : void
  @date	   : 30.11.22
  @brief   : Increase the volume.
********************************************************************************/
void volumeIncrease(void) {
  /*
  uint8_t command[3] = {0xaa, 0x14, 0x00};
  sendCommand(command, 3, 0xbe);
  */
  sendCommand( &controlCommands[VOLUME_INC][0]             ,
		      	LENGTHOF_COMMANDS				  		   ,
		      	controlCommands[VOLUME_INC][CMD_CRC_INDEX] );

}
/*******************************************************************************
  @func    : volumeDecrease
  @param   : void
  @return  : void
  @date	   : 30.11.22
  @brief   : Decrease the volume.
********************************************************************************/
void volumeDecrease(void) {
  /*
  uint8_t command[3] = {0xaa, 0x15, 0x00};
  sendCommand(command, 3, 0xbf);
  */

  sendCommand( &controlCommands[VOLUME_DEC][0]             ,
			   LENGTHOF_COMMANDS				  		   ,
			   controlCommands[VOLUME_DEC][CMD_CRC_INDEX] );
}
/*******************************************************************************
  @func    : interludeSpecified
  @param   : device_t device, uint16_t number
  @return  : void
  @date	   : 30.11.22
  @brief   : Play an interlude file by device and number, number sent as 2 bytes.
             Note from the manual: "Music interlude" only has level 1. Continuous
             interlude will cover the previous interlude (the interlude will be
             played immediately). When the interlude is finished, it will return to
             the first interlude breakpoint and continue to play.
********************************************************************************/
void interludeSpecified(device_t device, uint16_t number) {

  uint8_t command[6] = { 0xaa, 0x0b, 0x03, 0x00, 0x00, 0x00 };

  command[3] = (uint8_t) device;
  command[4] = number >> 8;
  command[5] = number & 0xff;
  sendCommand_nocrc(command, 6);
}
/*******************************************************************************
  @func    : interludeSpecifiedDevicePath
  @param   : device_t device, char *path
  @return  : void
  @date	   : 30.11.22
  @brief   : Play an interlude by device and path.
       		 Note from the manual: "Music interlude" only has level 1. Continuous
       		 interlude will cover the previous interlude (the interlude will be
       	 	 played immediately). When the interlude is finished, it will return to
       		 the first interlude breakpoint and continue to play.
********************************************************************************/
void interludeSpecifiedDevicePath(device_t device, char *path) {
  DYPlayer.byPathCommand(0x17, device, path);
}
/*******************************************************************************
  @func    : stopInterlude
  @param   : void
  @return  : void
  @date	   : 30.11.22
  @brief   : Stop the interlude and continue playing.
********************************************************************************/
void stopInterlude(void) {
  /*
  uint8_t command[3] = {0xaa, 0x10, 0x00};
  sendCommand(command, 3, 0xba);
  */
  sendCommand( &controlCommands[STOP_PLAYING][0]             ,
			   LENGTHOF_COMMANDS				  		   ,
			   controlCommands[STOP_PLAYING][CMD_CRC_INDEX] );
}
/*******************************************************************************
  @func    : setCycleMode
  @param   : play_mode_t mode
  @return  : void
  @date	   : 30.11.22
  @brief   : Sets the cycle mode
********************************************************************************/
void setCycleMode(play_mode_t mode) {
  /*
  uint8_t command[4] = { 0xaa, 0x18, 0x01, 0x00 };
  */
  uint8_t command[4] = {0};

  memcpy( &command[0] ,
		  &controlCommands[SETLOOPMODE_CMD][0],
		  LENGTHOF_COMMANDS);

  command[3] = mode;
  sendCommand_nocrc(command, 4);
}
/*******************************************************************************
  @func    : setCycleTimes
  @param   : uint16_t cycles
  @return  : void
  @date	   : 30.11.22
  @brief   : Set how many cycles to play when in cycle modes 0, 1 or 4
********************************************************************************/
void setCycleTimes(uint16_t cycles) {
  /*
  uint8_t command[5] = { 0xaa, 0x19, 0x02, 0x00, 0x00 };
  */

  uint8_t command[5] = {0};

  memcpy( &command[0] ,
		  &controlCommands[SETCYCTIMES_CMD][0],
		  LENGTHOF_COMMANDS);

  command[3] = cycles >> 8;
  command[4] = cycles & 0xff;
  sendCommand_nocrc(command, 5);
}
/*******************************************************************************
  @func    : setEq
  @param   : eq_t eq
  @return  : void
  @date	   : 30.11.22
  @brief   : Set the equalizer setting.
********************************************************************************/
void setEq(eq_t eq) {
  /*
   uint8_t command[4] = { 0xaa, 0x1a, 0x01, 0x00 };
   */

  uint8_t command[4] = {0};

  memcpy( &command[0] ,
		  &controlCommands[SETEQ_CMD][0],
		  LENGTHOF_COMMANDS);

  command[3] = (uint8_t) eq;
  sendCommand_nocrc(command, 4);
}
/*******************************************************************************
  @func    : select
  @param   : uint16_t number
  @return  : void
  @date	   : 30.11.22
  @brief   : Select a sound file without playing it.  e.g. `1` for `00001.mp3`.
********************************************************************************/
void select(uint16_t number) {
  /*
  uint8_t command[5] = { 0xaa, 0x1f, 0x02, 0x00, 0x00};
  */

  uint8_t command[5] = {0};

  memcpy( &command[0] ,
		  &controlCommands[SLCTBUTNOPLAY_CMD][0],
		  LENGTHOF_COMMANDS);

  command[3] = number >> 8;
  command[4] = number & 0xff;
  sendCommand_nocrc(command, 5);
}
/*******************************************************************************
  @func    : combinationPlay
  @param   : char *sounds[], uint8_t len
  @return  : void
  @date	   : 30.11.22
  @brief   : Combination play allows you to make a playlist of multiple sound files.

             You could use this to combine numbers e.g.: "fourthy-two" where you
             have samples for "fourthy" and "two".

             This feature has a particularly curious parameters, you have to
             specify the sound files by name, they have to be named by 2 numbers
             and an extension, e.g.: `01.mp3` and specified by `01`. You should
             pass them as an array pointer. You need to put the files into a
             directory that can be called `DY`, `ZH or `XY`, you will have to check
             the manual that came with your module, or try all of them. There may
             well be more combinations! Also see
********************************************************************************/
void combinationPlay(char *sounds[], uint8_t len) {
  if (len < 1) return;
  // This part of the command can be easily determined already.
  uint8_t command[3] = { 0xaa, 0x1b, 0x00 };
  command[2] = len * 2;
  // Depends on the length, checksum is a sum so we can add the other values
  // later.
  uint8_t crc = checksum(command, 3);
  // Send the command and length already.
  serialWrite(command, 3);
  // Send each pair of chars containing the file name and add the values of
  // each char to the crc.
  for (uint8_t i=0; i < len; i++) {
    crc += checksum((uint8_t*) sounds[i], 2);
    serialWrite((uint8_t*) sounds[i], 2);
  }
  // Lastly, write the crc value.
  serialWrite_crc(crc);
}
/*******************************************************************************
  @func    : endCombinationPlay
  @param   : void
  @return  : void
  @date	   : 30.11.2022
  @brief   : End combination play.
********************************************************************************/
void endCombinationPlay(void) {
  uint8_t command[3] = {0xaa, 0x1c, 0x00};
  DYPlayer.sendCommand(command, 3, 0xc6);
}
/*******************************************************************************
  @func    : getCycleMode
  @param   : play_mode_t mode
  @return  : void
  @date	   : 30.11.22
  @brief   : Set Cycle mode
********************************************************************************/
void getCycleMode(play_mode_t mode){
	/*
	uint8_t command[4] = {0xaa, 0x18, 0x01, 0x00};
	*/

	uint8_t command[4] = {0};

	memcpy( &command[0] ,
		    &controlCommands[SETLOOPMODE_CMD][0],
		    LENGTHOF_COMMANDS);

	command[3] = mode;
	DYPlayer.sendCommand_nocrc(command, 4);

}
