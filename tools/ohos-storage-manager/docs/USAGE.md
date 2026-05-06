# ohos-storageManager Usage Guide

## Overview

`ohos-storageManager` is a command-line tool for querying storage information. It provides access to storage statistics through the StorageManager System Ability.

## Syntax

```
ohos-storageManager <command> [options]
```

## Commands

### get-total-size

Get the total storage capacity of the device.

```
ohos-storageManager get-total-size
```

**Output:**
```json
{"success":true,"data":{"totalBytes":128849018880,"totalReadable":"120.00GB"}}
```

### get-free-size

Get the available free storage space.

```
ohos-storageManager get-free-size
```

**Output:**
```json
{"success":true,"data":{"freeBytes":64424509440,"freeReadable":"60.00GB"}}
```

### get-system-size

Get the size of the system partition.

```
ohos-storageManager get-system-size
```

**Output:**
```json
{"success":true,"data":{"systemBytes":21474836480,"systemReadable":"20.00GB"}}
```

### get-user-storage-stats

Get storage statistics for user data.

```
ohos-storageManager get-user-storage-stats [userId]
```

**Parameters:**
- `userId` (optional): Specific user ID. If not provided, returns stats for current user.

**Output:**
```json
{"success":true,"data":{"total":32212254720,"audio":5368709120,"video":10737418240,"image":5368709120,"file":5368709120,"app":5368709120,"totalReadable":"30.00GB"}}
```

### get-bundle-stats

Get storage statistics for a specific application bundle.

```
ohos-storageManager get-bundle-stats <packageName> [appIndex]
```

**Parameters:**
- `packageName` (required): The package name of the application
- `appIndex` (optional): Application index (default: 0)

**Output:**
```json
{"success":true,"data":{"packageName":"com.example.app","appIndex":0,"appSize":104857600,"cacheSize":20971520,"dataSize":52428800,"appSizeReadable":"100.00MB","cacheSizeReadable":"20.00MB","dataSizeReadable":"50.00MB"}}
```

### get-current-bundle-stats

Get storage statistics for the current running application.

```
ohos-storageManager get-current-bundle-stats
```

**Output:**
```json
{"success":true,"data":{"appSize":104857600,"cacheSize":20971520,"dataSize":52428800,"appSizeReadable":"100.00MB","cacheSizeReadable":"20.00MB","dataSizeReadable":"50.00MB"}}
```

## Error Handling

All commands return JSON with error information on failure:

```json
{"success":false,"error":{"code":-1,"message":"error description"}}
```

### Common Error Codes

| Code | Description |
|------|-------------|
| -1 | General error |
| E_SA_IS_NULLPTR (201) | Storage Manager service unavailable |
| E_PARAMS_INVALID (202) | Invalid parameters |
| E_BUNDLEMGR_ERROR (21600201) | Bundle manager error |

## Help

```
ohos-storageManager --help
```

or

```
ohos-storageManager -h
```

or

```
ohos-storageManager help
```

## Requirements

- Root or system privileges
- `ohos.permission.STORAGE_MANAGER` permission
- StorageManager SA must be running

## Notes

- All size values are in bytes
- Readable format uses IEC binary units (B, KiB, MiB, GiB, TiB)
- Some commands require the caller to be a system app
