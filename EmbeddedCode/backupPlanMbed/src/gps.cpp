#include "gps.h"


GPS::GPS(PinName Tx, PinName Rx, int baud) : serial(Tx, Rx, baud)
{
  serial.set_format(8, BufferedSerial::None, 1);
  serial.set_blocking(false);    
}

GPS::~GPS() {}

void GPS::resetData(GPGGA *data) {
  memset(data,0,sizeof(*data));
  data->NS_Indicator='-';
  data->EW_Indicator='-';
  data->Geoid_Units='-';
  data->MSL_Units='-';
}

void GPS::updateData(GPGGA *data) {
    memset(gpsBuffer, '\0', sizeof(gpsBuffer));

    int len = serial.read(gpsBuffer, sizeof(gpsBuffer));
    if(len > 0) {
      gpsLineBuffer += gpsBuffer;
      std::size_t pos1 = gpsLineBuffer.find("$GPGGA");
      std::size_t pos2 = gpsLineBuffer.find('\n', pos1);
      #if DEBUG_GPS == 1
      printf("\r\nTmp buffer size: %d\r\n  Tmp Buffer: %s\r\n\r\nLine Buffer Size: %d\r\n\r\n", len , gpsBuffer, gpsLineBuffer.size());
      printf(gpsLineBuffer.c_str());
      #endif

    if(pos1 != string::npos && pos2 != string::npos) {
      std::string gpsDataString = gpsLineBuffer.substr(pos1, pos2-pos1);
      memset(data,0,sizeof(*data));
      sscanf(gpsDataString.c_str(),"$GPGGA,%2hu%2hu%2hu.%3hu,%f,%c,%f,%c,%hu,%hu,%f,%f,%c,%f,%c,,*%2s",
          &data->UTC_Hour,&data->UTC_Min,&data->UTC_Sec,&data->UTC_MicroSec,&data->Latitude,
          &data->NS_Indicator,&data->Longitude,&data->EW_Indicator,&data->PositionFixIndicator,
          &data->SatellitesUsed,&data->HDOP,&data->MSL_Altitude,&data->MSL_Units,&data->Geoid_Separation,
          &data->Geoid_Units,data->CheckSum);

      if(data->NS_Indicator==0)
        data->NS_Indicator='-';
      if(data->EW_Indicator==0)
        data->EW_Indicator='-';
      if(data->Geoid_Units==0)
        data->Geoid_Units='-';
      if(data->MSL_Units==0)
        data->MSL_Units='-';

      data->LatitudeDecimal=convertDegMinToDecDeg(data->Latitude);
      data->LongitudeDecimal=convertDegMinToDecDeg(data->Longitude);      
      gpsLineBuffer.clear();
      }
    }

}

double GPS::convertDegMinToDecDeg (float degMin)
{
  double min = 0.0;
  double decDeg = 0.0;

  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);

  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );

  return decDeg;
}