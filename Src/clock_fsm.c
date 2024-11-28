/*
 * clock_fsm.c
 *
 *  Created on: Nov 16, 2024
 *      Author: ASUS TUF GAMING
 */

#include "clock_fsm.h"

#include "uart.h"

#define BUTTONUP button_count[3]
#define BUTTONDOWN button_count[7]
#define LONGPRESSTIME 40
#define LONGUPDATETIME 4

ClockState clockState = INIT;
ClockModifyState clockModifyState = DATE;
int isRemind = NOREMIND;

int isUartUpdate = 0;
int uartUpdateValue = 0;

int displayDate = 0;
int displayMonth = 0;
int displayYear = 0;
int displayDay = 0;
int displayHour = 0;
int displayMinute = 0;
int displaySecond = 0;

int remindDate = 0;
int remindMonth = 0;
int remindYear = 0;
int remindDay = 0;
int remindHour = 0;
int remindMinute = 0;
int remindSecond = 0;

static int uart_sent_conut = 0;

void updateTime(uint8_t date, uint8_t month, uint8_t year, uint8_t day, uint8_t hour,
                uint8_t minute, uint8_t second)
{
    ds3231_Write(ADDRESS_YEAR, year);
    ds3231_Write(ADDRESS_MONTH, month);
    ds3231_Write(ADDRESS_DATE, date);
    ds3231_Write(ADDRESS_DAY, day);
    ds3231_Write(ADDRESS_HOUR, hour);
    ds3231_Write(ADDRESS_MIN, minute);
    ds3231_Write(ADDRESS_SEC, second);
}

void updateDisplayTime(int date, int month, int year, int day, int hour, int minute, int second)
{
    displayDate = date;
    displayMonth = month;
    displayYear = year;
    displayDay = day;
    displayHour = hour;
    displayMinute = minute;
    displaySecond = second;
}

void updateRemindTime(int date, int month, int year, int day, int hour, int minute, int second)
{
    remindDate = date;
    remindMonth = month;
    remindYear = year;
    remindDay = day;
    remindHour = hour;
    remindMinute = minute;
    remindSecond = second;
}

void checkTimeRemind()
{
    if (isRemind == NOREMIND)
    {
        if (remindDate == ds3231_date && remindMonth == ds3231_month && remindYear == ds3231_year &&
            remindDay == ds3231_day && remindHour == ds3231_hours && remindMinute == ds3231_min &&
            remindSecond == ds3231_sec)
        {
            isRemind = REMIND;
        }
    }
    else
    {
        if (ds3231_sec == (remindSecond - 1) || (remindSecond == 0 && ds3231_sec == 59))
        {
            isRemind = NOREMIND;
        }
    }
}

void showUpdateErrorAndBackToNormal(){
    lcd_ShowStr(70, 180, "UPDATE FALSE", GRED, BLACK, 32, 1);
    HAL_Delay(1000); // show error string, delay 1s then back to normal mode
    clockState = NORMAL;
    lcd_Clear(BLACK);
    updateDisplayTime(ds3231_date, ds3231_month, ds3231_year, ds3231_day, ds3231_hours,
                    ds3231_min, ds3231_sec);
    displayNormalTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                    displayMinute, displaySecond);
}

void clock_fsm()
{
    switch (clockState)
    {
        case INIT:
            clockState = NORMAL;
            updateDisplayTime(ds3231_date, ds3231_month, ds3231_year, ds3231_day, ds3231_hours,
                              ds3231_min, ds3231_sec);
            displayNormalTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                              displayMinute, displaySecond);
            break;
        case NORMAL:
            if (button_count[0] == 1)
            {
                clockState = MODIFY;
                clockModifyState = DATE;
                toggleVar = 0;
                setTimer3(0.25 * 1000 / MAINTASKTIME);  // 0.25s
                lcd_Clear(BLACK);
                updateDisplayTime(ds3231_date, ds3231_month, ds3231_year, ds3231_day, ds3231_hours,
                                  ds3231_min, ds3231_sec);
                displayToggleTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                                  displayMinute, displaySecond, clockModifyState);
            }
            else
            {
                clockState = NORMAL;
                lcd_ShowStr(70, 40, "NORMAL", GRED, BLACK, 32, 1);
                updateDisplayTime(ds3231_date, ds3231_month, ds3231_year, ds3231_day, ds3231_hours,
                                  ds3231_min, ds3231_sec);
                displayNormalTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                                  displayMinute, displaySecond);
                checkTimeRemind();
                if (isRemind == REMIND)
                {
                    lcd_ShowStr(70, 180, "WAKE UP", GRED, BLACK, 32, 1);
                }
                else
                {
                    lcd_ShowStr(70, 180, "WAKE UP", BLACK, BLACK, 32, 1);
                }
            }
            break;
        case MODIFY:
            if (button_count[0] == 1)
            {
                clockState = MODIFYREMIND;
                clockModifyState = DATE;
                toggleVar = 0;
                setTimer3(0.25 * 1000 / MAINTASKTIME);  // 0.25s
                lcd_Clear(BLACK);
                updateDisplayTime(remindDate, remindMonth, remindYear, remindDay, remindHour,
                                  remindMinute, remindSecond);
                displayNormalTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                                  displayMinute, displaySecond);
            }
            else
            {
                clockState = MODIFY;
                lcd_ShowStr(70, 40, "MODIFY", GRED, BLACK, 32, 1);
                if (clockModifyState == DATE)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayDate < 31)
                        {
                            ++displayDate;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayDate > 1)
                        {
                            --displayDate;
                        }
                    }
                }
                else if (clockModifyState == MONTH)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayMonth < 12)
                        {
                            ++displayMonth;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayMonth > 1)
                        {
                            --displayMonth;
                        }
                    }
                }
                else if (clockModifyState == YEAR)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayYear < 99)
                        {
                            ++displayYear;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayYear > 1)
                        {
                            --displayYear;
                        }
                    }
                }
                else if (clockModifyState == DAY)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayDay < 8)
                        {
                            ++displayDay;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayDay > 2)
                        {
                            --displayDay;
                        }
                    }
                }
                else if (clockModifyState == HOUR)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayHour < 23)
                        {
                            ++displayHour;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayHour > 0)
                        {
                            --displayHour;
                        }
                    }
                }
                else if (clockModifyState == MINUTE)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayMinute < 59)
                        {
                            ++displayMinute;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayMinute > 0)
                        {
                            --displayMinute;
                        }
                    }
                }
                else if (clockModifyState == SECOND)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displaySecond < 59)
                        {
                            ++displaySecond;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displaySecond > 0)
                        {
                            --displaySecond;
                        }
                    }
                }
                else
                {
                }

                if (flag_timer3)
                {
                    toggleVar = 1 - toggleVar;
                    setTimer3(0.25 * 1000 / MAINTASKTIME);  // 0.25s
                }
                displayToggleTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                                  displayMinute, displaySecond, clockModifyState);

                if (button_count[1] == 1)
                {
                    // save time and change to next variable
                    lcd_Clear(BLACK);
                    updateTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                               displayMinute, displaySecond);
                    toggleVar = 0;
                    if (clockModifyState == SECOND)
                    {
                        clockModifyState = DATE;
                    }
                    else
                    {
                        ++clockModifyState;
                    }
                }
            }
            break;
        case MODIFYREMIND:
            if (button_count[0] == 1)
            {
                clockState = REQUEST;
                clockModifyState = DATE;
                toggleVar = 0;
                setTimer3(0.25 * 1000 / MAINTASKTIME);  // 0.25s
                isUartUpdate = 0;
                lcd_Clear(BLACK);
                updateDisplayTime(ds3231_date, ds3231_month, ds3231_year, ds3231_day, ds3231_hours,
                                  ds3231_min, ds3231_sec);
                displayToggleTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                                  displayMinute, displaySecond, clockModifyState);
            }
            else
            {
                clockState = MODIFYREMIND;
                lcd_ShowStr(30, 40, "MODIFYREMIND", GRED, BLACK, 32, 1);
                if (clockModifyState == DATE)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayDate < 31)
                        {
                            ++displayDate;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayDate > 1)
                        {
                            --displayDate;
                        }
                    }
                }
                else if (clockModifyState == MONTH)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayMonth < 12)
                        {
                            ++displayMonth;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayMonth > 1)
                        {
                            --displayMonth;
                        }
                    }
                }
                else if (clockModifyState == YEAR)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayYear < 99)
                        {
                            ++displayYear;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayYear > 1)
                        {
                            --displayYear;
                        }
                    }
                }
                else if (clockModifyState == DAY)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayDay < 8)
                        {
                            ++displayDay;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayDay > 2)
                        {
                            --displayDay;
                        }
                    }
                }
                else if (clockModifyState == HOUR)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayHour < 23)
                        {
                            ++displayHour;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayHour > 0)
                        {
                            --displayHour;
                        }
                    }
                }
                else if (clockModifyState == MINUTE)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayMinute < 59)
                        {
                            ++displayMinute;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displayMinute > 0)
                        {
                            --displayMinute;
                        }
                    }
                }
                else if (clockModifyState == SECOND)
                {
                    if ((BUTTONUP == 1) || ((BUTTONUP >= LONGPRESSTIME) &&
                                            ((BUTTONUP - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displaySecond < 59)
                        {
                            ++displaySecond;
                        }
                    }
                    if ((BUTTONDOWN == 1) ||
                        ((BUTTONDOWN >= LONGPRESSTIME) &&
                         ((BUTTONDOWN - LONGPRESSTIME) % (LONGUPDATETIME) == 0)))
                    {
                        if (displaySecond > 0)
                        {
                            --displaySecond;
                        }
                    }
                }
                else
                {
                }

                if (flag_timer3)
                {
                    toggleVar = 1 - toggleVar;
                    setTimer3(0.25 * 1000 / MAINTASKTIME);  // 0.25s
                }
                displayToggleTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                                  displayMinute, displaySecond, clockModifyState);

                if (button_count[1] == 1)
                {
                    // save time and change to next variable
                    lcd_Clear(BLACK);
                    updateRemindTime(displayDate, displayMonth, displayYear, displayDay,
                                     displayHour, displayMinute, displaySecond);
                    toggleVar = 0;
                    if (clockModifyState == SECOND)
                    {
                        clockModifyState = DATE;
                    }
                    else
                    {
                        ++clockModifyState;
                    }
                }
            }
            break;
        case REQUEST:
            if (button_count[0] == 1)
            {
                clockState = NORMAL;
                lcd_Clear(BLACK);
                updateDisplayTime(ds3231_date, ds3231_month, ds3231_year, ds3231_day, ds3231_hours,
                                  ds3231_min, ds3231_sec);
                displayNormalTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                                  displayMinute, displaySecond);
            }
            else
            {
                clockState = REQUEST;
                lcd_ShowStr(70, 40, "REQUEST", GRED, BLACK, 32, 1);

                if (clockModifyState == DATE)
                {
                    if (isUartUpdate == 0)
                    {
                        // send request uart
                        uart_Rs232SendString("D");
                        uart_Rs232SendString("a");
                        uart_Rs232SendString("t");
                        uart_Rs232SendString("e");
                        uart_Rs232SendString("\n");
						uart_sent_conut = 1;
                        uartState = NUM1;
                        isUartUpdate = 3; //3 is wait
						setTimer4(10000);
                        // change var isUartUpdate = 1 if updated
                    }
					else if (isUartUpdate == 3)
					{
						if (flag_timer4)
						{	
							if (uart_sent_conut <= 2)
							{	
								uart_Rs232SendString("D");
								uart_Rs232SendString("a");
								uart_Rs232SendString("t");
								uart_Rs232SendString("e");
								uart_Rs232SendString("\n");
								++uart_sent_conut;
								setTimer4(10000);
							}
							else // if (uart_sent_conut >= 3)
							{
								uart_sent_conut = 0;
								//TODO: báo lỗi thông qua LCD và quay về chế độ hoạt động bình thường.
                                showUpdateErrorAndBackToNormal();
							}
						}
					}
                    else if (isUartUpdate == 1)
                    {	
						if (uartUpdateValue >= 0 && uartUpdateValue <= 31)
						{
							displayDate = uartUpdateValue;
							isUartUpdate = 2;
						}
						else
						{
							isUartUpdate = 0;
						}
                    }
                }
				
                else if (clockModifyState == MONTH)
                {
                    if (isUartUpdate == 0)
                    {
                        // send request uart
                        uart_Rs232SendString("M");
                        uart_Rs232SendString("o");
                        uart_Rs232SendString("n");
                        uart_Rs232SendString("t");
                        uart_Rs232SendString("h");
                        uart_Rs232SendString("\n");
						uart_sent_conut = 1;
                        uartState = NUM1;
                        isUartUpdate = 3; //3 is wait
						setTimer4(10000);
                        // change var isUartUpdate = 1 if updated
                    }
					else if (isUartUpdate == 3)
					{
						if (flag_timer4)
						{	
							if (uart_sent_conut <= 2)
							{	
								uart_Rs232SendString("M");
								uart_Rs232SendString("o");
								uart_Rs232SendString("n");
								uart_Rs232SendString("t");
								uart_Rs232SendString("h");
								uart_Rs232SendString("\n");
								++uart_sent_conut;
								setTimer4(10000);
							}
							else // if (uart_sent_conut >= 3)
							{
								uart_sent_conut = 0;
								//TODO: báo lỗi thông qua LCD và quay về chế độ hoạt động bình thường.
                                showUpdateErrorAndBackToNormal();
							}
						}
					}
                    else if (isUartUpdate == 1)
                    {
                        if (uartUpdateValue >= 0 && uartUpdateValue <= 12)
						{
							displayDate = uartUpdateValue;
							isUartUpdate = 2;
						}
						else
						{
							isUartUpdate = 0;
						}
                    }
                }

                else if (clockModifyState == YEAR)
                {
                    if (isUartUpdate == 0)
                    {
                        // send request uart
                        uart_Rs232SendString("y");
                        uart_Rs232SendString("e");
                        uart_Rs232SendString("a");
                        uart_Rs232SendString("r");
                        uart_Rs232SendString("\n");
						uart_sent_conut = 1;
                        uartState = NUM1;
                        isUartUpdate = 3; //3 is wait
						setTimer4(10000);
                        // change var isUartUpdate = 1 if updated
                    }
					else if (isUartUpdate == 3)
					{
						if (flag_timer4)
						{	
							if (uart_sent_conut <= 2)
							{	
								uart_Rs232SendString("D");
								uart_Rs232SendString("a");
								uart_Rs232SendString("t");
								uart_Rs232SendString("e");
								uart_Rs232SendString("\n");
								++uart_sent_conut;
								setTimer4(10000);
							}
							else // if (uart_sent_conut >= 3)
							{
								uart_sent_conut = 0;
								//TODO: báo lỗi thông qua LCD và quay về chế độ hoạt động bình thường.
                                showUpdateErrorAndBackToNormal();
							}
						}
					}
                    else if (isUartUpdate == 1)
                    {	
						if (uartUpdateValue >= 0 && uartUpdateValue <= 31)
						{
							displayDate = uartUpdateValue;
							isUartUpdate = 2;
						}
						else
						{
							isUartUpdate = 0;
						}
                    }
                }

                else if (clockModifyState == DAY)
                {
                    if (isUartUpdate == 0)
                    {
                        // send request uart
                        uart_Rs232SendString("d");
                        uart_Rs232SendString("a");
                        uart_Rs232SendString("y");
                        uart_Rs232SendString("\n");
						uart_sent_conut = 1;
                        uartState = NUM1;
                        isUartUpdate = 3; //3 is wait
						setTimer4(10000);
                        // change var isUartUpdate = 1 if updated
                    }
					else if (isUartUpdate == 3)
					{
						if (flag_timer4)
						{	
							if (uart_sent_conut <= 2)
							{	
								uart_Rs232SendString("D");
								uart_Rs232SendString("a");
								uart_Rs232SendString("y");
								uart_Rs232SendString("\n");
								++uart_sent_conut;
								setTimer4(10000);
							}
							else // if (uart_sent_conut >= 3)
							{
								uart_sent_conut = 0;
								//TODO: báo lỗi thông qua LCD và quay về chế độ hoạt động bình thường.
                                showUpdateErrorAndBackToNormal();
							}
						}
					}
                    else if (isUartUpdate == 1)
                    {	
						if (uartUpdateValue >= 0 && uartUpdateValue <= 31)
						{
							displayDate = uartUpdateValue;
							isUartUpdate = 2;
						}
						else
						{
							isUartUpdate = 0;
						}
                    }
                }

                else if (clockModifyState == HOUR)
                {
                    if (isUartUpdate == 0)
                    {
                        // send request uart
                        uart_Rs232SendString("h");
                        uart_Rs232SendString("o");
                        uart_Rs232SendString("u");
                        uart_Rs232SendString("r");
                        uart_Rs232SendString("\n");
						uart_sent_conut = 1;
                        uartState = NUM1;
                        isUartUpdate = 3; //3 is wait
						setTimer4(10000);
                        // change var isUartUpdate = 1 if updated
                    }
					else if (isUartUpdate == 3)
					{
						if (flag_timer4)
						{	
							if (uart_sent_conut <= 2)
							{	
								uart_Rs232SendString("h");
								uart_Rs232SendString("o");
								uart_Rs232SendString("u");
								uart_Rs232SendString("r");
								uart_Rs232SendString("\n");
								++uart_sent_conut;
								setTimer4(10000);
							}
							else // if (uart_sent_conut >= 3)
							{
								uart_sent_conut = 0;
								//TODO: báo lỗi thông qua LCD và quay về chế độ hoạt động bình thường.
                                showUpdateErrorAndBackToNormal();
							}
						}
					}
                    else if (isUartUpdate == 1)
                    {	
						if (uartUpdateValue >= 0 && uartUpdateValue <= 31)
						{
							displayDate = uartUpdateValue;
							isUartUpdate = 2;
						}
						else
						{
							isUartUpdate = 0;
						}
                    }
                }

                else if (clockModifyState == MINUTE)
                {
                    if (isUartUpdate == 0)
                    {
                        // send request uart
                        uart_Rs232SendString("M");
                        uart_Rs232SendString("i");
                        uart_Rs232SendString("n");
                        uart_Rs232SendString("u");
                        uart_Rs232SendString("\n");
						uart_sent_conut = 1;
                        uartState = NUM1;
                        isUartUpdate = 3; //3 is wait
						setTimer4(10000);
                        // change var isUartUpdate = 1 if updated
                    }
					else if (isUartUpdate == 3)
					{
						if (flag_timer4)
						{	
							if (uart_sent_conut <= 2)
							{	
								uart_Rs232SendString("M");
								uart_Rs232SendString("i");
								uart_Rs232SendString("n");
								uart_Rs232SendString("u");
								uart_Rs232SendString("t");
								uart_Rs232SendString("e");
								uart_Rs232SendString("\n");
								++uart_sent_conut;
								setTimer4(10000);
							}
							else // if (uart_sent_conut >= 3)
							{
								uart_sent_conut = 0;
								//TODO: báo lỗi thông qua LCD và quay về chế độ hoạt động bình thường.
                                showUpdateErrorAndBackToNormal();
							}
						}
					}
                    else if (isUartUpdate == 1)
                    {	
						if (uartUpdateValue >= 0 && uartUpdateValue <= 31)
						{
							displayDate = uartUpdateValue;
							isUartUpdate = 2;
						}
						else
						{
							isUartUpdate = 0;
						}
                    }
                }
                else if (clockModifyState == SECOND)
                {
                    if (isUartUpdate == 0)
                    {
                        // send request uart
                        uart_Rs232SendString("S");
                        uart_Rs232SendString("e");
                        uart_Rs232SendString("c");
                        uart_Rs232SendString("o");
						uart_Rs232SendString("n");
						uart_Rs232SendString("d");
                        uart_Rs232SendString("\n");
						uart_sent_conut = 1;
                        uartState = NUM1;
                        isUartUpdate = 3; //3 is wait
						setTimer4(10000);
                        // change var isUartUpdate = 1 if updated
                    }
					else if (isUartUpdate == 3)
					{
						if (flag_timer4)
						{	
							if (uart_sent_conut <= 2)
							{	
								uart_Rs232SendString("S");
								uart_Rs232SendString("e");
								uart_Rs232SendString("c");
								uart_Rs232SendString("o");
								uart_Rs232SendString("n");
								uart_Rs232SendString("d");
								uart_Rs232SendString("\n");
								++uart_sent_conut;
								setTimer4(10000);
							}
							else // if (uart_sent_conut >= 3)
							{
								uart_sent_conut = 0;
								//TODO: báo lỗi thông qua LCD và quay về chế độ hoạt động bình thường.
                                showUpdateErrorAndBackToNormal();
							}
						}
					}
                    else if (isUartUpdate == 1)
                    {	
						if (uartUpdateValue >= 0 && uartUpdateValue <= 31)
						{
							displayDate = uartUpdateValue;
							isUartUpdate = 2;
						}
						else
						{
							isUartUpdate = 0;
						}
                    }
                }
                else
                {
                }

                if (flag_timer3)
                {
                    toggleVar = 1 - toggleVar;
                    setTimer3(0.25 * 1000 / MAINTASKTIME);  // 0.25s
                }
                displayToggleTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                                  displayMinute, displaySecond, clockModifyState);

                if (button_count[1] == 1)
                {
                    // save time and change to next variable
                    lcd_Clear(BLACK);
                    isUartUpdate = 0;
                    updateTime(displayDate, displayMonth, displayYear, displayDay, displayHour,
                               displayMinute, displaySecond);
                    toggleVar = 0;
                    if (clockModifyState == SECOND)
                    {
                        clockModifyState = DATE;
                    }
                    else
                    {
                        ++clockModifyState;
                    }
                }
            }
            break;
    }
}
