/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the `fdatasync' function. */
#ifndef CONFIG_H
#define CONFIG_H

#define HAVE_FDATASYNC 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `mtp' library (-lmtp). */
#define HAVE_LIBMTP 1

/* Check device capabilities */
#undef HAVE_LIBMTP_CHECK_CAPABILITY

/* Define to 1 if you have the <libmtp.h> header file. */
#define HAVE_LIBMTP_H 1

/* Have libusb 1.0 */
#define HAVE_LIBUSB1 /*  */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Name of package */
#define PACKAGE "simple-mtpfs"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "peter@hatina.eu"

/* Define to the full name of this package. */
#define PACKAGE_NAME "simple-mtpfs"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "simple-mtpfs 0.4.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "simple-mtpfs"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.4.0"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Directory for temporary files */
#define TMPDIR "/tmp"

/* USB device path format string */
#define USB_DEVPATH "/dev/bus/usb/%u/%u"

/* Version number of package */
#define VERSION "0.4.0"

#endif //CONFIG_H