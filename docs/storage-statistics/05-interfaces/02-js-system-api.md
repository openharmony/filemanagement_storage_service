---
target_release: OpenHarmony-5.0-Release
last_updated: 2026-05-28
---

# JS系统API

## 模块导入

```ts
import { storageStatistics } from '@kit.CoreFileKit';
```

通用要求:
- **权限**: `ohos.permission.STORAGE_MANAGER`
- **系统接口**: 是（仅系统应用可用）
- **系统能力**: SystemCapability.FileManagement.StorageService.SpatialStatistics

---

## 1. storageStatistics.getTotalSizeOfVolume

获取外置存储设备中指定卷设备的总空间大小。

### 重载一：Promise方式

```ts
getTotalSizeOfVolume(volumeUuid: string): Promise<number>
```

### 重载二：Callback方式

```ts
getTotalSizeOfVolume(volumeUuid: string, callback: AsyncCallback<number>): void
```

### 参数

| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| volumeUuid | string | 是 | 卷设备UUID |
| callback | AsyncCallback\<number\> | 否 | 获取指定卷设备总空间之后的回调 |

### 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|---------|
| 201 | 权限校验失败 | WHEN 调用方未声明 ohos.permission.STORAGE_MANAGER 权限 THEN 返回 201 |
| 202 | 调用方不是系统应用 | WHEN 调用方为非系统应用 THEN 返回 202 |
| 401 | 输入参数无效 | WHEN volumeUuid为空或非string类型 THEN 返回 401 |
| 13600001 | IPC错误 | WHEN storage_manager SA服务未启动或IPC代理失效 THEN 返回 13600001 |
| 13600008 | 无此对象(卷不存在) | WHEN 指定UUID的卷设备不存在或未挂载 THEN 返回 13600008 |
| 13900042 | 未知错误 | WHEN 内部执行出现未预期异常 THEN 返回 13900042 |

### 示例

```ts
let volumeUuid = "uuid-123-456";
storageStatistics.getTotalSizeOfVolume(volumeUuid).then((totalSize) => {
    console.info('totalSize of volume: ' + totalSize);
});
```

---

## 2. storageStatistics.getFreeSizeOfVolume

获取外置存储设备中指定卷设备的可用空间大小。

### 重载一：Promise方式

```ts
getFreeSizeOfVolume(volumeUuid: string): Promise<number>
```

### 重载二：Callback方式

```ts
getFreeSizeOfVolume(volumeUuid: string, callback: AsyncCallback<number>): void
```

### 参数

| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| volumeUuid | string | 是 | 卷设备UUID |
| callback | AsyncCallback\<number\> | 否 | 获取指定卷设备可用空间之后的回调 |

### 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|---------|
| 201 | 权限校验失败 | WHEN 调用方未声明 ohos.permission.STORAGE_MANAGER 权限 THEN 返回 201 |
| 202 | 调用方不是系统应用 | WHEN 调用方为非系统应用 THEN 返回 202 |
| 401 | 输入参数无效 | WHEN volumeUuid为空或非string类型 THEN 返回 401 |
| 13600001 | IPC错误 | WHEN storage_manager SA服务未启动或IPC代理失效 THEN 返回 13600001 |
| 13600008 | 无此对象(卷不存在) | WHEN 指定UUID的卷设备不存在或未挂载 THEN 返回 13600008 |
| 13900042 | 未知错误 | WHEN 内部执行出现未预期异常 THEN 返回 13900042 |

### 示例

```ts
let volumeUuid = "uuid-123-456";
storageStatistics.getFreeSizeOfVolume(volumeUuid).then((freeSize) => {
    console.info('freeSize of volume: ' + freeSize);
});
```

---

## 3. storageStatistics.getBundleStats

获取指定应用包名的存储数据空间大小（支持分身应用）。

### 重载一：Promise方式

```ts
getBundleStats(packageName: string, index?: number): Promise<BundleStats>
```

### 重载二：Callback方式

```ts
getBundleStats(packageName: string, callback: AsyncCallback<BundleStats>, index?: number): void
```

### 参数

| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| packageName | string | 是 | 应用包名 |
| callback | AsyncCallback\<BundleStats\> | 是(Callback方式) | 获取应用存储空间大小之后的回调 |
| index | number | 否 | 分身应用索引号(API 12+)，默认0(主应用)，分身从1开始 |

### 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|---------|
| 201 | 权限校验失败 | WHEN 调用方未声明 ohos.permission.STORAGE_MANAGER 权限 THEN 返回 201 |
| 202 | 调用方不是系统应用 | WHEN 调用方为非系统应用 THEN 返回 202 |
| 401 | 输入参数无效 | WHEN 调用方未传入必填参数或参数类型错误 THEN 返回 401 |
| 13600001 | IPC错误 | WHEN storage_manager SA服务未启动或IPC代理失效 THEN 返回 13600001 |
| 13600008 | 无此对象(应用不存在) | WHEN 指定UUID的卷设备不存在或未挂载 THEN 返回 13600008 |
| 13900042 | 未知错误 | WHEN 内部执行出现未预期异常 THEN 返回 13900042 |

### 示例

```ts
// 查询主应用
storageStatistics.getBundleStats("com.example.app").then((bundleStats) => {
    console.info('appSize: ' + bundleStats.appSize);
    console.info('cacheSize: ' + bundleStats.cacheSize);
    console.info('dataSize: ' + bundleStats.dataSize);
});

// 查询分身应用(API 12+)
storageStatistics.getBundleStats("com.example.app", 1).then((bundleStats) => {
    console.info('clone app appSize: ' + bundleStats.appSize);
});
```

---

## 4. storageStatistics.getSystemSize

获取系统数据的空间大小。

### 重载一：Promise方式

```ts
getSystemSize(): Promise<number>
```

### 重载二：Callback方式

```ts
getSystemSize(callback: AsyncCallback<number>): void
```

### 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|---------|
| 201 | 权限校验失败 | WHEN 调用方未声明 ohos.permission.STORAGE_MANAGER 权限 THEN 返回 201 |
| 202 | 调用方不是系统应用 | WHEN 调用方为非系统应用 THEN 返回 202 |
| 401 | 输入参数无效 | WHEN 调用方未传入必填参数或参数类型错误 THEN 返回 401 |
| 13600001 | IPC错误 | WHEN storage_manager SA服务未启动或IPC代理失效 THEN 返回 13600001 |
| 13900042 | 未知错误 | WHEN 内部执行出现未预期异常 THEN 返回 13900042 |

### 示例

```ts
storageStatistics.getSystemSize().then((systemSize) => {
    console.info('systemSize: ' + systemSize);
});
```

---

## 5. storageStatistics.getUserStorageStats

获取用户各类别存储空间大小。有四个重载。

### 重载一：获取当前用户（Promise）

```ts
getUserStorageStats(): Promise<StorageStats>
```

### 重载二：获取当前用户（Callback）

```ts
getUserStorageStats(callback: AsyncCallback<StorageStats>): void
```

### 重载三：获取指定用户（Promise）

```ts
getUserStorageStats(userId: number): Promise<StorageStats>
```

### 重载四：获取指定用户（Callback）

```ts
getUserStorageStats(userId: number, callback: AsyncCallback<StorageStats>): void
```

### 参数

| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| userId | number | 否 | 用户ID，不传则查询当前用户 |
| callback | AsyncCallback\<StorageStats\> | 是(Callback方式) | 返回各类别存储空间大小的回调 |

### 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|---------|
| 201 | 权限校验失败 | WHEN 调用方未声明 ohos.permission.STORAGE_MANAGER 权限 THEN 返回 201 |
| 202 | 调用方不是系统应用 | WHEN 调用方为非系统应用 THEN 返回 202 |
| 401 | 输入参数无效 | WHEN 调用方未传入必填参数或参数类型错误 THEN 返回 401 |
| 13600001 | IPC错误 | WHEN storage_manager SA服务未启动或IPC代理失效 THEN 返回 13600001 |
| 13600009 | 用户ID超出范围 | WHEN 传入的userId超出系统有效用户ID范围 THEN 返回 13600009 |
| 13900042 | 未知错误 | WHEN 内部执行出现未预期异常 THEN 返回 13900042 |

### 示例

```ts
// 查询当前用户
storageStatistics.getUserStorageStats().then((storageStats) => {
    console.info('total: ' + storageStats.total);
    console.info('audio: ' + storageStats.audio);
    console.info('video: ' + storageStats.video);
    console.info('image: ' + storageStats.image);
    console.info('file: ' + storageStats.file);
    console.info('app: ' + storageStats.app);
});

// 查询指定用户
storageStatistics.getUserStorageStats(100).then((storageStats) => {
    console.info('user 100 total: ' + storageStats.total);
});
```

---

## 数据类型: StorageStats

| 属性名 | 类型 | 只读 | 可选 | 说明 |
|--------|------|------|------|------|
| total | number | 否 | 否 | 内置存储总空间大小(字节) |
| audio | number | 否 | 否 | 音频数据大小(字节) |
| video | number | 否 | 否 | 视频数据大小(字节) |
| image | number | 否 | 否 | 图像数据大小(字节) |
| file | number | 否 | 否 | 文件数据大小(字节) |
| app | number | 否 | 否 | 应用数据大小(字节) |

**系统接口**: 是

---

## 兼容性声明

- **已有 API 行为变更:** getTotalSizeOfVolume/getFreeSizeOfVolume/getBundleStats/getSystemSize/getUserStorageStats 的函数签名、参数类型、返回值结构、错误码含义为稳定系统契约，不允许破坏性变更
- **权限要求:** 所有系统 API 持续要求 ohos.permission.STORAGE_MANAGER 权限及系统应用身份，此要求不可放宽
- **数据返回格式:** StorageStats 的 total/audio/video/image/file/app 字段单位始终为字节（Byte），不可变更
- **错误码稳定性:** 错误码 201/202/401/13600001/13600008/13600009/13900042 的含义和触发条件为稳定契约，不可变更
