/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import type { AsyncCallback } from "./basic";

/**
 * Provides filesystem statistics APIs
 *
 * @namespace storageStatistics
 * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
 * @since 8
 */
declare namespace storageStatistics {
  /**
   * Get the totalSize of volume.
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { string } volumeUuid The uuid of the volume
   * @param { AsyncCallback<number> } callback The callback is used to return the total size of volume
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13600008 - No such object.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 8
   */
  function getTotalSizeOfVolume(volumeUuid: string, callback: AsyncCallback<number>): void;

  /**
   * Get the totalSize of volume.
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { string } volumeUuid The uuid of the volume
   * @returns { Promise<number> } Return total size of volume
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13600008 - No such object.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 8
   */
  function getTotalSizeOfVolume(volumeUuid: string): Promise<number>;
  /**
   * Get the free size Of volume.
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { string } volumeUuid The uuid of the volume
   * @param { AsyncCallback<number> } callback The callback is used to return the free size of volume
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13600008 - No such object.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 8
   */
  function getFreeSizeOfVolume(volumeUuid: string, callback: AsyncCallback<number>): void;

  /**
   * Get the free size Of volume.
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { string } volumeUuid The uuid of the volume
   * @returns { Promise<number> } Return free size of volume
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13600008 - No such object.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 8
   */
  function getFreeSizeOfVolume(volumeUuid: string): Promise<number>;

  /**
   * Get the bundlestat
   *
   * @interface BundleStats
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @since 9
   */
  export interface BundleStats {
    /**
     * The size of application installation data.
     *
     * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
     * @since 9
     */
    appSize: number;

    /**
     * The size of application cache data.
     *
     * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
     * @since 9
     */
    cacheSize: number;

    /**
     * The size of application local data, distributed data and database data.
     *
     * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
     * @since 9
     */
    dataSize: number;
  }
  /**
   * Get the bundlestat
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { string } packageName The name of the application
   * @param { AsyncCallback<BundleStats> } callback The callback is used to return bundleStats of application
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13600008 - No such object.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getBundleStats(packageName: string, callback: AsyncCallback<BundleStats>): void;

  /**
   * Get the bundlestat
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { string } packageName The name of the application
   * @returns { Promise<BundleStats> } Return bundleStats of application
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13600008 - No such object.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getBundleStats(packageName: string): Promise<BundleStats>;

  /**
   * Get the Current BundleStats
   *
   * @param { AsyncCallback<BundleStats> } callback The callback is used to return bundleStats of the current application
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @since 9
   */
  function getCurrentBundleStats(callback: AsyncCallback<BundleStats>): void;

  /**
   * Get the Current BundleStats
   *
   * @returns { Promise<BundleStats> } Return bundleStats of the current application
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @since 9
   */
  function getCurrentBundleStats(): Promise<BundleStats>;

  /**
   * Get the systemsize
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { AsyncCallback<number> } callback The callback is used to return system size
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getSystemSize(callback: AsyncCallback<number>): void;

  /**
   * Get the systemsize
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @returns { Promise<number> } Return system size
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getSystemSize(): Promise<number>;

  /**
   * Get the UserStorageStats
   *
   * @interface StorageStats
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  export interface StorageStats {
    /**
     * The total size of device.
     *
     * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
     * @systemapi
     * @since 9
     */
    total: number;

    /**
     * The size of audio file.
     *
     * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
     * @systemapi
     * @since 9
     */
    audio: number;

    /**
     * The size of video file.
     *
     * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
     * @systemapi
     * @since 9
     */
    video: number;

    /**
     * The size of image file.
     *
     * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
     * @systemapi
     * @since 9
     */
    image: number;

    /**
     * The size of other file.
     *
     * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
     * @systemapi
     * @since 9
     */
    file: number;

    /**
     * The size of application.
     *
     * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
     * @systemapi
     * @since 9
     */
    app: number;
  }

  /**
   * Get the UserStorageStats
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @returns { Promise<StorageStats> } Return storageStats of current user
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getUserStorageStats(): Promise<StorageStats>;

  /**
   * Get the UserStorageStats
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { AsyncCallback<StorageStats> } callback The callback is used to return storageStats of current user
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getUserStorageStats(callback: AsyncCallback<StorageStats>): void;

  /**
   * Get the UserStorageStats
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { number } userId The id of the user
   * @returns { Promise<StorageStats> } Return storageStats of the user
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13600009 - User if out of range.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getUserStorageStats(userId: number): Promise<StorageStats>;

  /**
   * Get the UserStorageStats
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { number } userId The id of the user
   * @param { AsyncCallback<StorageStats> } callback The callback is used to return storageStats of the user
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13600009 - User if out of range.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getUserStorageStats(userId: number, callback: AsyncCallback<StorageStats>): void;

  /**
   * Get the totalsize
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { AsyncCallback<number> } callback The callback is used to return the total size
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getTotalSize(callback: AsyncCallback<number>): void;

  /**
   * Get the totalsize
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @returns { Promise<number> } Return the total size
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getTotalSize(): Promise<number>;

  /**
   * Get the freesize
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @param { AsyncCallback<number> } callback The callback is used to return the free size
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getFreeSize(callback: AsyncCallback<number>): void;

  /**
   * Get the freesize
   *
   * @permission ohos.permission.STORAGE_MANAGER
   * @returns { Promise<number> } Return the free size
   * @throws { BusinessError } 201 - Permission verification failed.
   * @throws { BusinessError } 202 - The caller is not a system application.
   * @throws { BusinessError } 401 - The input parameter is invalid.
   * @throws { BusinessError } 13600001 - IPC error.
   * @throws { BusinessError } 13900032 - Unknown error.
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   * @since 9
   */
  function getFreeSize(): Promise<number>;
}

export default storageStatistics;
