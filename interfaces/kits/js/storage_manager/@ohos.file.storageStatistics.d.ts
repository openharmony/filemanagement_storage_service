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

import type { AsyncCallback } from './basic';

/**
 * Provides filesystem statistics APIs
 *
 * @since 8
 * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
 * @systemapi
 */
declare namespace storageStatistics {
  /**
   * Get the totalSize of volume.
   *
   * @since 8
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */
  function getTotalSizeOfVolume(volumeUuid: string, callback: AsyncCallback<number>): void;
  function getTotalSizeOfVolume(volumeUuid: string): Promise<number>;
  /**
   * Get the free size Of volume.
   * 
   * @since 8
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */
  function getFreeSizeOfVolume(volumeUuid: string, callback: AsyncCallback<number>): void;
  function getFreeSizeOfVolume(volumeUuid: string): Promise<number>;

  /**
   * Get the bundlestat 
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */

  export interface BundleStats {
    appSize: number;
    cacheSize: number;
    dataSize: number;
  }
  /**
   * Get the bundlestat 
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */
  function getBundleStats(packageName: string, callback: AsyncCallback<BundleStats>): void;
  function getBundleStats(packageName: string): Promise<BundleStats>;

  /**
   * Get the CurrentBundleStats
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   */
  function getCurrentBundleStats(callback: AsyncCallback<BundleStats>): void;
  function getCurrentBundleStats(): Promise<BundleStats>;

  /**
   * Get the systemsize
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */
  function getSystemSize(callback: AsyncCallback<number>): void;
  function getSystemSize(): Promise<number>;

  /**
   * Get the UserStorageStats
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @systemapi
   */
  export interface StorageStats {
    total: number;
    audio: number;
    video: number;
    image: number;
    file: number;
    app: number;
  }

  /**
   * Get the UserStorageStats
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */
  function getUserStorageStats(userId: string, callback: AsyncCallback<StorageStats>): void;
  function getUserStorageStats(callback: AsyncCallback<StorageStats>): void;
  function getUserStorageStats(userId?: string): Promise<StorageStats>;

  /**
   * Get the totalsize
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */
  function getTotalSize(callback: AsyncCallback<number>): void;
  function getTotalSize(): Promise<number>;

  /**
   * Get the freesize
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.SpatialStatistics
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */
  function getFreeSize(callback: AsyncCallback<number>): void;
  function getFreeSize(): Promise<number>;

}

export default storageStatistics;
