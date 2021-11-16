#ifndef GPS_H
#define GPS_H

#include <mbed.h>
#include <string>

#define GPS_BUFFER_SIZE 1024


typedef struct {
	uint16_t UTC_Hour;
	uint16_t UTC_Min;
	uint16_t UTC_Sec;
	uint16_t UTC_MicroSec;

	float	Latitude;
	double  LatitudeDecimal;
	char	NS_Indicator;

	float	Longitude;
	double 	LongitudeDecimal;
	char	EW_Indicator;

	uint16_t	PositionFixIndicator;
	uint16_t	SatellitesUsed;
	float	HDOP;
	float	MSL_Altitude;
	char	MSL_Units;
	float	Geoid_Separation;
	char	Geoid_Units;

	uint16_t	AgeofDiffCorr;
	char	DiffRefStationID[4];
	char	CheckSum[2];


} GPGGA;


class GPS {
public:
    GPS(PinName Tx, PinName Rx, int baud);
    ~GPS();

    void resetData(GPGGA *data);
    void updateData(GPGGA *data);


private:
    double convertDegMinToDecDeg(float degMin);

    BufferedSerial serial;
    char gpsBuffer[GPS_BUFFER_SIZE];
    std::string gpsLineBuffer;
};


#endif // GPS_H