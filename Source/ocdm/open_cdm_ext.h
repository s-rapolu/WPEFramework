#pragma once

#include "open_cdm.h"

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

enum OcdmLicenseType {
    // this is in the order of priority
    // standard license has priority over limited duration license
    // TODO: do we need prefix here?
    OCDM_LICENSE_INVALID = 0,
    OCDM_LICENSE_LIMITED_DURATION,
    OCDM_LICENSE_STANDARD
};

enum OcdmSessionState {
    LicenseAcquisitionState = 0,
    InactiveDecryptionState,
    ActiveDecryptionState,
    InvalidState
};

//////////////////////////////////////
// Session
//////////////////////////////////////

// TODO: Do we need this one?
struct OpenCDMAccessor* opencdm_create_system_netflix(const char readDir[], const char storeLocation[]);

OpenCDMError opencdm_init_system_netflix(struct OpenCDMAccessor* system);

// TODO: document we need at least 64 bytes in "versionStr"
OpenCDMError opencdm_system_get_version(struct OpenCDMAccessor* system, char versionStr[]);

OpenCDMError opencdm_system_get_ldl_session_limit(struct OpenCDMAccessor* system, uint32_t * ldlLimit);

OpenCDMError opencdm_system_enable_secure_stop(struct OpenCDMAccessor* system, uint32_t use);

OpenCDMError opencdm_system_commit_secure_stop(struct OpenCDMAccessor* system, const unsigned char sessionID[], uint32_t sessionIDLength, const unsigned char serverResponse[], uint32_t serverResponseLength);

OpenCDMError opencdm_system_get_drm_time(struct OpenCDMAccessor* system, time_t * time);

OpenCDMError opencdm_system_teardown(struct OpenCDMAccessor* system);
OpenCDMError opencdm_delete_secure_store(struct OpenCDMAccessor* system);
// TODO: document that buffer needs to be at least 256 bytes big
OpenCDMError opencdm_get_secure_store_hash(struct OpenCDMAccessor* system, uint8_t secureStoreHash[], uint32_t secureStoreHashLength);


//////////////////////////////////////
// Session
//////////////////////////////////////

OpenCDMError opencdm_create_session_netflix(struct OpenCDMAccessor* system, struct OpenCDMSession ** opencdmSession, uint32_t sessionId, const char contentId[], uint32_t contentIdLength,
                                            enum OcdmLicenseType licenseType, const uint8_t drmHeader[], uint32_t drmHeaderLength);

uint32_t opencdm_session_get_session_id_netflix(struct OpenCDMSession * opencdmSession);

// TODO: do we need a specific "opencdm_destroy_session" for Netflix?
// TODO: rename to "destruct"?
OpenCDMError opencdm_destroy_session_netflix(struct OpenCDMSession * opencdmSession);

// play levels
uint16_t opencdm_session_get_playlevel_compressed_video(struct OpenCDMSession * mOpenCDMSession);
uint16_t opencdm_session_get_playlevel_uncompressed_video(struct OpenCDMSession * mOpenCDMSession);
uint16_t opencdm_session_get_playlevel_analog_video(struct OpenCDMSession * mOpenCDMSession);
uint16_t opencdm_session_get_playlevel_compressed_audio(struct OpenCDMSession * mOpenCDMSession);
uint16_t opencdm_session_get_playlevel_uncompressed_audio(struct OpenCDMSession * mOpenCDMSession);

OpenCDMError opencdm_session_get_content_id(struct OpenCDMSession * opencdmSession, char * buffer, uint32_t * bufferSize);
OpenCDMError opencdm_session_set_content_id(struct OpenCDMSession * opencdmSession, const char contentId[], uint32_t contentIdLength);

enum OcdmLicenseType opencdm_session_get_license_type(struct OpenCDMSession * opencdmSession);
OpenCDMError opencdm_session_set_license_type(struct OpenCDMSession * opencdmSession, enum OcdmLicenseType licenseType);

enum OcdmSessionState opencdm_session_get_session_state(struct OpenCDMSession * opencdmSession);
OpenCDMError opencdm_session_set_session_state(struct OpenCDMSession * opencdmSession, enum OcdmSessionState sessionState);

OpenCDMError opencdm_session_set_drm_header(struct OpenCDMSession * opencdmSession, const uint8_t drmHeader[], uint32_t drmHeaderSize);

// TODO: document that this is a two-pass system (first get size, then get data).
OpenCDMError opencdm_session_get_challenge_data_netflix(struct OpenCDMSession * mOpenCDMSession, uint8_t * challenge, uint32_t * challengeSize, uint32_t isLDL);

// TODO: document that "secureStopId" should be 16 bytes.
OpenCDMError opencdm_session_store_license_data(struct OpenCDMSession * mOpenCDMSession, const uint8_t licenseData[], uint32_t licenseDataSize, unsigned char * secureStopId);

OpenCDMError opencdm_session_init_decrypt_context_by_kid(struct OpenCDMSession * mOpenCDMSession);

// TODO: document that IVData can be NULL.
OpenCDMError opencdm_session_decrypt_netflix(struct OpenCDMSession * mOpenCDMSession, const unsigned char* IVData, uint32_t IVDataSize, unsigned long long byteOffset, unsigned char dataBuffer[], uint32_t dataBufferSize);

#ifdef __cplusplus
} // extern "C"
#endif
