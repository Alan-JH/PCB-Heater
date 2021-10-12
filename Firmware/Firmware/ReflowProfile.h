/* Sample Reflow profile
 * 
 */

int PEAKTHRESHOLD = 180;

uint8_t stageTemps[4] = {
  90, //Start of soak
  120, //End of soak
  184, //Peak
  25 //End
};

uint8_t stageTimes[4] = {
  40, //Ramp to soak
  100, //During soak
  30, //Time at peak
  60 //Cool time
};
