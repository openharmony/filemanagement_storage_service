---
target_release: OpenHarmony-5.0-Release
last_updated: 2026-05-28
---

# JS公共API

## 模块导入

```ts
import { storageStatistics } from '@kit.CoreFileKit';
```

系统能力: SystemCapability.FileManagement.StorageService.SpatialStatistics

---

## 1. storageStatistics.getCurrentBundleStats

获取当前应用自身的存储空间大小信息。

### 重载一：Promise方式

```ts
getCurrentBundleStats(): Promise<BundleStats>
```

- **API版本**: 9+
- **权限**: 无
- **返回值**: `Promise<BundleStats>` — 当前应用存储空间大小(字节)

### 重载二：Callback方式

```ts
getCurrentBundleStats(callback: AsyncCallback<BundleStats>): void
```

- **参数**:

| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| callback | AsyncCallback\<BundleStats\> | 是 | 获取当前应用存储空间大小之后的回调 |

### 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|---------|
| 401 | 输入参数无效 | WHEN 调用方未传入必填参数或参数类型错误 THEN 返回 401 |
| 13600001 | IPC错误 | WHEN storage_manager SA服务未启动或IPC代理失效 THEN 返回 13600001 |
| 13900042 | 未知错误 | WHEN 内部执行出现未预期异常（如文件系统statfs调用失败）THEN 返回 13900042，需通过hilog日志标签StorageStatusMgr定位具体原因 |

### 示例

```ts
import { storageStatistics } from '@kit.CoreFileKit';

// Promise方式
storageStatistics.getCurrentBundleStats().then((bundleStats) => {
    console.info('appSize: ' + bundleStats.appSize);
    console.info('cacheSize: ' + bundleStats.cacheSize);
    console.info('dataSize: ' + bundleStats.dataSize);
}).catch((err) => {
    console.error('getCurrentBundleStats failed with error: ' + err.code);
});

// Callback方式
storageStatistics.getCurrentBundleStats((err, bundleStats) => {
    if (err) {
        console.error('getCurrentBundleStats failed with error: ' + err.code);
        return;
    }
    console.info('appSize: ' + bundleStats.appSize);
    console.info('cacheSize: ' + bundleStats.cacheSize);
    console.info('dataSize: ' + bundleStats.dataSize);
});
```

---

## 2. storageStatistics.getTotalSize

获取内置存储的总空间大小。

### 重载一：Promise方式

```ts
getTotalSize(): Promise<number>
```

- **API版本**: 15+
- **权限**: 无
- **返回值**: `Promise<number>` — 内置存储总空间大小(字节)

### 重载二：Callback方式

```ts
getTotalSize(callback: AsyncCallback<number>): void
```

### 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|---------|
| 401 | 输入参数无效 | WHEN 调用方未传入必填参数或参数类型错误 THEN 返回 401 |
| 13600001 | IPC错误 | WHEN storage_manager SA服务未启动或IPC代理失效 THEN 返回 13600001 |
| 13900042 | 未知错误 | WHEN 内部执行出现未预期异常（如文件系统statfs调用失败）THEN 返回 13900042，需通过hilog日志标签StorageStatusMgr定位具体原因 |

### 示例

```ts
// Promise方式
storageStatistics.getTotalSize().then((totalSize) => {
    console.info('totalSize: ' + totalSize);
});

// Callback方式
storageStatistics.getTotalSize((err, totalSize) => {
    if (err) { console.error('getTotalSize failed: ' + err.code); return; }
    console.info('totalSize: ' + totalSize);
});
```

---

## 3. storageStatistics.getTotalSizeSync

同步获取内置存储的总空间大小。

```ts
getTotalSizeSync(): number
```

- **API版本**: 15+
- **权限**: 无
- **返回值**: `number` — 内置存储总空间大小(字节)

### 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|---------|
| 13600001 | IPC错误 | WHEN storage_manager SA服务未启动或IPC代理失效 THEN 返回 13600001 |
| 13900042 | 未知错误 | WHEN 内部执行出现未预期异常（如文件系统statfs调用失败）THEN 返回 13900042，需通过hilog日志标签StorageStatusMgr定位具体原因 |

### 示例

```ts
let totalSize = storageStatistics.getTotalSizeSync();
console.info('totalSize: ' + totalSize);
```

---

## 4. storageStatistics.getFreeSize

获取内置存储的可用空间大小。

### 重载一：Promise方式

```ts
getFreeSize(): Promise<number>
```

- **API版本**: 15+
- **权限**: 无
- **返回值**: `Promise<number>` — 内置存储可用空间大小(字节)

### 重载二：Callback方式

```ts
getFreeSize(callback: AsyncCallback<number>): void
```

### 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|---------|
| 401 | 输入参数无效 | WHEN 调用方未传入必填参数或参数类型错误 THEN 返回 401 |
| 13600001 | IPC错误 | WHEN storage_manager SA服务未启动或IPC代理失效 THEN 返回 13600001 |
| 13900042 | 未知错误 | WHEN 内部执行出现未预期异常（如文件系统statfs调用失败）THEN 返回 13900042，需通过hilog日志标签StorageStatusMgr定位具体原因 |

### 示例

```ts
storageStatistics.getFreeSize().then((freeSize) => {
    console.info('freeSize: ' + freeSize);
});
```

---

## 5. storageStatistics.getFreeSizeSync

同步获取内置存储的可用空间大小。

```ts
getFreeSizeSync(): number
```

- **API版本**: 15+
- **权限**: 无
- **返回值**: `number` — 内置存储可用空间大小(字节)

### 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|---------|
| 13600001 | IPC错误 | WHEN storage_manager SA服务未启动或IPC代理失效 THEN 返回 13600001 |
| 13900042 | 未知错误 | WHEN 内部执行出现未预期异常（如文件系统statfs调用失败）THEN 返回 13900042，需通过hilog日志标签StorageStatusMgr定位具体原因 |

### 示例

```ts
let freeSize = storageStatistics.getFreeSizeSync();
console.info('freeSize: ' + freeSize);
```

---

## 数据类型: BundleStats

| 属性名 | 类型 | 只读 | 可选 | 说明 |
|--------|------|------|------|------|
| appSize | number | 否 | 否 | 应用安装文件大小(字节) |
| cacheSize | number | 否 | 否 | 应用缓存文件大小(字节) |
| dataSize | number | 否 | 否 | 应用文件存储大小(字节，除安装文件) |

---

## 兼容性声明

- **已有 API 行为变更:** getCurrentBundleStats（API 9+）、getTotalSize/getFreeSize（API 15+）的函数签名、参数类型、返回值结构、错误码含义为稳定公共契约，不允许破坏性变更
- **数据返回格式:** BundleStats 的 appSize/cacheSize/dataSize 字段单位始终为字节（Byte），不可变更
- **同步接口:** getTotalSizeSync/getFreeSizeSync（API 15+）为同步阻塞调用，最大阻塞时间不超过 IPC 默认超时（5000ms）
