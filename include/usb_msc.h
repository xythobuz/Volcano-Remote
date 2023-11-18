/*
 * usb_msc.h
 *
 * Copyright (c) 2022 - 2023 Thomas Buck (thomas@xythobuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * See <http://www.gnu.org/licenses/>.
 */

#ifndef __USB_MSC_H__
#define __USB_MSC_H__

/*
 * Available: a disk is present in the drive
 * Locked: the host wants to prevent disk removal
 */

bool msc_is_medium_locked(void);
bool msc_is_medium_available(void);

// should only be set to false when unlocked
void msc_set_medium_available(bool state);

#endif // __USB_MSC_H__
