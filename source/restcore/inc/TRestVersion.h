#ifndef REST_Version
#define REST_Version
/* Version information automatically generated by installer. *//*
 * These macros can be used in the following way:
 * 
 * #if REST_VERSION_CODE >= REST_VERSION(2,23,4)
 *     #include <newheader.h>
 * #else
 *     #include <oldheader.h>
 * #endif
 *
 */
#define REST_RELEASE "2.2.24"
#define REST_RELEASE_DATE "Mon Dec 28"
#define REST_RELEASE_TIME "18:17:55 CET 2020"
#define REST_GIT_COMMIT "a1b98265"
#define REST_VERSION_CODE 131608
#define REST_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define REST_SCHEMA_EVOLUTION "ON"
#endif
