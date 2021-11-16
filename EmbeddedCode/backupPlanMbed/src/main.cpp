#include "blockdevice/COMPONENT_SD/include/SD/SDBlockDevice.h"
#include "FATFileSystem.h"

#include <string.h>
#include <errno.h>

#include "GY80.h"
#include "gps.h"

#define CSV_OUTPUT  1


uint32_t period_ms = 1000;

double convertDegMinToDecDeg (float degMin);

static BufferedSerial pc(USBTX, USBRX, 115200);
static BufferedSerial hc05(PD_5, PD_6, 115200);

GY80 gy80(PB_9, PB_8);
GY80Data gy80data;

GPS gps(PF_7, PF_6, 9600);
GPGGA gpsData;

Timeout gpsLedTimer;
DigitalOut gpsLed(LED2);
void togglegpsLed(void);

SDBlockDevice sd(MBED_CONF_SD_SPI_MOSI, MBED_CONF_SD_SPI_MISO, MBED_CONF_SD_SPI_CLK, MBED_CONF_SD_SPI_CS);
FATFileSystem fs("fs");



char buff[GPS_BUFFER_SIZE];

int gpsLineIndex = 0;
float gpsLedBlinkPeriod = 0.1;
float prevgpsLedBlinkPeriod = gpsLedBlinkPeriod;

int main() {
  // put your setup code here, to run once:

  pc.set_format(8,BufferedSerial::None,1);
  hc05.set_format(8, BufferedSerial::None, 1);

  gps.resetData(&gpsData);

  gpsLed = 1;
  gpsLedTimer.attach(&togglegpsLed, gpsLedBlinkPeriod);


  if (0 != sd.init()) {
      printf("Init failed \n");
      return -1;
  }

  // Set the frequency
  if (0 != sd.frequency(5000000)) {
      printf("Error setting frequency \n");
  }

  int err = fs.mount(&sd);


  #if DEBUG_SD == 1
  printf("sd size: %llu\n",         sd.size());
  printf("sd read size: %llu\n",    sd.get_read_size());
  printf("sd program size: %llu\n", sd.get_program_size());
  printf("sd erase size: %llu\n",   sd.get_erase_size());  
  printf("%s\n", (err ? "Fail :(" : "OK"));
  #endif

  mkdir("/fs/data", 0777);

  FILE *fp = fopen("/fs/data/log.csv", "w");
  if(fp == NULL) {
    strcpy(buff, "Could not open file for write\r\n");
    pc.write(buff, strlen(buff));
    error("Could not open file for write\r\n");
  }

  sprintf((char*)buff,
    "UTC Time, Loop Time, Lat (Deg), NS, Long. (Deg), EW, MSL Alt., Units, Sats., mx (uT), my (uT), mz (uT), ax (m/s^2), ay (m/s^2), az (m/s^2), gx (deg/s), gy (deg/s), gz (deg/s)\r\n" 
  );
  pc.write(buff, strlen(buff));
  hc05.write(buff, strlen(buff));
  fprintf(fp, buff);

  uint32_t prevTime = HAL_GetTick();
  uint32_t elapsedTime = 0;

  while(1) {
    // put your main code here, to run repeatedly:
    gps.updateData(&gpsData);
    gy80.Read_All(&gy80data);

    #if CSV_OUTPUT == 1
    sprintf((char*)buff,
      "%hu:%hu:%hu,%ld,%.5f,%c,%.5f,%c,%.1f,%c,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",
      
      gpsData.UTC_Hour, gpsData.UTC_Min, gpsData.UTC_Sec, elapsedTime,
      gpsData.LatitudeDecimal, gpsData.NS_Indicator, gpsData.LongitudeDecimal, gpsData.EW_Indicator,
      gpsData.MSL_Altitude, gpsData.MSL_Units, gpsData.SatellitesUsed,
      gy80data.mag_ut[0], gy80data.mag_ut[1], gy80data.mag_ut[2],
      gy80data.acc_ms[0], gy80data.acc_ms[1], gy80data.acc_ms[2],
      gy80data.w_rads[0], gy80data.w_rads[1], gy80data.w_rads[2]
    );
    #elif
    sprintf((char*)buff, 
			  "\u001b[0m\u001b[1mMX: \u001b[0m%8.3f | \u001b[0m\u001b[1mMY: \u001b[0m%8.3f | \u001b[0m\u001b[1mMZ: \u001b[0m%8.3f | \u001b[0m\u001b[1mAX: \u001b[0m%8.3f | \u001b[0m\u001b[1mAY: \u001b[0m%8.3f | \u001b[0m\u001b[1mGZ: \u001b[0m%8.3f | \u001b[0m\u001b[1mGX: \u001b[0m%8.3f | \u001b[0m\u001b[1mGY: \u001b[0m%8.3f | \u001b[0m\u001b[1mGZ: \u001b[0m%8.3f                         \r\n"
			  "\u001b[0m\u001b[1mTime: \u001b[0m%hu:%hu:%hu (UTC) | \u001b[0m\u001b[1mLatitude: \u001b[0m%10.5f %c | \u001b[0m\u001b[1mLongitude: \u001b[0m%10.5f %c | \u001b[0m\u001b[1mAltitude: \u001b[0m%10.5f %c | \u001b[0m\u001b[1mSatellites Fixed: \u001b[0m%d                                   \r\n"
        "\u001b[0m\u001b[1mLoop Time: \u001b[0m%4lu                                                     \033[2A\r",

        gy80data.mag_ut[0], gy80data.mag_ut[1], gy80data.mag_ut[2],
        gy80data.acc_ms[0], gy80data.acc_ms[1], gy80data.acc_ms[2],
        gy80data.w_rads[0], gy80data.w_rads[1], gy80data.w_rads[2],
			  gpsData.UTC_Hour, gpsData.UTC_Min, gpsData.UTC_Sec,
			  gpsData.LatitudeDecimal, gpsData.NS_Indicator,
			  gpsData.LongitudeDecimal, gpsData.EW_Indicator,
        gpsData.MSL_Altitude, gpsData.MSL_Units,
			  gpsData.SatellitesUsed, elapsedTime);
    #endif

    if(gpsData.SatellitesUsed > 1) {
      gpsLedBlinkPeriod = 1.0;
      if(prevgpsLedBlinkPeriod != gpsLedBlinkPeriod)
        gpsLedTimer.attach(&togglegpsLed, gpsLedBlinkPeriod);
      prevgpsLedBlinkPeriod = gpsLedBlinkPeriod;
    }
    else {
      gpsLedBlinkPeriod = 0.1;
      if(prevgpsLedBlinkPeriod != gpsLedBlinkPeriod)
        gpsLedTimer.attach(&togglegpsLed, gpsLedBlinkPeriod);
      prevgpsLedBlinkPeriod = gpsLedBlinkPeriod;
    }


    pc.write(buff, strlen(buff));
    hc05.write(buff, strlen(buff));
    fprintf(fp, buff);


    elapsedTime = HAL_GetTick()-prevTime;
    prevTime = HAL_GetTick();

    ThisThread::sleep_for(1000-elapsedTime);
  }
  fclose(fp);
}




void togglegpsLed() {
  gpsLed = !gpsLed;
  gpsLedTimer.attach(&togglegpsLed, gpsLedBlinkPeriod);
}