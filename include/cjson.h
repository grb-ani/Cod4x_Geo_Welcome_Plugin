#ifndef COD4X_CJSON_GEO_WELCOME_PLUGIN
#define COD4X_CJSON_GEO_WELCOME_PLUGIN

struct jsonGeoInfoStruct {
    const char*     countryName;
    const char*     cityName;
    const char*     asnOrganizatoin;
    const char*     time;

};

void                            jsonGeoInfoStructDelete(struct jsonGeoInfoStruct* geoInfoStruct);
struct jsonGeoInfoStruct*       jsonGeoInfoParse(const char* json);
const char*                     jsonApplicationConfigParse(const char* json);

#endif

