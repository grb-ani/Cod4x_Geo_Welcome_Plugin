#ifndef COD4X_GEO_WELCOME_PLUGIN_CJSON_H
#define COD4X_GEO_WELCOME_PLUGIN_CJSON_H

struct jsonGeoIpStruct {
    const char* countryName;
    const char* cityName;
    const char* asnOrganization;
    const char* time;

};

struct jsonGeoIpStruct* jsonGeoIpParse(const char* json);

#endif

