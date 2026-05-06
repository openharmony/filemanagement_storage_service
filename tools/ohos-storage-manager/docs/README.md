# ohos-storage-manager

Storage manager CLI tool for querying storage information.
## Features

- Query total storage size
- Query free storage size
- Query system partition size
- Query user storage statistics
- Query bundle storage statistics
- Query current bundle storage statistics

## Installation

This tool is built as part of the `storage_service` component and is installed to the system partition during image build.

```bash
# Build command (example)
./build.sh --product-name rk3568 --build-target //foundation/filemanagement/storage_service/tools/
    ohos-storage-manager:ohos-storageManager
```

## Permissions
                
The tool requires the following permission:                                                                                         
- `ohos.permission.STORAGE_MANAGER`

## Commands

| Command | Description |
|---------|-------------|
| `get-total-size` | Get total storage size |
| `get-free-size` | Get free storage size |
| `get-system-size` | Get system partition size |
| `get-user-storage-stats [userId]` | Get user storage statistics |
| `get-bundle-stats <pkg> [index]` | Get bundle storage statistics |
| `get-current-bundle-stats` | Get current bundle storage statistics |

## Output Format

All commands output JSON format:

```json
{"success":true,"data":{...}}
```

On error:
```json
{"success":false,"error":{"code":-1,"message":"error message"}}
```

## Examples

```bash
# Get total storage size
ohos-storageManager get-total-size
# Output: {"success":true,"data":{"totalBytes":128849018880,"totalReadable":"120.00GB"}}

# Get free storage size
ohos-storageManager get-free-size

# Get system partition size
ohos-storageManager get-system-size

# Get user storage stats
ohos-storageManager get-user-storage-stats

# Get user storage stats for specific user
ohos-storageManager get-user-storage-stats 100

# Get bundle stats for an app
ohos-storageManager get-bundle-stats com.example.app 0

# Get current bundle stats
ohos-storageManager get-current-bundle-stats
```
