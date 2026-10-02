#ifndef COD4X_GEO_WELCOME_PLUGIN_CJSON_H
#define COD4X_GEO_WELCOME_PLUGIN_CJSON_H

struct jsonGeoIpStruct {
    const char* countryName;
    const char* cityName;
    const char* asnOrganization;
    const char* timeZone;

};

void jsonGeoIpStructDelete(struct jsonGeoIpStruct* geoIpStruct);
struct jsonGeoIpStruct* jsonGeoIpParse(const char* json);

#endif

