#pragma once

#include "QString"

typedef struct SettingsData
{
    QString ip;

    uint16_t port;

    QString configFile;

    bool isLocalHost;
} SettingsData;
