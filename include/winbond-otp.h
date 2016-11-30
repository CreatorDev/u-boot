/*
 * Copyright (C) 2015 Imagination Technologies
 * Author: Avinash Tahakik <avinash.tahakik@imgtec.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef WINBOND_OTP_H
#define WINBOND_OTP_H
        #define MAC_ADDR_LEN 6
        int read_otp_data(loff_t from, size_t len, char *data);
#endif
