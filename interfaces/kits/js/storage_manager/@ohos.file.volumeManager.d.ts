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

import {AsyncCallback, Callback} from "./basic";

/**
 * Provides volumemanager statistics APIs
 *
 * @since 9
 * @syscap SystemCapability.FileManagement.StorageService.Volume
 * @systemapi
 */
declare namespace volumeManager {

/**
   * Get All Volumes
   * 
   * @since 9
   * @systemapi
   */
 export interface Volume {
  id: string;
  uuid: string;
  diskId: string;
  description: string;
  removable: boolean;
  state: number;
  path: string;
}
/**
   * Get All Volumes
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.Volume
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */
function getAllVolumes(callback: AsyncCallback<Array<Volume>>): void;
function getAllVolumes(): Promise<Array<Volume>>;


 /**
   * Mount
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.Volume
   * @permission ohos.permission.MOUNT_UNMOUNT_MANAGER
   * @systemapi
   */
function mount(volumeId: string, callback: AsyncCallback<void>): void;
function mount(volumeId: string): Promise<void>;

/**
   * UnMount
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.Volume
   * @permission ohos.permission.MOUNT_UNMOUNT_MANAGER
   * @systemapi
   */
function unmount(volumeId: string, callback: AsyncCallback<void>): void;
function unmount(volumeId: string): Promise<void>;


/**
   * Get the volume by uuid.
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.Volume
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */
 function getVolumeByUuid(uuid: string, callback: AsyncCallback<Volume>): void;
 function getVolumeByUuid(uuid: string): Promise<Volume>;
 
/**
   * Get the volume by id.
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.Volume
   * @permission ohos.permission.STORAGE_MANAGER
   * @systemapi
   */
function getVolumeById(volumeId: string, callback: AsyncCallback<Volume>): void;
function getVolumeById(volumeId: string): Promise<Volume>;
 
/**
   * Set the description of volume.
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.Volume
   * @permission ohos.permission.MOUNT_UNMOUNT_MANAGER
   * @systemapi
   */
function setVolumeDescription(uuid: string, description: string, callback: AsyncCallback<boolean>): void;
function setVolumeDescription(uuid: string, description: string): Promise<boolean>;
 
/**
   * Format.
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.Volume
   * @permission ohos.permission.MOUNT_FORMAT_MANAGER
   * @systemapi
   */
function format(volumeId: string, fsType: string, callback: AsyncCallback<boolean>): void;
function format(volumeId: string, fsType: string): Promise<boolean>;
   
  
/**
   * Partition.
   * 
   * @since 9
   * @syscap SystemCapability.FileManagement.StorageService.Volume
   * @permission ohos.permission.MOUNT_FORMAT_MANAGER
   * @systemapi
   */
function partition(diskId: string, type: number, callback: AsyncCallback<boolean>): void;
function partition(diskId: string, type: number): Promise<boolean>;
}

export default volumeManager;
