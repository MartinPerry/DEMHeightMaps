#include "./Utils.h"



double Utils::MapRange(double fromMin, double fromMax, double toMin, double toMax, double s)
{
	return toMin + (s - fromMin) * (toMax - toMin) / (fromMax - fromMin);
}
