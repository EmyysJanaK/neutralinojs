#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>

#include "lib/json/json.hpp"
#include "lib/platformfolders/platform_folders.h"

#include "settings.h"
#include "helpers.h"
#include "errors.h"
#include "api/fs/fs.h"

#if defined(_WIN32)
#include <windows.h>
#include <fileapi.h>
#endif

#define NEU_STORAGE_DIR "/.storage"
#define NEU_STORAGE_EXT ".neustorage"
#define NEU_STORAGE_EXT_REGEX ".*neustorage$"
#define NEU_STORAGE_KEY_REGEX "^[a-zA-Z-_0-9]{1,50}$"

using namespace std;
using json = nlohmann::json;

string storagePath;

namespace storage {

void init() {
    string storageLoc = "app";
    json jLoc = settings::getOptionForCurrentMode("storageLocation");
    if(!jLoc.is_null()) {
        storageLoc = jLoc.get<string>();
    }
    
    if(storageLoc == "system") {
        storagePath = sago::getDataHome() + "/" + settings::getAppId() + NEU_STORAGE_DIR;
        storagePath = helpers::normalizePath(storagePath);
    }
    else {
        storagePath = settings::joinAppPath(NEU_STORAGE_DIR);
    }
}

namespace controllers {

json __validateStorageBucket(const string &key) {
    if(regex_match(key, regex(NEU_STORAGE_KEY_REGEX)))
        return nullptr;
    json output;
    output["error"] = errors::makeErrorPayload(errors::NE_ST_INVSTKY, string(NEU_STORAGE_KEY_REGEX));
    return output;
}

json getData(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"key"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string key = input["key"].get<string>();
    json errorPayload = __validateStorageBucket(key);
    if(!errorPayload.is_null())
        return errorPayload;

    string filename = storagePath + "/" + key + NEU_STORAGE_EXT;

    fs::FileReaderResult fileReaderResult;
    fileReaderResult = fs::readFile(filename);
    if(fileReaderResult.status != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(errors::NE_ST_NOSTKEX, key);
        return output;
    }
    output["returnValue"] = fileReaderResult.data;
    output["success"] = true;
    return output;
}

json setData(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"key"})) {
        output["error"] = errors::makeMissingArgErrorPayload();
        return output;
    }
    string key = input["key"].get<string>();
    json errorPayload = __validateStorageBucket(key);
    if(!errorPayload.is_null())
        return errorPayload;

    filesystem::create_directories(CONVSTR(storagePath));
    #if defined(_WIN32)
    SetFileAttributesA(storagePath.c_str(), FILE_ATTRIBUTE_HIDDEN);
    #endif

    string filename = storagePath + "/" + key + NEU_STORAGE_EXT;
    if(!helpers::hasField(input, "data")) {
        filesystem::remove(CONVSTR(filename));
    }
    else {
        fs::FileWriterOptions fileWriterOptions;
        fileWriterOptions.data = input["data"].get<string>();
        fileWriterOptions.filename = filename;

        if(!fs::writeFile(fileWriterOptions)) {
            output["error"] = errors::makeErrorPayload(errors::NE_ST_STKEYWE, key);
            return output;
        }
    }
    output["success"] = true;
    return output;
}

json getKeys(const json &input) {
    json output;
    output["returnValue"] = json::array();

    fs::DirReaderResult dirResult;
    dirResult = fs::readDirectory(storagePath);
    if(dirResult.status != errors::NE_ST_OK) {
        output["error"] = errors::makeErrorPayload(errors::NE_ST_NOSTDIR, storagePath);
        return output;
    }

    for(const fs::DirReaderEntry &entry: dirResult.entries) {
        if(entry.type == fs::EntryTypeFile && regex_match(entry.name, regex(NEU_STORAGE_EXT_REGEX))) {
            output["returnValue"].push_back(regex_replace(entry.name, regex(NEU_STORAGE_EXT), ""));
        }
    }
    output["success"] = true;
    return output;
}

} // namespace controllers

} // namespace storage
